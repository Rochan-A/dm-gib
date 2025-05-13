package(default_visibility = ["//visibility:public"])

# ---------------------------------------------------------------------
# 1.  Tiny helper: generate   include/assimp/config.h
#     We replace every   '#cmakedefine FOO'   line with   '// #undef FOO'
# ---------------------------------------------------------------------
genrule(
    name = "config_h",
    srcs = ["include/assimp/config.h.in"],
    outs = ["include/assimp/config.h"],
    cmd = """
      # Simple one-liner: comment-out any '#cmakedefine …' directive
      sed -e 's/^#cmakedefine/\\/\\/ #undef/' $(SRCS) > $@
    """,
)

cc_library(
    name = "assimp",
    srcs = glob([
        "code/**/*.c", "code/**/*.cpp",
    ], exclude = [
        "*/contrib/**",
        "*/test/**",
        "*/unit/**",
        "*/tools/**",
        "*/samples/**",
        "*/fuzz/**",
    ]),
    hdrs = glob([
        "include/assimp/**/*.h",
        "include/assimp/**/*.hpp",
        "include/assimp/**/*.inl",
    ]) + [":config_h"],
    textual_hdrs = glob([
        "code/**/*.h",
        "code/**/*.hpp",
    ]),
    includes = [
        "code"
    ],
    strip_include_prefix = "include",
    copts = [
        "-fno-rtti",
    ],
    defines = [
        "ASSIMP_BUILD_STATIC_LIB",
        "ASSIMP_BUILD_NO_EXPORT",
        "ASSIMP_NO_OWN_ZLIB",
    ],
    deps = ["@zlib//:zlib"],
)
