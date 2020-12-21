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
  ros_context::init(argc, argv, std::chrono::milliseconds{3}, "ex_ros_context");

  signal(SIGINT, signal_handler);

  for(int i=0; run; ++i)
  {
    std_msgs::Int32 msg;
    msg.data = i;

    printf("sending: %d\n", msg.data);


    ros_context_write(msg, "/ex_ros_context");

    std::this_thread::sleep_for(std::chrono::milliseconds{3});
  }

  return 0;
}
