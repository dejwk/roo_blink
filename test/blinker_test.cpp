#include <vector>

#include "gtest/gtest.h"
#include "roo_blink.h"
#include "roo_scheduler.h"
#include "roo_time.h"

namespace roo_blink {
namespace {

using roo_time::Duration;
using roo_time::Micros;
using roo_time::Millis;
using roo_time::Seconds;

class RecordingLed : public Led {
 public:
  void setLevel(uint16_t level) override { levels.push_back(level); }

  bool fade(uint16_t, Duration duration) override {
    fades.push_back(duration);
    return true;
  }

  std::vector<uint16_t> levels;
  std::vector<Duration> fades;
};

class RecordingRgbLed : public RgbLed {
 public:
  void setColor(Color color) override { colors.push_back(color); }

  std::vector<Color> colors;
};

// Verifies that an instantaneous loop stops an active pattern and clears its
// timer.
TEST(BlinkerSequence, MonochromeLoopWithoutDelayTurnsOff) {
  roo_scheduler::SchedulingService scheduler;
  RecordingLed led;
  Blinker blinker(led, scheduler);
  blinker.loop(Blink(Seconds(10)));
  scheduler.executeEligibleTasks(1);
  ASSERT_EQ(led.levels, (std::vector<uint16_t>{65535}));

  BlinkSequence instant;
  instant.add(TurnOn());
  instant.add(Hold(Millis(-1)));
  instant.add(FadeOff(Micros(999)));
  blinker.loop(instant);
  ASSERT_TRUE(scheduler.empty());
  EXPECT_EQ(led.levels, (std::vector<uint16_t>{65535, 0}));
}

// Verifies that an instantaneous RGB loop turns the LED off without queuing
// work.
TEST(BlinkerSequence, RgbLoopWithoutDelayTurnsOff) {
  roo_scheduler::SchedulingService scheduler;
  RecordingRgbLed led;
  RgbBlinker blinker(led, scheduler);
  blinker.setColor(Color(255, 0, 0));

  RgbBlinkSequence instant;
  instant.add(RgbSetTo(Color(0, 255, 0)));
  instant.add(RgbHold(Millis(0)));
  instant.add(RgbFadeOff(Millis(0)));
  blinker.loop(instant);
  ASSERT_TRUE(scheduler.empty());
  ASSERT_EQ(led.colors.size(), 2u);
  EXPECT_EQ(led.colors.back().r(), 0);
  EXPECT_EQ(led.colors.back().g(), 0);
  EXPECT_EQ(led.colors.back().b(), 0);
}

// Verifies that replacing a waiting pattern dispatches the new first step
// immediately.
TEST(BlinkerSequence, MonochromeReplacementStartsDuringExistingHold) {
  roo_scheduler::SchedulingService scheduler;
  RecordingLed led;
  Blinker blinker(led, scheduler);
  BlinkSequence active;
  active.add(SetTo(100));
  active.add(Hold(Seconds(10)));
  blinker.loop(active);
  scheduler.executeEligibleTasks(1);
  ASSERT_EQ(led.levels, (std::vector<uint16_t>{100}));

  BlinkSequence replacement;
  replacement.add(SetTo(200));
  replacement.add(Hold(Seconds(10)));
  blinker.execute(replacement, 300);
  EXPECT_EQ(led.levels, (std::vector<uint16_t>{100}));
  scheduler.executeEligibleTasks(1);
  EXPECT_EQ(led.levels, (std::vector<uint16_t>{100, 200}));
}

// Verifies that RGB replacement preserves the current output until the next
// step runs.
TEST(BlinkerSequence, RgbReplacementStartsDuringExistingHold) {
  roo_scheduler::SchedulingService scheduler;
  RecordingRgbLed led;
  RgbBlinker blinker(led, scheduler);
  blinker.loop(RgbBlink(Seconds(10), Color(255, 0, 0)));
  scheduler.executeEligibleTasks(1);
  ASSERT_EQ(led.colors.size(), 1u);
  EXPECT_EQ(led.colors.front().r(), 255);

  RgbBlinkSequence replacement;
  replacement.add(RgbSetTo(Color(0, 255, 0)));
  replacement.add(RgbHold(Seconds(10)));
  blinker.execute(replacement, Color(0, 0, 255));
  EXPECT_EQ(led.colors.size(), 1u);
  scheduler.executeEligibleTasks(1);
  ASSERT_EQ(led.colors.size(), 2u);
  EXPECT_EQ(led.colors.back().g(), 255);
}

// Verifies that a finite RGB sequence writes the terminal color at completion.
TEST(BlinkerSequence, RgbCompletionAppliesTerminalColor) {
  roo_scheduler::SchedulingService scheduler;
  RecordingRgbLed led;
  RgbBlinker blinker(led, scheduler);
  RgbBlinkSequence sequence;
  sequence.add(RgbFadeTo(Color(255, 0, 0), Millis(0)));
  blinker.repeat(sequence, 2, Color(0, 255, 0));
  scheduler.executeEligibleTasks();

  ASSERT_EQ(led.colors.size(), 3u);
  EXPECT_EQ(led.colors[0].r(), 255);
  EXPECT_EQ(led.colors[1].r(), 255);
  EXPECT_EQ(led.colors[2].r(), 0);
  EXPECT_EQ(led.colors[2].g(), 255);
  EXPECT_TRUE(scheduler.empty());
}

// Verifies that zero repetitions apply the terminal output and cancel pending
// work.
TEST(BlinkerSequence, ZeroRepetitionsStopExistingPattern) {
  roo_scheduler::SchedulingService scheduler;
  RecordingLed led;
  Blinker blinker(led, scheduler);
  const BlinkSequence pattern = Blink(Seconds(10));
  blinker.loop(pattern);
  blinker.repeat(pattern, 0, 100);

  EXPECT_TRUE(scheduler.empty());
  scheduler.executeEligibleTasks();
  EXPECT_EQ(led.levels, (std::vector<uint16_t>{100}));
}

}  // namespace
}  // namespace roo_blink
