#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>
#include <std_msgs/msg/float32.h>

class ROSNode

namespace Hermes {
rcl_publisher_t publisher;
rcl_publisher_t publisher_message;
std_msgs__msg__Int32 msg;
std_msgs__msg__String received_msg;
std_msgs__msg__String send_msg; 
char received_buffer[50];
char send_buffer[100];
rcl_subscription_t subscriber;

const char* kNodeName = "teensy";
const char* kPublisherTopic = "micro_ros_response";
const char* kSubscriberTopic = "micro_ros_name";
const int kExecutorTimeout = 100;  // ms
const size_t kDomainId = 8;  // ROS domain ID

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

// Error handle loop
void error_loop() {
  while(1) {
    delay(100);
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
    msg.data++;
  }
}

void SubscriptionCallback(const void* msgin);

void init()
{
  received_msg.data.data = received_buffer;
  received_msg.data.capacity = sizeof(received_buffer);

  set_microros_serial_transports(Serial);
  delay(2000);
  Serial1.println("Hello from teensy 1!");

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "micro_ros_platformio_node_publisher"));

  if (rclc_subscription_init_default(&subscriber, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
      kSubscriberTopic) != RCL_RET_OK) {}

  const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  if (rclc_executor_add_subscription(&executor, &subscriber, &received_msg,
      &SubscriptionCallback, ON_NEW_DATA) != RCL_RET_OK) {
  }

  msg.data = 0;
}

void loop()
{
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

void SubscriptionCallback(const void* msgin) {
  digitalWrite(LED_BUILTIN, HIGH);
  const std_msgs__msg__String* msg = (const std_msgs__msg__String*)msgin;
  Serial1.print("[SUB] Received: ");
  Serial1.println(msg->data.data);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  // Create response message: "Hello <received_name>!"
  //snprintf(response_buffer, sizeof(response_buffer), "Hello %s!", msg->data.data);
  //response_msg.data.size = strlen(response_buffer);

}
}
