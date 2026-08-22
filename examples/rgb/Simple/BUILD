load("@roo_testing//roo_testing/emulation:arduino.bzl", "roo_arduino_example")

roo_arduino_example(
    name = "Simple",
    sketch = "Simple.ino",
    visibility = ["//visibility:public"],
    deps = [
        "//:roo_blink",
        "//examples/rgb/emulation:adafruit_neopixel",
    ],
)
