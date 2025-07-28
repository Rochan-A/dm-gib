#pragma once

#include "util/macros.h"
#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "engine/shaders/compiler.h"
#include "engine/shaders/types.h"
#include "util/report/report.h"

#include <glm/glm.hpp>

namespace gib {

class Shader {
public:
  Shader() = default;
  explicit Shader(const ShaderSource &source);
  Shader(const ShaderSource &source1, const ShaderSource &source2);

  ~Shader() = default;

  [[nodiscard]] inline unsigned int GetProgramId() const {
    return shader_program_;
  }

  void Link();

  void Activate() const { glUseProgram(shader_program_); }
  static void Deactivate() { glUseProgram(0); }

  void UpdateUniforms();

  // Utility uniform functions
  void SetBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(shader_program_, name.c_str()),
                static_cast<int>(value));
  }

  void SetInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(shader_program_, name.c_str()), value);
  }

  void SetFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(shader_program_, name.c_str()), value);
  }

  void SetVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(shader_program_, name.c_str()), 1,
                 &value[0]);
  }

  void SetVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(shader_program_, name.c_str()), 1,
                 &value[0]);
  }

  void SetVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(shader_program_, name.c_str()), 1,
                 &value[0]);
  }

  void SetMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(shader_program_, name.c_str()), 1,
                       GL_FALSE, &mat[0][0]);
  }

  void SetMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(shader_program_, name.c_str()), 1,
                       GL_FALSE, &mat[0][0]);
  }

  void SetMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(shader_program_, name.c_str()), 1,
                       GL_FALSE, &mat[0][0]);
  }

  DISALLOW_COPY_AND_ASSIGN(Shader);

protected:
  ShaderCompiler shader_compiler_{};
  unsigned int shader_program_{0};
};

} // namespace gib