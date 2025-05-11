cc_library(
    name = "glm",
    hdrs = glob(["glm/**/*.hpp", "glm/**/*.h"]),
    visibility = ["//visibility:public"],
    includes = ["."],
    strip_include_prefix = "",
    textual_hdrs = glob(["glm/**/*.inl"]),
    copts = select({
        "//conditions:default": [
            "-std=c++17",
            # Optional GLM feature toggles:
            # "-DGLM_FORCE_INTRINSICS",
            # "-DGLM_FORCE_CTOR_INIT",
            # "-DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES",
        ],
    }),
)
