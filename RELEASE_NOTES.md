# roo_blink 1.2.0

- Upgrade `roo_scheduler` to 2.3.0.
- Update `Blinker` and `RgbBlinker` constructors to accept `roo_scheduler::SchedulerClient&`; `DefaultScheduler()` now returns this interface.
- Migrate the default scheduler, example, and tests from the deprecated `Scheduler` type to `SchedulingService`.

---

# roo_blink 1.1.1

- Upgrade dependencies: `roo_logging` to 1.5.11, `roo_scheduler` to 2.2.1, `roo_threads` to 1.2.9, and `roo_time` to 2.0.1.
- Upgrade `roo_testing` to 2.3.0.
- Improve Bazel tooling with automatic ESP-IDF profile selection for example runs and a helper to test both Arduino and ESP-IDF ESP32 profiles.

---

# roo_blink 1.1.0

- Fixed `Blinker` and `RgbBlinker` destruction to stop scheduling and wait for callbacks before tearing down animation state.
- Added permanent `shutdown()` support. Subsequent updates are ignored; the last LED output is preserved, so call `turnOff()` first if needed.
- Added lifecycle regression tests and documented threading and shutdown requirements.
- Updated dependencies to `roo_scheduler` 2.2.0, `roo_time` 2.0.0, `roo_logging` 1.5.10, and `roo_threads` 1.2.8.
- Updated Bazel tooling and CI dependencies to `rules_cc` 0.2.25, GoogleTest 1.18.0.bcr.1, and `roo_testing` 2.1.2.
- Added consolidated release history.

---

# [roo_blink 1.0.7](https://github.com/dejwk/roo_blink/releases/tag/1.0.7)

Published 2026-08-29.

his release improves ESP32 LED behavior and makes every example runnable under host emulation.

### Highlights

- Added the `VoltageTrace` monochrome example, which prints CSV voltage/PWM measurements during emulated runs.
- Made all monochrome and RGB examples buildable/runnable with the Arduino ESP32 host-emulation profile.
- Updated CI to use `roo_testing` 2.0 profiles and modern GitHub Actions configuration.
- Centralized AddressSanitizer configuration and added a Bazelisk wrapper for the vendored test tooling.
- Updated dependencies, including `roo_testing` 2.1.0.

### ESP32 fixes and API changes

- Fixed ESP32 `Blinker` use during static initialization: the LEDC fade service now initializes lazily from a running FreeRTOS task.
- `GpioLed::fade()` now returns `false` when ESP-IDF fade setup or start fails.
- Corrected LED brightness duty-cycle rounding and initial output state.
- Changed the default `GpioLed` polarity from `ON_LOW` to `ON_HIGH`. Existing active-low LED setups should now pass `GpioLed::ON_LOW` explicitly.
- Made `RgbLed::setColor()` pure virtual, requiring concrete RGB LED implementations to provide it.

### Host emulation

Run the basic example with:

```sh
bazel run //examples/monochrome/Trivial:Trivial
```

Run the terminal-visible voltage trace with:

```sh
bazel run //examples/monochrome/VoltageTrace:VoltageTrace
```

Full changes: [[1.0.6...1.0.7](https://github.com/dejwk/roo_blink/compare/1.0.6...1.0.7)](https://github.com/dejwk/roo_blink/compare/1.0.6...1.0.7)

---

# [roo_blink 1.0.6](https://github.com/dejwk/roo_blink/releases/tag/1.0.6)

Published 2026-02-26.

* Generated doxygen documentation.
* Updated dependencies.
* Cleaned compiler warnings.

---

# [roo_blink 1.0.5](https://github.com/dejwk/roo_blink/releases/tag/1.0.5)

Published 2026-01-26.

Fixed tests after change in Bazel behavior.

**Full Changelog**: https://github.com/dejwk/roo_blink/compare/1.0.4...1.0.5

---

# [roo_blink 1.0.4](https://github.com/dejwk/roo_blink/releases/tag/1.0.4)

Published 2026-01-06.

Updated dependencies.

---

# [roo_blink 1.0.3](https://github.com/dejwk/roo_blink/releases/tag/1.0.3)

Published 2026-01-06.

Updated dependencies.

**Full Changelog**: https://github.com/dejwk/roo_blink/compare/1.0.2...1.0.3

---

# [roo_blink 1.0.2](https://github.com/dejwk/roo_blink/releases/tag/1.0.2)

Published 2025-11-12.

Updated dependencies only.

**Full Changelog**: https://github.com/dejwk/roo_blink/compare/1.0.1...1.0.2

---

# [roo_blink 1.0.1](https://github.com/dejwk/roo_blink/releases/tag/1.0.1)

Published 2025-10-31.

Basic CI, .gitignore; refreshed dependencies.

---

# [roo_blink 1.0.0](https://github.com/dejwk/roo_blink/releases/tag/1.0.0)

Published 2025-09-26.

Initial release.

---

