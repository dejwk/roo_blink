#include "roo_blink/default_scheduler.h"

namespace roo_blink {

namespace {
roo_scheduler::SchedulingService default_scheduler;

roo::thread CreateSchedulerThread() {
  roo::thread::attributes attrs;
  attrs.set_name("roo_blink");
  attrs.set_stack_size(3096);
  attrs.set_priority(6);
  return roo::thread(attrs, []() {
    while (true) {
      default_scheduler.run();
    }
  });
}
}  // namespace

roo_scheduler::SchedulerClient& DefaultScheduler() {
  static roo::thread scheduler_thread = CreateSchedulerThread();
  return default_scheduler;
}

}  // namespace roo_blink