#pragma once

#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rcl/timer.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/string.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float32.h>

// ROS Context
class Context {
  private:
    rcl_allocator_t allocator;
    rclc_support_t support;
    rclc_executor_t executor;
    bool inited;

  public:
    Context() = default;

    bool init(size_t handles, size_t domain_id);
    void spin(unsigned long timeout);
    bool get_inited();

    rcl_allocator_t* get_allocator() { return &allocator; }
    rclc_support_t* get_support() { return &support; }
    rclc_executor_t* get_executor() { return &executor; }
};

extern Context hermes;

// ROS Nodes
class Node {
  private:
    rcl_node_t node;
    Context& context;
    bool inited;

  public:
    Node(Context& ctx, const char* name, const char* ns) :
      context{ ctx } {
        node = rcl_get_zero_initialized_node();
        rcl_ret_t rc = rclc_node_init_default(&node, name, ns, context.get_support());
        
        if (rc == RCL_RET_OK) { inited = true; }
    }

    bool get_inited();
    rcl_node_t* get_node() { return &node; }
    Context* get_context() { return &context; }
};

class Timer {
  private:
    bool inited;
    rcl_timer_t timer;
    Node& ros_node;

  public:
    Timer(Node& node, uint64_t interval, void (*cb)(rcl_timer_t*, int64_t)) :
      ros_node{ node } {
        timer = rcl_get_zero_initialized_timer();
        uint64_t period = interval * 1000000;
        rcl_ret_t rc = rclc_timer_init_default2(
          &timer, 
          node.get_context()->get_support(), 
          period,
          cb,
          false
        );
        if (rc != RCL_RET_OK) { return; }

        rc = rclc_executor_add_timer(
            node.get_context()->get_executor(),
            &timer
        );

        if (rc == RCL_RET_OK) { inited = true; }
    }

    bool start();
    bool stop();
    bool get_inited();
};

template <typename T> 
class Publisher {
  private:
    rcl_publisher_t publisher;
    Node& node;
    const rosidl_message_type_support_t* type_support;
    bool inited;

  public:
    Publisher(Node& node_ref, const rosidl_message_type_support_t* type, const char* topic) :
      node{ node_ref }, type_support{ type } {
        publisher = rcl_get_zero_initialized_publisher();
        rcl_ret_t rc = rclc_publisher_init_default(
          &publisher,
          node.get_node(),
          type_support,
          topic
        );
        if (rc == RCL_RET_OK) { inited = true; }
    }

    bool get_inited();
    bool publish(const T& msg);
};

template <typename T>
class Subscriber {
  private:
    rcl_subscription_t subscriber;
    Node& node;
    T msg_buf;
    void (*callback_func)(const T*);
    bool inited;

    static void callback_internal(const void* msg, void* ctx);
  
  public:
    Subscriber(Node& node_ref, const rosidl_message_type_support_t* type, const char* topic, void (*cb)(const T*)) :
      node{ node_ref }, callback_func{ cb } {
        subscriber = rcl_get_zero_initialized_subscription();
        rcl_ret_t rc = rclc_subscription_init_default(
            &subscriber, 
            node.get_node(), 
            type, 
            topic
        );
        if (rc != RCL_RET_OK) return;

        rc = rclc_executor_add_subscription_with_context(
            node.get_context()->get_executor(),
            &subscriber,
            &msg_buf,
            &Subscriber::callback_internal,
            this,
            ON_NEW_DATA
        );
        if (rc == RCL_RET_OK) { inited = true; }
    }

    bool get_inited();
};

template <typename T>
bool Publisher<T>::get_inited()
{
    return inited;
}

template <typename T>
inline bool Publisher<T>::publish(const T &msg)
{
  if (!inited) return false;
  return rcl_publish(&publisher, &msg, nullptr) == RCL_RET_OK;
}

template <typename T>
inline void Subscriber<T>::callback_internal(const void* msg, void* ctx)
{
  auto* self = static_cast<Subscriber<T>*>(ctx);
  if (msg && self && self->callback_func) {
    self->callback_func(static_cast<const T*>(msg));
  }
}

template <typename T>
bool Subscriber<T>::get_inited()
{
    return inited;
}
