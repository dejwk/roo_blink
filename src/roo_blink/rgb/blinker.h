#pragma once

#include <Arduino.h>

#include <vector>

#include "roo_blink/rgb/led.h"
#include "roo_logging.h"
#include "roo_scheduler.h"
#include "roo_time.h"

namespace roo_blink {

class RgbStep {
 public:
  friend constexpr RgbStep RgbSetTo(Color color);
  friend constexpr RgbStep RgbHold(roo_time::Interval duration);
  friend constexpr RgbStep RgbTurnOff();
  friend constexpr RgbStep RgbFadeTo(Color color, roo_time::Interval duration);
  friend constexpr RgbStep RgbFadeOff(roo_time::Interval duration);

 private:
  friend class RgbBlinker;

  enum Type { kSet, kHold, kFade };

  constexpr RgbStep(Type type, Color color, uint16_t duration_millis);

  Type type_;
  Color target_color_;
  uint16_t duration_millis_;
};

class RgbBlinkSequence {
 public:
  void add(RgbStep step) { sequence_.push_back(std::move(step)); }

 private:
  std::vector<RgbStep> sequence_;

  friend class RgbBlinker;
};

// Creates a step that sets the LED to the the specified color instantly.
constexpr RgbStep RgbSetTo(Color color);

// Creates a step that disables the LED. Equivalent to RgbSetTo(Color()).
constexpr RgbStep RgbTurnOff();

// Creates a step that gradually transforms the color to the specified target
// color, over the specified time interval.
constexpr RgbStep RgbFadeTo(Color color, roo_time::Interval duration);

// Creates a step that gradually fades the LED off, over the specified time
// interval.
constexpr RgbStep RgbFadeOff(roo_time::Interval duration);

// Creates a step that maintains the current brightness for the specified
// duration.
constexpr RgbStep RgbHold(roo_time::Interval duration);

class RgbBlinker {
 public:
  // Constructs a RgbBlinker that controls the specified LED, using a default
  // scheduler to schedule the LED updates.
  RgbBlinker(RgbLed& led);

  // Constructs a RgbBlinker that controls the specified LED, using the
  // specified scheduler to schedule the LED updates.
  RgbBlinker(RgbLed& led, roo_scheduler::Scheduler& scheduler);

  // Infinite loop of steps.
  void loop(RgbBlinkSequence sequence);

  // Specified count of repetitions of steps.
  void repeat(RgbBlinkSequence sequence, int repetitions,
              Color terminal_color = Color());

  // Executes the sequence once.
  void execute(RgbBlinkSequence sequence, Color terminal_color = Color());

  // Enables the LED, setting it to the specified color.
  void setColor(Color color);

  // Disables the LED.
  void turnOff();

 private:
  void updateSequence(RgbBlinkSequence sequence, int repetitions,
                      Color terminal_color);
  void step();

  RgbLed& led_;
  roo_scheduler::SingletonTask stepper_;
  std::vector<RgbStep> sequence_;
  Color current_color_;
  Color terminal_color_;
  size_t repetitions_;
  size_t pos_;

  mutable roo::mutex mutex_;

  bool fade_in_progress_;
  Color fade_start_color_;
  Color fade_target_color_;
  roo_time::Uptime fade_start_time_;
  roo_time::Uptime fade_end_time_;
};

RgbBlinkSequence RgbBlink(roo_time::Interval period, Color color,
                          int duty_percent = 50, int rampup_percent_on = 0,
                          int rampup_percent_off = 0);

// Implementation details.

constexpr RgbStep::RgbStep(Type type, Color color, uint16_t duration_millis)
    : type_(type), target_color_(color), duration_millis_(duration_millis) {}

constexpr RgbStep RgbSetTo(Color color) {
  return RgbStep(RgbStep::kSet, color, 0);
}

constexpr RgbStep RgbTurnOff() { return RgbSetTo(Color()); }

constexpr RgbStep RgbHold(roo_time::Interval duration) {
  return RgbStep(RgbStep::kHold, Color(), (uint16_t)duration.inMillis());
}

constexpr RgbStep RgbFadeTo(Color color, roo_time::Interval duration) {
  return RgbStep(RgbStep::kFade, color, (uint16_t)duration.inMillis());
}

constexpr RgbStep RgbFadeOff(roo_time::Interval duration) {
  return RgbFadeTo(Color(), duration);
}

}  // namespace roo_blink