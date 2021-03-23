#include <tyndall/ros_context.h>
#include <cstdio>
#ifdef NO_ROS
  namespace std_msgs { struct Int32 { int data; }; }
#else
  #include <std_msgs/Int32.h>
#endif
#include <csignal>
#include <thread>
#include <chrono>

sig_atomic_t run = 1;

void signal_handler(int sig)
{
  run = 0;
}

int main(int argc, char** argv)
{
  ros_context::init(argc, argv, std::chrono::milliseconds{3}, "ex_ros_context_read");

  signal(SIGINT, signal_handler);

  while(run)
  {
    std_msgs::Int32 msg;


    ros_context_read(msg, "/ex_ros_context");

    printf("read: %d\n", msg.data);

    std::this_thread::sleep_for(std::chrono::milliseconds{3});
  }

  return 0;
}
