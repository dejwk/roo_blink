#include "roo_blink/rgb/blinker.h"

#include "roo_blink.h"
#include "roo_blink/default_scheduler.h"
#include "roo_logging.h"

using namespace roo_time;

namespace roo_blink {

RgbBlinker::RgbBlinker(RgbLed& led) : RgbBlinker(led, DefaultScheduler()) {}

RgbBlinker::RgbBlinker(RgbLed& led, roo_scheduler::Scheduler& scheduler)
    : led_(led),
      stepper_(scheduler, [this]() { step(); }),
      sequence_(),
      pos_(0) {}

void RgbBlinker::loop(RgbBlinkSequence sequence) {
  updateSequence(std::move(sequence), -1, Color());
}

void RgbBlinker::repeat(RgbBlinkSequence sequence, int repetitions, Color terminal_color) {
  updateSequence(std::move(sequence), repetitions - 1, terminal_color);
}

void RgbBlinker::execute(RgbBlinkSequence sequence, Color terminal_color) {
  updateSequence(std::move(sequence), 0, terminal_color);
}

void RgbBlinker::setColor(Color color) {
  updateSequence({}, 0, color);
}

void RgbBlinker::turnOff() {
  updateSequence({}, 0, Color());
}

void RgbBlinker::updateSequence(RgbBlinkSequence sequence, int repetitions,
                                Color terminal_color) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  sequence_ = std::move(sequence.sequence_);
  terminal_color_ = terminal_color;
  current_color_ = terminal_color;
  repetitions_ = repetitions;
  pos_ = 0;
  if (!sequence_.empty() && !stepper_.is_scheduled()) {
    stepper_.scheduleNow(roo_scheduler::PRIORITY_ELEVATED);
  } else {
    current_color_ = terminal_color;
    led_.setColor(terminal_color_);
  }
}

void RgbBlinker::step() {
  roo::lock_guard<roo::mutex> lock(mutex_);
  uint16_t next_delay = 0;
  do {
    if (pos_ >= sequence_.size()) {
      sequence_.clear();
      return;
    }
    const RgbStep& s = sequence_[pos_];
    switch (s.type_) {
      case RgbStep::kSet: {
        led_.setColor(s.color_);
        break;
      }
      case RgbStep::kHold:
      default: {
        next_delay = s.duration_millis_;
        break;
      }
        //   case RgbStep::kFade:
        //   default: {
        //     led_.fade(s.target_level_, roo_time::Millis(s.duration_millis_));
        //     next_delay = s.duration_millis_;
        //   }
    }
    ++pos_;
    if (pos_ == sequence_.size() && repetitions_ != 0) {
      if (repetitions_ > 0) --repetitions_;
      pos_ = 0;
    }
  } while (next_delay == 0);
  stepper_.scheduleAfter(roo_time::Millis(next_delay),
                         roo_scheduler::PRIORITY_ELEVATED);
}

RgbBlinkSequence RgbBlink(roo_time::Interval period, Color color,
                          int duty_percent) {
  CHECK_GE(duty_percent, 0);
  CHECK_LE(duty_percent, 100);
  int millis = period.inMillis();
  int millis_1st = duty_percent * millis / 100;
  int millis_2nd = millis - millis_1st;

  RgbBlinkSequence result;

  result.add(RgbSetTo(color));
  result.add(RgbHold(Millis(millis_1st)));

  result.add(RgbTurnOff());
  result.add(RgbHold(Millis(millis_2nd)));
  return result;
}

}  // namespace roo_blink
