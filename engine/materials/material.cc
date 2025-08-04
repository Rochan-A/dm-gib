#include "engine/materials/material.h"
#include "engine/textures/texture_utils.h"

namespace gib {

static constexpr GLuint kUBOBindingPoint = 3; // keep in sync with GLSL

Material::Material(Shader *shader) : shader_(shader) {
  glGenBuffers(1, &ubo_);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialParams),
               nullptr, // allocate, fill later
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, kUBOBindingPoint, ubo_);
}

void Material::UploadUBO() {
  if (!dirty_) {
    return;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
  // TODO(rochan): optimize this.
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialParams), &params_);
  dirty_ = false;
}

void Material::Bind(TextureRegistry &reg) {
  shader_->Activate();
  UploadUBO();

  for (auto &kv : textures_) {
    unsigned int unit = reg.GetNextTextureUnit();
    kv.second->BindToUnit(unit);

    // Uniform name convention:  "u_<MapType>"  (e.g. u_Albedo, u_Normal)
    shader_->SetInt(TextureMapTypeToChar(kv.first), unit);
  }
}

} // namespace gib