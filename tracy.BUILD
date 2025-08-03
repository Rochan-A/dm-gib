load("@bazel_skylib//rules:common_settings.bzl", "bool_flag")
load("@rules_cc//cc:defs.bzl", "cc_library")

bool_flag(
    name = "enable_tracy",
    build_setting_default = False,
)

config_setting(
    name = "do_enable_tracy",
    flag_values = {":enable_tracy": "True"},
)

_TRACY_SOURCE_FILES = [
    "public/client/tracy_rpmalloc.hpp",
    "public/client/TracyArmCpuTable.hpp",
    "public/client/TracyCallstack.hpp",
    "public/client/TracyCpuid.hpp",
    "public/client/TracyDebug.hpp",
    "public/client/TracyDxt1.hpp",
    "public/client/TracyFastVector.hpp",
    "public/client/TracyKCore.hpp",
    "public/client/TracyLock.hpp",
    "public/client/TracyProfiler.hpp",
    "public/client/TracyRingBuffer.hpp",
    "public/client/TracyScoped.hpp",
    "public/client/TracyStringHelpers.hpp",
    "public/client/TracySysPower.hpp",
    "public/client/TracySysTime.hpp",
    "public/client/TracySysTrace.hpp",
    "public/client/TracyThread.hpp",
    "public/common/tracy_lz4.hpp",
    "public/common/tracy_lz4hc.hpp",
    "public/common/TracyAlign.hpp",
    "public/common/TracyAlloc.hpp",
    "public/common/TracyColor.hpp",
    "public/common/TracyForceInline.hpp",
    "public/common/TracyMutex.hpp",
    "public/common/TracyProtocol.hpp",
    "public/common/TracyQueue.hpp",
    "public/common/TracySocket.hpp",
    "public/common/TracyStackFrames.hpp",
    "public/common/TracySystem.hpp",
    "public/common/TracyUwp.hpp",
    "public/common/TracyVersion.hpp",
    "public/common/TracyYield.hpp",
    "public/libbacktrace/backtrace.hpp",
    "public/libbacktrace/filenames.hpp",
    "public/libbacktrace/internal.hpp",
    "public/tracy/TracyD3D11.hpp",
    "public/tracy/TracyD3D12.hpp",
    "public/tracy/TracyLua.hpp",
    "public/tracy/TracyOpenCL.hpp",
    "public/tracy/TracyOpenGL.hpp",
]


_TRACY_HEADER_FILES = [
    "public/client/TracyAlloc.cpp",
    "public/client/TracyCallstack.cpp",
    "public/client/TracyDxt1.cpp",
    "public/client/TracyOverride.cpp",
    "public/client/TracyProfiler.cpp",
    "public/client/TracySysPower.cpp",
    "public/client/TracySysTime.cpp",
    "public/client/TracySysTrace.cpp",
    "public/client/tracy_rpmalloc.cpp",
    "public/client/TracyKCore.cpp",
    "public/common/TracySocket.cpp",
    "public/common/TracyStackFrames.cpp",
    "public/common/TracySystem.cpp",
    "public/common/tracy_lz4.cpp",
    "public/common/tracy_lz4hc.cpp",
    "public/libbacktrace/alloc.cpp",
    "public/libbacktrace/config.h",
    "public/libbacktrace/dwarf.cpp",
    "public/libbacktrace/elf.cpp",
    "public/libbacktrace/fileline.cpp",
    "public/libbacktrace/macho.cpp",
    "public/libbacktrace/mmapio.cpp",
    "public/libbacktrace/posix.cpp",
    "public/libbacktrace/sort.cpp",
    "public/libbacktrace/state.cpp",
]


_TRACY_CLIENT_HEADERS = [
    "public/client/tracy_concurrentqueue.h",
    "public/client/tracy_rpmalloc.hpp",
    "public/client/tracy_SPSCQueue.h",
    "public/client/TracyCallstack.h",
    "public/client/TracyCallstack.hpp",
    "public/client/TracyFastVector.hpp",
    "public/client/TracyKCore.hpp",
    "public/client/TracyLock.hpp",
    "public/client/TracyProfiler.hpp",
    "public/client/TracyScoped.hpp",
    "public/client/TracySysPower.hpp",
    "public/client/TracySysTime.hpp",
    "public/client/TracySysTrace.hpp",
    "public/common/TracyAlign.hpp",
    "public/common/TracyAlloc.hpp",
    "public/common/TracyApi.h",
    "public/common/TracyColor.hpp",
    "public/common/TracyForceInline.hpp",
    "public/common/TracyMutex.hpp",
    "public/common/TracyProtocol.hpp",
    "public/common/TracyQueue.hpp",
    "public/common/TracySystem.hpp",
    # Public header
    "public/tracy/Tracy.hpp",
    "public/tracy/TracyC.h",
]

_TRACY_DEFINES = [
    "TRACY_ENABLE",
    # Enable if you want to only record instrumented code.
    "TRACY_NO_SAMPLING"
]

cc_library(
    name = "_tracy",
    srcs = _TRACY_SOURCE_FILES,
    hdrs = _TRACY_HEADER_FILES,
    includes = ["public"],
    features = ["-gcc_werror_flag"],
    defines = _TRACY_DEFINES,
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "tracy_client",
    srcs = ["public/TracyClient.cpp"] + _TRACY_CLIENT_HEADERS,
    visibility = ["//visibility:public"],
    deps = [":_tracy"],
    linkopts = [
        "-ldl",
    ],
    linkshared = True,
    linkstatic = False,
    tags = ["manual"],
    features = ["-gcc_werror_flag"],
)

cc_library(
    name = "client_enabled",
    srcs = [":tracy_client"],
    hdrs = _TRACY_CLIENT_HEADERS,
    includes = ["public"],
    defines = _TRACY_DEFINES,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "client_disabled",
    hdrs = _TRACY_CLIENT_HEADERS,
    includes = ["public"],
    visibility = ["//visibility:public"],
)

# The ":client_enabled" project will be picked as a dependency if The
# `--@tracy//:enable_tracy=True` flag is specified; if the flag is
# set to False or omitted, ":client_disabled" will be picked instead.
cc_library(
    name = "tracy",
    # linkopts = ["-ldl"],
    visibility = ["//visibility:public"],
    deps = select({
        ":do_enable_tracy": [":client_enabled"],
        "//conditions:default": [":client_disabled"],
    }),
)

# TODO(rochan): cc_binary target for profiler client