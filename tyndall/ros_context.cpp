#include "ros_context.h"

namespace ros_context
{
  ros::NodeHandle *nh = NULL;
  std::mutex ros_mutex;
  std::thread ros_thread;
  int run_ros = 1;
}
