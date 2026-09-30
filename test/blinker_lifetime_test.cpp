#include <atomic>
#include <memory>

#include "gtest/gtest.h"
#include "roo_blink/monochrome/blinker.h"
#include "roo_blink/rgb/blinker.h"
#include "roo_threads/semaphore.h"

namespace roo_blink {
using namespace roo_time;

struct TestLed : Led {
  int calls = 0;
  void setLevel(uint16_t) override { ++calls; }
  bool fade(uint16_t, Duration) override { return false; }
};
struct TestRgbLed : RgbLed {
  int calls = 0;
  void setColor(Color) override { ++calls; }
};

TEST(BlinkerLifetime, MonochromeShutdownPreventsFurtherWork) {
  roo_scheduler::SchedulingService scheduler;
  TestLed led;
  Blinker blinker(led, scheduler);
  blinker.loop(Blink(Seconds(1)));
  scheduler.executeEligibleTasks(1);
  EXPECT_TRUE(blinker.shutdown());
  const int before = led.calls;
  blinker.loop(Blink(Seconds(1)));
  blinker.turnOn();
  scheduler.executeEligibleTasks();
  EXPECT_TRUE(scheduler.empty());
  EXPECT_EQ(led.calls, before);
}

TEST(BlinkerLifetime, RgbShutdownPreventsFurtherWork) {
  roo_scheduler::SchedulingService scheduler;
  TestRgbLed led;
  RgbBlinker blinker(led, scheduler);
  blinker.loop(RgbBlink(Seconds(1), Color(255, 0, 0)));
  scheduler.executeEligibleTasks(1);
  EXPECT_TRUE(blinker.shutdown());
  const int before = led.calls;
  blinker.loop(RgbBlink(Seconds(1), Color(0, 255, 0)));
  blinker.turnOff();
  scheduler.executeEligibleTasks();
  EXPECT_TRUE(scheduler.empty());
  EXPECT_EQ(led.calls, before);
}

TEST(BlinkerLifetime, DestructionCancelsReadyStepper) {
  roo_scheduler::SchedulingService scheduler;
  TestLed led;
  {
    Blinker blinker(led, scheduler);
    blinker.loop(Blink(Seconds(1)));
    scheduler.executeEligibleTasks(roo_scheduler::Priority::kMaximum);
  }
  scheduler.executeEligibleTasks();
  EXPECT_EQ(led.calls, 0);
  EXPECT_TRUE(scheduler.empty());
}

TEST(BlinkerLifetime, DestructionWaitsForRunningStep) {
  struct BlockingLed : Led {
    roo::binary_semaphore entered{0}, release{0};
    std::atomic<bool> returned{false};
    void setLevel(uint16_t) override {
      entered.release();
      release.acquire();
      returned = true;
    }
    bool fade(uint16_t, Duration) override { return false; }
  } led;
  roo_scheduler::SchedulingService scheduler;
  auto blinker = std::unique_ptr<Blinker>(new Blinker(led, scheduler));
  blinker->loop(Blink(Seconds(1)));
  roo::thread dispatcher([&] { scheduler.executeEligibleTasks(1); });
  led.entered.acquire();
  roo::binary_semaphore deleting(0);
  roo::thread destroyer([&] {
    deleting.release();
    blinker.reset();
    EXPECT_TRUE(led.returned.load());
  });
  deleting.acquire();
  led.release.release();
  dispatcher.join();
  destroyer.join();
  EXPECT_TRUE(scheduler.empty());
}
}  // namespace roo_blink

namespace roo_blink {
TEST(BlinkerLifetime, SelfShutdownDoesNotLockAnimationMutexAgain) {
  struct SelfStoppingLed : Led {
    Blinker* blinker = nullptr;
    bool result = true;
    void setLevel(uint16_t) override { result = blinker->shutdown(); }
    bool fade(uint16_t, roo_time::Duration) override { return false; }
  } led;
  roo_scheduler::SchedulingService scheduler;
  Blinker blinker(led, scheduler);
  led.blinker = &blinker;
  blinker.loop(Blink(roo_time::Seconds(1)));
  scheduler.executeEligibleTasks(1);
  EXPECT_FALSE(led.result);
  EXPECT_TRUE(blinker.shutdown());
  EXPECT_TRUE(scheduler.empty());
}
}  // namespace roo_blink
