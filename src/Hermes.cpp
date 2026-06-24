#include <Hermes.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float32.h>

bool Context::init(size_t handles)
{
    allocator = rcl_get_default_allocator();

    rcl_ret_t rc = rclc_support_init(&support, 0, nullptr, &allocator);
    if (rc != RCL_RET_OK) { return false; }

    rc = rclc_executor_init(&executor, &support.context, handles, &allocator);
    if (rc != RCL_RET_OK) { return false; }

    inited = true;
    return true;
}

void Context::spin(unsigned long timeout)
{
    if (!inited) { return; }
    rclc_executor_spin_some(&executor, timeout);
}

bool Context::get_inited()
{
    return inited;
}

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

bool Node::get_inited()
{
    return inited;
}

bool Timer::start()
{
  rcl_ret_t rc = rcl_timer_reset(&timer);
  return (rc == RCL_RET_OK);
}

bool Timer::stop()
{
  rcl_ret_t rc = rcl_timer_cancel(&timer);
  return (rc == RCL_RET_OK);
}

bool Timer::get_inited()
{
    return inited;
}

// hack to fix compilation on pio
void setup() {}
void loop() {}