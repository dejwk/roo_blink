load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "VoltageTrace",
    sketch = "VoltageTrace.ino",
    visibility = ["//visibility:public"],
    deps = [
        "//:roo_blink",
        "@roo_testing//roo_testing/microcontrollers/esp32:core",
        "@roo_testing//roo_testing/system:timer",
        "@roo_testing//roo_testing/transducers/voltage:voltage",
    ],
)
