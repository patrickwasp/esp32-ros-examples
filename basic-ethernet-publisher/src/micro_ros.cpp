#include "micro_ros.h"

enum AgentStates g_agent_state;

// ros core
rclc_support_t g_ros_support;
rcl_node_t g_ros_node;
rclc_executor_t g_ros_executor;
static rcl_allocator_t g_ros_allocator;
rcl_init_options_t g_ros_init_options;
uint64_t g_agent_time_offset = 0;

// timers
rcl_timer_t g_example_update_timer;
rcl_timer_t g_example_publish_timer;

// publishers
rcl_publisher_t g_example_publisher;

// messages
std_msgs__msg__Int32 g_example_int_message;

void HandleReturnCodeError(rcl_ret_t error_code) { esp_restart(); }

struct timespec GetTime() {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = 0;

  uint64_t adjusted_time_ms = millis() + g_agent_time_offset;

  time.tv_sec = adjusted_time_ms / kMillisecondsInASecond;
  time.tv_nsec =
      (adjusted_time_ms % kMillisecondsInASecond) * kNanosecondsInAMillisecond;

  return time;
}

void UpdateTimeOffsetFromAgent() {
  uint64_t time_now = millis();
  RC_CHECK(rmw_uros_sync_session(10));
  uint64_t ros_time_ms = rmw_uros_epoch_millis();
  g_agent_time_offset = ros_time_ms - time_now;
}

void InitializeMicroRosEthernetTransport() {
  set_microros_ethernet_transports(kMicroControllerIp, kSubnetMask, kAgentIp,
                                   kAgentPort);
  g_agent_state = AgentStates::kWaitingForConnection;
}

void ExamplePublishCallback(rcl_timer_t *timer, int64_t last_call_time_ns) {
  (void)last_call_time_ns;
  if (timer != NULL) {
    RC_CHECK(rcl_publish(&g_example_publisher, &g_example_int_message, NULL));
  }
}

extern void ExampleUpdateCallback(rcl_timer_t *timer,
                                  int64_t last_call_time_ns);

void UpdateExampleMessage(int32_t value) { g_example_int_message.data = value; }

void InitializePublishersAndTimers() {
  std_msgs__msg__Int32__init(&g_example_int_message);

  RC_CHECK(rclc_publisher_init_default(
      &g_example_publisher, &g_ros_node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "example_int_publisher"));

  RC_CHECK(rclc_timer_init_default(&g_example_publish_timer, &g_ros_support,
                                   RCL_MS_TO_NS(kPublisherPeriodMs),
                                   ExamplePublishCallback));
  RC_CHECK(rclc_executor_add_timer(&g_ros_executor, &g_example_publish_timer));

  RC_CHECK(rclc_timer_init_default(&g_example_update_timer, &g_ros_support,
                                   RCL_MS_TO_NS(kUpdatePeriodMs),
                                   ExampleUpdateCallback));
  RC_CHECK(rclc_executor_add_timer(&g_ros_executor, &g_example_update_timer));
}

void DeinitializePublishersAndTimers() {
  std_msgs__msg__Int32__fini(&g_example_int_message);
  RC_CHECK(rcl_publisher_fini(&g_example_publisher, &g_ros_node));
  RC_CHECK(rcl_timer_fini(&g_example_publish_timer));
  RC_CHECK(rcl_timer_fini(&g_example_update_timer));
}

bool CreateEntities() {
  g_ros_allocator = rcl_get_default_allocator();

  g_ros_init_options = rcl_get_zero_initialized_init_options();
  RC_CHECK(rcl_init_options_init(&g_ros_init_options, g_ros_allocator));
  RC_CHECK(rcl_init_options_set_domain_id(&g_ros_init_options, kDomainId));
  rclc_support_init_with_options(&g_ros_support, 0, NULL, &g_ros_init_options,
                                 &g_ros_allocator);

  RC_CHECK(rclc_node_init_default(&g_ros_node, kNodeName, kNamespace,
                                  &g_ros_support));

  g_ros_executor = rclc_executor_get_zero_initialized_executor();
  RC_CHECK(rclc_executor_init(&g_ros_executor, &g_ros_support.context,
                              kNumberOfHandles, &g_ros_allocator));

  InitializePublishersAndTimers();

  UpdateTimeOffsetFromAgent();
  return true;
}

void DestroyEntities() {
  rmw_context_t *rmw_context =
      rcl_context_get_rmw_context(&g_ros_support.context);
  (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

  DeinitializePublishersAndTimers();

  rclc_executor_fini(&g_ros_executor);
  RC_CHECK(rcl_node_fini(&g_ros_node));
  rclc_support_fini(&g_ros_support);
}

void ManageAgentLifecycle() {
  // auto reconnect to agent
  switch (g_agent_state) {
    case AgentStates::kWaitingForConnection:
      Serial.println("Waiting for connection");
      EXECUTE_EVERY_N_MS(
          500, g_agent_state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1))
                                   ? AgentStates::kAvailable
                                   : AgentStates::kWaitingForConnection;);
      break;
    case AgentStates::kAvailable:
      Serial.println("Available");
      g_agent_state = (true == CreateEntities())
                          ? AgentStates::kConnected
                          : AgentStates::kWaitingForConnection;
      if (g_agent_state == AgentStates::kWaitingForConnection) {
        DestroyEntities();
      };
      break;
    case AgentStates::kConnected:
      Serial.println("Connected");
      EXECUTE_EVERY_N_MS(
          200, g_agent_state = (RMW_RET_OK == rmw_uros_ping_agent(200, 3))
                                   ? AgentStates::kConnected
                                   : AgentStates::kDisconnected;);
      if (g_agent_state == AgentStates::kConnected) {
        rclc_executor_spin_some(&g_ros_executor, RCL_MS_TO_NS(100));
      }
      break;
    case AgentStates::kDisconnected:
      Serial.println("Disconnected");
      DestroyEntities();
      g_agent_state = AgentStates::kWaitingForConnection;
      break;
    default:
      break;
  }
}