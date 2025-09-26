#pragma once

#include <Arduino.h>

#include <vector>

#include "roo_blink/monochrome/led.h"
#include "roo_logging.h"
#include "roo_scheduler.h"
#include "roo_threads.h"
#include "roo_time.h"

namespace roo_blink {

class Step {
 public:
  friend constexpr Step TurnOn();
  friend constexpr Step TurnOff();
  friend constexpr Step FadeOn(roo_time::Interval duration);
  friend constexpr Step FadeOff(roo_time::Interval duration);

  friend constexpr Step SetTo(uint16_t level);
  friend constexpr Step FadeTo(uint16_t level, roo_time::Interval duration);
  friend constexpr Step Hold(roo_time::Interval duration);

 private:
  friend class Blinker;

  enum Type { kSet, kHold, kFade };

  constexpr Step(Type type, uint16_t target_level, uint16_t duration_millis);

  Type type_;
  uint16_t target_level_;
  uint16_t duration_millis_;
};

class BlinkSequence {
 public:
  void add(Step step) { sequence_.push_back(std::move(step)); }

 private:
  std::vector<Step> sequence_;

  friend class Blinker;
};

// Creates a step that sets the LED to the maximum brightness instantly.
constexpr Step TurnOn();

// Creates a step that sets the LED to completely off instantly.
constexpr Step TurnOff();

// Creates a step that sets the LED to the specifed brightness level instantly.
constexpr Step SetTo(uint16_t level);

// Creates a step that fades the brightness linearly to the specified brighness
// level, over the specified time interval.
constexpr Step FadeTo(uint16_t level, roo_time::Interval duration);

// Creates a step that fades the brightness linearly to the maximum brighness,
// over the specified time interval.
constexpr Step FadeOn(roo_time::Interval duration);

// Creates a step that fades the brightness linearly down to off, over the
// specified time interval.
constexpr Step FadeOff(roo_time::Interval duration);

// Creates a step that maintains the current brightness for the specified
// duration.
constexpr Step Hold(roo_time::Interval duration);

class Blinker {
 public:
  // Constructs a Blinker that controls the specified LED, using a default
  // scheduler to schedule the LED updates.
  Blinker(Led& led);

  // Constructs a Blinker that controls the specified LED, using the specified
  // scheduler to schedule the LED updates.
  Blinker(Led& led, roo_scheduler::Scheduler& scheduler);

  // Infinite loop of steps.
  void loop(BlinkSequence sequence);

  // Specified count of repetitions of steps.
  void repeat(BlinkSequence sequence, int repetitions,
              uint16_t terminal_level = 0);

  // Executes the sequence once.
  void execute(BlinkSequence sequence, uint16_t terminal_level = 0);

  // Enables the LED at the specified intensity.
  void set(uint16_t intensity);

  // Enables the LED at the maximum intensity.
  void turnOn();

  // Disables the LED.
  void turnOff();

 private:
  void updateSequence(BlinkSequence sequence, int repetitions,
                      uint16_t terminal_level);
  void step();

  Led& led_;
  roo_scheduler::SingletonTask stepper_;
  std::vector<Step> sequence_;
  uint16_t terminal_level_;
  size_t repetitions_;
  size_t pos_;

  mutable roo::mutex mutex_;
};

BlinkSequence Blink(roo_time::Interval period, int duty_percent = 50,
                    int rampup_percent_on = 0, int rampup_percent_off = 0);

// Implementation details.

constexpr Step::Step(Type type, uint16_t target_level, uint16_t duration_millis)
    : type_(type),
      target_level_(target_level),
      duration_millis_(duration_millis) {}

constexpr Step TurnOn() { return Step(Step::kSet, 65535, 0); }
constexpr Step TurnOff() { return Step(Step::kSet, 0, 0); }

constexpr Step SetTo(uint16_t level) { return Step(Step::kSet, level, 0); }

constexpr Step FadeTo(uint16_t level, roo_time::Interval duration) {
  return Step(Step::kFade, level, (uint16_t)duration.inMillis());
}

constexpr Step FadeOn(roo_time::Interval duration) {
  return FadeTo(65535, duration);
}

constexpr Step FadeOff(roo_time::Interval duration) {
  return FadeTo(0, duration);
}

constexpr Step Hold(roo_time::Interval duration) {
  return Step(Step::kHold, 0, (uint16_t)duration.inMillis());
}

}  // namespace roo_blink