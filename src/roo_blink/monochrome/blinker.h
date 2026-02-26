#pragma once

#include <Arduino.h>

#include <vector>

#include "roo_blink/monochrome/led.h"
#include "roo_logging.h"
#include "roo_scheduler.h"
#include "roo_threads.h"
#include "roo_time.h"

namespace roo_blink {

/// Single step of a monochrome blink sequence.
class Step {
 public:
  friend constexpr Step TurnOn();
  friend constexpr Step TurnOff();
  friend constexpr Step FadeOn(roo_time::Duration duration);
  friend constexpr Step FadeOff(roo_time::Duration duration);

  friend constexpr Step SetTo(uint16_t level);
  friend constexpr Step FadeTo(uint16_t level, roo_time::Duration duration);
  friend constexpr Step Hold(roo_time::Duration duration);

 private:
  friend class Blinker;

  enum Type { kSet, kHold, kFade };

  constexpr Step(Type type, uint16_t target_level, uint16_t duration_millis);

  Type type_;
  uint16_t target_level_;
  uint16_t duration_millis_;
};

/// Sequence of steps for monochrome blinking.
class BlinkSequence {
 public:
  void add(Step step) { sequence_.push_back(std::move(step)); }

 private:
  std::vector<Step> sequence_;

  friend class Blinker;
};

/// Creates a step that sets the LED to the maximum brightness instantly.
constexpr Step TurnOn();

/// Creates a step that sets the LED to completely off instantly.
constexpr Step TurnOff();

/// Creates a step that sets the LED to the specified brightness instantly.
constexpr Step SetTo(uint16_t level);

/// Creates a step that fades linearly to the target level over the duration.
constexpr Step FadeTo(uint16_t level, roo_time::Duration duration);

/// Creates a step that fades linearly to the maximum brightness over duration.
constexpr Step FadeOn(roo_time::Duration duration);

/// Creates a step that fades linearly down to off over the duration.
constexpr Step FadeOff(roo_time::Duration duration);

/// Creates a step that maintains the current brightness for the duration.
constexpr Step Hold(roo_time::Duration duration);

/// Runs blink sequences on a monochrome LED.
class Blinker {
 public:
  /// Constructs a Blinker using the default scheduler.
  Blinker(Led& led);

  /// Constructs a Blinker using the specified scheduler.
  Blinker(Led& led, roo_scheduler::Scheduler& scheduler);

  /// Repeats the sequence indefinitely.
  void loop(BlinkSequence sequence);

  /// Repeats the sequence the specified number of times.
  void repeat(BlinkSequence sequence, int repetitions,
              uint16_t terminal_level = 0);

  /// Executes the sequence once.
  void execute(BlinkSequence sequence, uint16_t terminal_level = 0);

  /// Enables the LED at the specified intensity.
  void set(uint16_t intensity);

  /// Enables the LED at the maximum intensity.
  void turnOn();

  /// Disables the LED.
  void turnOff();

 private:
  void updateSequence(BlinkSequence sequence, int repetitions,
                      uint16_t terminal_level);
  void step();

  Led& led_;
  roo_scheduler::SingletonTask stepper_;
  std::vector<Step> sequence_;
  uint16_t current_level_;
  uint16_t terminal_level_;
  size_t repetitions_;
  size_t pos_;

  mutable roo::mutex mutex_;

  // For when hardware fading is not supported.
  bool fade_in_progress_;
  uint16_t fade_start_level_;
  uint16_t fade_target_level_;
  roo_time::Uptime fade_start_time_;
  roo_time::Uptime fade_end_time_;
};

/// Creates a symmetric blink sequence with optional ramp-up/down segments.
BlinkSequence Blink(roo_time::Duration period, int duty_percent = 50,
                    int rampup_percent_on = 0, int rampup_percent_off = 0);

// Implementation details.

constexpr Step::Step(Type type, uint16_t target_level, uint16_t duration_millis)
    : type_(type),
      target_level_(target_level),
      duration_millis_(duration_millis) {}

constexpr Step TurnOn() { return Step(Step::kSet, 65535, 0); }
constexpr Step TurnOff() { return Step(Step::kSet, 0, 0); }

constexpr Step SetTo(uint16_t level) { return Step(Step::kSet, level, 0); }

constexpr Step FadeTo(uint16_t level, roo_time::Duration duration) {
  return Step(Step::kFade, level, (uint16_t)duration.inMillis());
}

constexpr Step FadeOn(roo_time::Duration duration) {
  return FadeTo(65535, duration);
}

constexpr Step FadeOff(roo_time::Duration duration) {
  return FadeTo(0, duration);
}

constexpr Step Hold(roo_time::Duration duration) {
  return Step(Step::kHold, 0, (uint16_t)duration.inMillis());
}

}  // namespace roo_blink