#include "ros_context.h"


#ifndef NO_ROS
namespace ros_context
{
  ros::NodeHandle *nh = NULL;
  std::mutex ros_mutex;
  std::thread ros_thread;
  int run_ros = 1;
}
#endif
