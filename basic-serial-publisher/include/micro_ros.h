#ifndef MICRO_ROS_H_
#define MICRO_ROS_H_

#include <micro_ros_platformio.h>
#include <micro_ros_utilities/string_utilities.h>
#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>

#include "config.h"
#include "macros.h"
#include "pins.h"

constexpr uint16_t kMillisecondsInASecond = 1000;
constexpr uint32_t kNanosecondsInAMillisecond = 1000000;
constexpr uint32_t kNanosecondsInASecond = 1000000000;

enum class AgentStates {
  kWaitingForConnection,
  kAvailable,
  kConnected,
  kDisconnected
};

void HandleReturnCodeError(rcl_ret_t error_code);
struct timespec GetTime();
void UpdateTimeOffsetFromAgent();
void InitializeMicroRosSerialTransport();
void ManageAgentLifecycle();
void UpdateExampleMessage(int32_t value);

#endif /* MICRO_ROS_H_ */