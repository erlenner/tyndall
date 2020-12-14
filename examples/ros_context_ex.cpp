#include <tyndall/ros_context.h>
#include <std_msgs/Int32.h>
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
  ros_context::init(argc, argv, std::chrono::milliseconds{3}, "comm");

  signal(SIGINT, signal_handler);

  for(int i=0; run; ++i)
  {
    std_msgs::Int32 msg;
    msg.data = i;
    ros_context_write(msg, "/ros_context_ex");

    std::this_thread::sleep_for(std::chrono::milliseconds{3});
  }

  return 0;
}
