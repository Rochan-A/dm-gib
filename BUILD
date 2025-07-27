load("@gazelle//:def.bzl", "gazelle", "gazelle_binary")
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

# Define a gazelle binary with a list of enabled extensions
gazelle_binary(
    name = "gazelle_cc",
    languages = [
        "@gazelle//language/proto",  # Optional, should be defined before cc
        "@gazelle_cc//language/cc",
    ],
)

# `gazelle` rule can be used to provide additional arguments, eg. for CI integration
gazelle(
    name = "gazelle",
    gazelle = ":gazelle_cc",
)

refresh_compile_commands(
    name = "refresh_compile_commands",
    exclude_external_sources = False,
    exclude_headers = "external",
    targets = {
        "@//...": "",
        "//third_party/...": "",
        "//engine/...": "",
        "//util/...": "",
        "//gib/...": "",
    },
)
