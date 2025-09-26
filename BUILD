cc_library(
    name = "roo_blink",
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_logging",
        "@roo_scheduler",
        "@roo_time",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/cores/esp32",
    ],
)
