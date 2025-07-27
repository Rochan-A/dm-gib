load("@rules_cc//cc:defs.bzl", "cc_library")

licenses(["notice"])

cc_library(
    name = "imgui",
    srcs = [
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_impl_glfw.cpp",
        "imgui_impl_opengl3.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
    ],
    hdrs = [
        "imconfig.h",
        "imgui.h",
        "imgui_impl_glfw.h",
        "imgui_impl_opengl3.h",
        "imgui_impl_opengl3_loader.h",
        "imgui_internal.h",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
    ],
    includes = ["."],  # keep
    visibility = ["//visibility:public"],
    deps = [
        "@glfw",  # keep
        "@glm",
        "@stb//:stb_sprintf",
        "@stb//:stb_truetype",
    ],
)
