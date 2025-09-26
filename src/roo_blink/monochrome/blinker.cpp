#include "roo_blink/monochrome/blinker.h"

#include "roo_blink.h"
#include "roo_logging.h"
#include "roo_threads.h"

using namespace roo_time;

namespace roo_blink {

namespace {
roo_scheduler::Scheduler default_scheduler;

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

roo_scheduler::Scheduler& DefaultScheduler() {
  static roo::thread scheduler_thread = CreateSchedulerThread();
  return default_scheduler;
}
}  // namespace

Blinker::Blinker(Led& led) : Blinker(led, DefaultScheduler()) {}

Blinker::Blinker(Led& led, roo_scheduler::Scheduler& scheduler)
    : led_(led),
      stepper_(scheduler, [this]() { step(); }),
      sequence_(),
      pos_(0) {}

void Blinker::loop(std::vector<Step> sequence) {
  updateSequence(std::move(sequence), -1, 0);
}

void Blinker::repeat(std::vector<Step> sequence, int repetitions,
                     uint16_t terminal_level) {
  updateSequence(std::move(sequence), repetitions - 1, terminal_level);
}

void Blinker::execute(std::vector<Step> sequence, uint16_t terminal_level) {
  updateSequence(std::move(sequence), 0, terminal_level);
}

void Blinker::set(uint16_t intensity) { updateSequence({}, 0, intensity); }

void Blinker::turnOn() { set(65535); }

void Blinker::turnOff() { set(0); }

void Blinker::updateSequence(std::vector<Step> sequence, int repetitions,
                             uint16_t terminal_level) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  sequence_ = std::move(sequence);
  terminal_level_ = terminal_level;
  repetitions_ = repetitions;
  pos_ = 0;
  if (!sequence_.empty() && !stepper_.is_scheduled()) {
    stepper_.scheduleNow(roo_scheduler::PRIORITY_ELEVATED);
  } else {
    led_.setLevel(terminal_level_);
  }
}

void Blinker::step() {
  roo::lock_guard<roo::mutex> lock(mutex_);
  uint16_t next_delay = 0;
  do {
    if (pos_ >= sequence_.size()) {
      sequence_.clear();
      led_.setLevel(terminal_level_);
      return;
    }
    const Step& s = sequence_[pos_];
    switch (s.type_) {
      case Step::kSet: {
        led_.setLevel(s.target_level_);
        break;
      }
      case Step::kHold: {
        next_delay = s.duration_millis_;
        break;
      }
      case Step::kFade:
      default: {
        led_.fade(s.target_level_, roo_time::Millis(s.duration_millis_));
        next_delay = s.duration_millis_;
      }
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

std::vector<Step> Blink(roo_time::Interval period, int duty_percent,
                        int rampup_percent_on, int rampup_percent_off) {
  CHECK_GE(duty_percent, 0);
  CHECK_LE(duty_percent, 100);
  CHECK_GE(rampup_percent_on, 0);
  CHECK_LE(rampup_percent_on, 100);
  CHECK_GE(rampup_percent_off, 0);
  CHECK_LE(rampup_percent_off, 100);
  int millis = period.inMillis();
  int millis_1st = duty_percent * millis / 100;
  int millis_1st_rampup = rampup_percent_on * millis_1st / 100;
  int millis_2nd = millis - millis_1st;
  int millis_2nd_rampup = rampup_percent_off * millis_2nd / 100;

  std::vector<Step> result;
  result.reserve(4);

  if (millis_1st_rampup > 0) {
    result.push_back(FadeOn(Millis(millis_1st_rampup)));
  } else {
    result.push_back(TurnOn());
  }
  if (millis_1st_rampup < millis_1st) {
    result.push_back(Hold(Millis(millis_1st - millis_1st_rampup)));
  }

  if (millis_2nd_rampup > 0) {
    result.push_back(FadeOff(Millis(millis_2nd_rampup)));
  } else {
    result.push_back(TurnOff());
  }
  if (millis_2nd_rampup < millis_2nd) {
    result.push_back(Hold(Millis(millis_2nd - millis_2nd_rampup)));
  }

  return result;
}

}  // namespace roo_blink