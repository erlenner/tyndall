#pragma once
#include <chrono>
#include <stdint.h>
#include <optional>
#include <mutex>
#include <thread>
#include <cerrno>
#ifdef NO_ROS
// mock
namespace ros_context
{
  inline int shutdown()
  { return 0; }
  inline int init(int argc, char** argv, std::chrono::milliseconds loop_sleep = std::chrono::milliseconds{3}, const char *node_name = "default_node_name")
  { return 0;}
}
#include <tyndall/ipc/ipc.h>
#define ros_context_read(...) ({ ipc_read(__VA_ARGS__); })
#define ros_context_write(...) ({ ipc_write(__VA_ARGS__); })
#define ros_context_serve(...) ({ (void)sizeof(msg); 0; })
#define ros_context_call(...) ({ (void)sizeof(msg); 0; })
#else
#include <ros/ros.h>

#include "tyndall/meta/strval.h"

// ros_context wraps ros initialization, destruction and pub sub pattern in a thread safe interface.
// lazy initialization of ros communication objects is used for ease of use
namespace ros_context
{
  extern ros::NodeHandle *nh;
  extern std::mutex ros_mutex;
  extern std::thread ros_thread;
  extern int run_ros;

  // methods

  inline int shutdown()
  {
    run_ros = 0;
    ros_thread.join();

    ros::shutdown();
    return 0;
  }

  inline int init(int argc, char** argv, std::chrono::milliseconds loop_sleep = std::chrono::milliseconds{3}, const char *node_name = "default_node_name")
  {
    assert(nh == NULL); // enforce single initialization per process

    ros::init(argc, argv, node_name); // node_name is usually overridden by launch file

    // start roscore if it's not running
    if (!ros::master::check() && (fork() == 0))
    {
      int rc = system("roscore");
      assert(rc != -1);
    }

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

  template<typename Message, typename Id>
  int lazy_read(Message& msg, Id)
  {
    static Message save;
    static std::mutex save_mutex;
    static bool new_save = false;
    static bool valid_save = false;

    // register ros callback
    static bool must_initialize_callback = true;
    if (must_initialize_callback)
    {
      must_initialize_callback = false;
      std::lock_guard<typeof(ros_mutex)> guard(ros_mutex);

      static auto sub = nh->subscribe<Message>(Id::c_str(), 1,
        boost::function<void(const Message&)>
        ([](const Message& sub_msg)
        -> void {
            std::lock_guard<typeof(save_mutex)> guard(save_mutex);
            save = sub_msg;
            new_save = true;
            valid_save = true;
          })
        );
    }

    // get saved ros message
    int rc;
    {
      std::lock_guard<typeof(save_mutex)> guard(save_mutex);
      if (new_save)
      {
        new_save = false;
        msg = save;
        rc = 0;
      }
      else if (valid_save)
      {
        msg = save;
        rc = -1;
        errno = EAGAIN;
      }
      else
      {
        rc = -1;
        errno = ENOMSG;
      }
    }

    return rc;
  }

  template<typename Message, typename Id>
  void lazy_write(const Message& msg, Id)
  {
    static ros::Publisher pub = nh->advertise<Message>(Id::c_str(), 1, true); // queue size 1 and latching

    pub.publish(msg);
  }


  // Ros service based methods

  template<typename Service, typename Id>
  int lazy_serve(Service& srv, Id)
  {
    static Service save;
    static std::mutex save_mutex;
    static bool new_save = false;
    static bool valid_save = false;

    // handle service data
    int rc;
    {
      std::lock_guard<typeof(save_mutex)> guard(save_mutex);

      save.response = srv.response; // set new response

      if (new_save)
      {
        new_save = false;
        srv = save;
        rc = 0;
      }
      else if (valid_save)
      {
        srv = save;
        rc = -1;
        errno = EAGAIN;
      }
      else
      {
        rc = -1;
        errno = ENOMSG;
      }
    }

    // register ros callback
    static bool must_initialize_callback = true;
    if (must_initialize_callback)
    {
      must_initialize_callback = false;
      std::lock_guard<typeof(ros_mutex)> guard(ros_mutex);

      static auto server = nh->advertiseService(Id::c_str(),
        boost::function<bool(typename Service::Request& req, typename Service::Response& rep)>
        ([](typename Service::Request& req, typename Service::Response& rep)
        -> bool {
            std::lock_guard<typeof(save_mutex)> guard(save_mutex);
            rep = save.response;
            save.request = req;
            new_save = true;
            valid_save = true;
            return true;
          })
      );
    }

    return rc;
  }

  template<typename Service, typename Id>
  int lazy_call(Service& srv, Id)
  {
    static ros::ServiceClient client = nh->serviceClient<Service>(Id::c_str(), true);

    if (!client.isValid())
    {
      client = nh->serviceClient<Service>(Id::c_str(), true);
    }

    return client.call(srv) ? 0 : -1;
  }
}

#define ros_context_read(msg, id) \
  ros_context::lazy_read(msg, id ## _strval)

#define ros_context_write(msg, id) \
  ros_context::lazy_write(msg, id ## _strval)

#define ros_context_serve(srv, id) \
  ros_context::lazy_serve(srv, id ## _strval)

#define ros_context_call(srv, id) \
  ros_context::lazy_call(srv, id ## _strval)

#endif
