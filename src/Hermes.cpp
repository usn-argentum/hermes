#include <Hermes.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float32.h>

bool Context::init(size_t handles, size_t domain_id)
{
    allocator = rcl_get_default_allocator();    

    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    
    rcl_ret_t rc;
    //rcl_ret_t rc = rcl_init_options_init(&init_options, allocator);
    //if (rc != RCL_RET_OK) { return false; }

    //rc = rcl_init_options_set_domain_id(&init_options, domain_id);
    //if (rc != RCL_RET_OK) { return false; }
    
    //rc = rclc_support_init_with_options(&support, 0, nullptr, &init_options, &allocator);
    rc = rclc_support_init(&support, 0, nullptr, &allocator);
    if (rc != RCL_RET_OK) { return false; }
    
    rc = rclc_executor_init(&executor, &support.context, handles, &allocator);
    if (rc != RCL_RET_OK) { return false; }

    rc = rcl_init_options_fini(&init_options);

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

#ifdef BUILDING_LOCAL_TEST
    // pio compilation fix dont upload this it wont do anything
    void setup() {}
    void loop() {}
#endif