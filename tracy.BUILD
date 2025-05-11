cc_library(
    name = "tracy",
    srcs = glob(["public/client/*.cpp"]),
    hdrs = glob([
        "public/tracy/*.h",
        "public/tracy/*.hpp",
        "public/common/*.hpp",
    ]),
    textual_hdrs = glob(["public/**/*.hpp"]),
    includes = ["public"],
    defines = [
        "TRACY_ENABLE",                  # turn the profiler on
        "TRACY_ON_DEMAND",               # only start if a viewer connects
    ],
    copts = select({
        "//conditions:default": [
            "-std=c++17",
            "-fno-omit-frame-pointer",    # nicer call-stacks
            "-g",                         # keep debug info
        ],
        # "//bazel/config:no_tracy": [],
    }),
    linkopts = select({
        "//conditions:default": ["-pthread"],
        # "//bazel/config:no_tracy": [],
    }),
    visibility = ["//visibility:public"],
)
