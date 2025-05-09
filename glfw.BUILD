cc_library(
    name = "glfw-headers",
    hdrs = [
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
    ],
    includes = ["."],
    strip_include_prefix = "include",
    visibility = [
        "//visibility:private",
    ],
)

DARWIN_HDRS = [
    "src/cocoa_joystick.h",
    "src/cocoa_platform.h",
    "src/cocoa_time.h",
    # "src/nsgl_context.h",
    "src/posix_thread.h",
    "src/wl_platform.h",
]

DARWIN_SRCS = [
    "src/cocoa_time.c",
    "src/posix_thread.c",
    "src/posix_module.c",
    "src/cocoa_init.m",
    "src/cocoa_joystick.m",
    "src/cocoa_monitor.m",
    "src/cocoa_window.m",
    "src/nsgl_context.m",
]

COMMON_HDRS = [
    "include/GLFW/glfw3.h",
    "include/GLFW/glfw3native.h",
    "src/platform.h",
    "src/internal.h",
    "src/mappings.h",
    "src/xkb_unicode.h",
    "src/null_platform.h",
    "src/null_joystick.h",
]

COMMON_SRCS = [
    "src/context.c",
    "src/egl_context.c",
    "src/null_init.c",
    "src/null_joystick.c",
    "src/null_window.c",
    "src/null_monitor.c",
    "src/init.c",
    "src/input.c",
    "src/platform.c",
    "src/osmesa_context.c",
    "src/monitor.c",
    "src/vulkan.c",
    "src/window.c",
    "src/xkb_unicode.c",
    "src/glx_context.c",
]

LINUX_HDRS = [
    "src/linux_joystick.h",
    "src/posix_thread.h",
    "src/posix_time.h",
    "src/x11_platform.h",
]

LINUX_SRCS = [
    "src/linux_joystick.c",
    "src/posix_thread.c",
    "src/posix_time.c",
    "src/x11_init.c",
    "src/x11_monitor.c",
    "src/x11_window.c",
]

objc_library(
    name = "glfw-cocoa",
    srcs = COMMON_SRCS + DARWIN_SRCS,
    hdrs = COMMON_HDRS + DARWIN_HDRS,
    copts = [
        "-fno-objc-arc",
    ],
    defines = [
        "_GLFW_COCOA",
        "GLFW_INVALID_CODEPOINT",
    ],
    linkopts = [
        "-framework OpenGL",
        "-framework Cocoa",
        "-framework IOKit",
        "-framework CoreFoundation",
    ],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "glfw-linux",
    srcs = COMMON_SRCS + LINUX_SRCS,
    hdrs = COMMON_HDRS + LINUX_HDRS,
    defines = ["_GLFW_X11"],
    linkopts = ["-lX11"],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "glfw",
    visibility = ["//visibility:public"],
    deps = select({
        "@bazel_tools//src/conditions:linux_x86_64": [
            ":glfw-headers",
            ":glfw-linux",
        ],
        "@bazel_tools//src/conditions:darwin": [
            ":glfw-cocoa",
            ":glfw-headers",
        ],
    }),
)