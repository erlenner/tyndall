#include <ros/ros.h>
#include <stdint.h>
#include <optional>
#include <mutex>
#include <thread>
#include <errno.h>

// ros_context wraps ros initialization, destruction and pub sub pattern in a thread safe interface.
// lazy initialization of ros communication objects is used for ease of use
namespace ros_context
{
  static ros::NodeHandle *nh = NULL;
  static std::mutex ros_mutex;
  static std::thread ros_thread;
  static int run_ros = 1;

  // methods

  int shutdown()
  {
    run_ros = 0;
    ros_thread.join();

    ros::shutdown();
    return 0;
  }

  int init(int argc, char** argv, std::chrono::milliseconds loop_sleep = std::chrono::milliseconds{3}, const char *node_name = "default_node_name")
  {
    assert(nh == NULL); // enforce single initialization per process

    ros::init(argc, argv, node_name); // node_name is usually overridden by launch file

    static ros::NodeHandle nh_instance("~");
    nh = &nh_instance;

    static struct lifetime_t
    {
      ~lifetime_t()
      { shutdown(); }
    } lifetime;

    ros_thread = std::thread([loop_sleep](){
      while (run_ros)
      {
        {
          std::lock_guard<typeof(ros_mutex)> guard(ros_mutex);
          ros::spinOnce();
        }
        std::this_thread::sleep_for(loop_sleep);
      }
    });
    return 0;
  }

  template<typename Message, int id_hash>
  int lazy_read(Message& msg, const char *id)
  {
    static Message save;
    static std::mutex save_mutex;
    static bool new_save = false;

    // register ros callback
    {
      std::lock_guard<typeof(ros_mutex)> guard(ros_mutex);

      static ros::Subscriber sub = nh->subscribe<Message>(id, 1,
        boost::function<void(const boost::shared_ptr<const Message>&)>
        ([](const boost::shared_ptr<const Message>& sub_msg)
        -> void {
            std::lock_guard<typeof(save_mutex)> guard(save_mutex);
            save = *sub_msg;
            new_save = true;
          })
        );
    }

    int rc;

    // get saved ros message
    {
      static bool wait_for_save = true;
      static const auto wait_for_save_since = std::chrono::system_clock::now();
      constexpr std::chrono::milliseconds max_wait_time{1000};
      do
      {
        std::lock_guard<typeof(save_mutex)> guard(save_mutex);
        if (new_save)
        {
          new_save = false;
          msg = save;
          rc = 0;
        }
        else
        {
          msg = save;
          rc = -1;
          errno = EAGAIN;
        }
      } while (
        // wait for first ros message
        wait_for_save
        &&
        (
          (
            (rc != 0)
            && (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - wait_for_save_since) < max_wait_time)
            && ({ std::this_thread::sleep_for(std::chrono::milliseconds{20}); true; })
          )
          ||
          ({
            if (rc == 0)
              wait_for_save = false;
            else
              errno = ENOMSG;
            false;
          })
        )
      );
    }

    return rc;
  }

  template<typename Message, int id_hash>
  int lazy_write(const Message& msg, const char *id)
  {
    static ros::Publisher pub = nh->advertise<Message>(id, 1, true); // queue size 1 and latching

    pub.publish(msg);

    return 0;
  }

  // helpers
  constexpr uint32_t hash_fnv1a_32(const char* s, size_t count)
  {
    return ((count ? hash_fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
  }

  constexpr uint32_t operator ""_hash(const char* s, size_t count)
  {
    return hash_fnv1a_32(s, count);
  }
}

#define ros_context_read(msg, id)                             \
({                                                            \
  using namespace ros_context;                                \
  ros_context::lazy_read<typeof(msg), id ## _hash>(msg, id);  \
})

#define ros_context_write(msg, id)                             \
({                                                            \
  using namespace ros_context;                                \
  ros_context::lazy_write<typeof(msg), id ## _hash>(msg, id);  \
})