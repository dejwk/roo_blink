load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

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
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/cores/esp32",
        "@roo_time",
    ],
)


cc_test(
    name = "blinker_lifetime_test",
    srcs = ["test/blinker_lifetime_test.cpp"],
    size = "small",
    linkstatic = 1,
    deps = [
        ":roo_blink",
        "@googletest//:gtest",
        "@roo_scheduler",
        "@roo_threads",
        "@roo_testing//:arduino_gtest_main",
    ],
)
