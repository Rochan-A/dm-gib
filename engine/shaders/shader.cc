#include "engine/shaders/shader.h"
#include "engine/shaders/compiler.h"
#include "engine/shaders/types.h"
#include "util/report/report.h"

namespace gib {

Shader::Shader(const ShaderSource &source) {
  shader_compiler_.LoadAndCompile(source);
}

Shader::Shader(const ShaderSource &source1, const ShaderSource &source2) {
  shader_compiler_.LoadAndCompile(source1);
  shader_compiler_.LoadAndCompile(source2);
}

void Shader::Link() { shader_program_ = shader_compiler_.Link(); }

} // namespace gib