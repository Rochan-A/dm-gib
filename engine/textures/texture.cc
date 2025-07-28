#include "engine/textures/texture.h"
#include "engine/textures/texture_utils.h"
#include "util/report/report.h"

namespace gib {

Texture Texture::Load2D(const std::string &path, const TextureParams &params,
                        const bool is_srgb) {
  Texture texture;
  texture.type_ = TextureType::TEXTURE_2D;

  stbi_set_flip_vertically_on_load(
      static_cast<int>(params.flip_vertical_on_load));
  unsigned char *data = stbi_load(path.c_str(), &texture.size_.x,
                                  &texture.size_.y, &texture.num_channels_,
                                  /*desired_channels=*/0);
  ASSERT(data != nullptr, "Failed to load texture from {}", path);

  GLenum data_format = GL_INVALID_ENUM;
  switch (texture.num_channels_) {
  case 1:
    texture.internal_format_ = GL_R8;
    data_format = GL_RED;
    break;
  case 2:
    texture.internal_format_ = GL_RG8;
    data_format = GL_RG;
    break;
  case 3:
    texture.internal_format_ = is_srgb ? GL_SRGB8 : GL_RGB8;
    data_format = GL_RGB;
    break;
  case 4:
    texture.internal_format_ = is_srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    data_format = GL_RGBA;
    break;
  default:
    THROW_FATAL("Attempting to load un-supported texture type. {} contains "
                "unsupported number of channels: {}",
                path, texture.num_channels_);
  }
  glGenTextures(1, &texture.texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture.texture_id_);

  // TODO(rochan): Replace with glTexStorage2D (OpenGL 4.4)
  glTexImage2D(GL_TEXTURE_2D, /* mipmap level */ 0, texture.internal_format_,
               texture.size_.x, texture.size_.y, 0,
               /* tex data format */ data_format, GL_UNSIGNED_BYTE, data);

  if (params.mip_generation == MipGeneration::NEVER) {
    texture.num_mips_ = 1;
  } else {
    texture.num_mips_ = std::min(params.max_num_mip, GetNumMips(texture.size_));
    texture.GenerateMips(texture.num_mips_);
  }
  ApplyTextureParams(params, texture.type_);
  stbi_image_free(data);
  return texture;
}

Texture Texture::Load2DHDR(const std::string &path,
                           const TextureParams &params) {
  Texture texture;
  texture.type_ = TextureType::TEXTURE_2D;

  stbi_set_flip_vertically_on_load(params.flip_vertical_on_load);
  float *data = stbi_loadf(path.c_str(), &texture.size_.x, &texture.size_.y,
                           &texture.num_channels_, /*desired_channels=*/0);
  ASSERT(data != nullptr, "Failed to load texture from {}", path);

  GLenum data_format = GL_INVALID_ENUM;
  switch (texture.num_channels_) {
  case 1:
    texture.internal_format_ = GL_R16F;
    data_format = GL_RED;
    break;
  case 2:
    texture.internal_format_ = GL_RG16F;
    data_format = GL_RG;
    break;
  case 3:
    texture.internal_format_ = GL_RGB16F;
    data_format = GL_RGB;
    break;
  case 4:
    texture.internal_format_ = GL_RGBA16F;
    data_format = GL_RGBA;
    break;
  default:
    THROW_FATAL("Attempting to load un-supported texture type. {} contains "
                "unsupported number of channels: {}",
                path, texture.num_channels_);
  }
  glGenTextures(1, &texture.texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture.texture_id_);

  // TODO(rochan): Replace with glTexStorage2D
  glTexImage2D(GL_TEXTURE_2D, /*mip=*/0, texture.internal_format_,
               texture.size_.x, texture.size_.y, 0,
               /*tex data format=*/data_format, GL_FLOAT, data);

  if (params.mip_generation == MipGeneration::NEVER) {
    texture.num_mips_ = 1;
  } else {
    texture.num_mips_ = std::min(params.max_num_mip, GetNumMips(texture.size_));
    texture.GenerateMips(texture.num_mips_);
  }
  ApplyTextureParams(params, texture.type_);
  stbi_image_free(data);
  return texture;
}

Texture Texture::LoadCubemap(const std::vector<std::string> &paths,
                             const TextureParams &params) {
  ASSERT(paths.size() == 6,
         "Must pass exactly 6 faces to Texture::LoadCubemap");

  Texture texture;
  texture.type_ = TextureType::CUBE_MAP;
  // TODO(rochan): support mip-map for cubemaps?
  texture.num_mips_ = 1;
  // Cubemaps must be RGB.
  texture.internal_format_ = GL_RGB8;

  glGenTextures(1, &texture.texture_id_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture.texture_id_);

  bool initialized = false;
  for (int face_idx = 0; face_idx < paths.size(); ++face_idx) {
    Size2D face_size{0, 0};
    int num_channels{0};

    unsigned char *data = stbi_load(paths.at(face_idx).c_str(), &face_size.x,
                                    &face_size.y, &num_channels,
                                    /*desired_channels=*/0);
    ASSERT(data != nullptr, "Failed to load cubemap texture from path: {}",
           paths.at(face_idx));
    ASSERT(num_channels == 3,
           "Cubemap texture contained unsupported number of channels: {}",
           num_channels);
    ASSERT(face_size.Width() > 0 && face_size.Width() == face_size.Height(),
           "Cubemap texture must be square, got {}.", to_string(face_size));
    if (!initialized) {
      texture.size_ = face_size;
      texture.num_channels_ = num_channels;
      initialized = true;
    }
    ASSERT(face_size == texture.size_,
           "Cubemap texture {} does not match expected size {}, got {}",
           paths.at(face_idx), to_string(texture.size_), to_string(face_size));
    ASSERT(num_channels == texture.num_channels_,
           "Cubemap texture must be square, got {}.", to_string(face_size));

    // Load into the next cube map texture position.
    // TODO(rochan): Replace with glTexStorage2D
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_idx, /*level=*/0,
                 texture.internal_format_, face_size.Width(),
                 face_size.Height(), /*border=*/0,
                 /*format=*/GL_RGB, /*type=*/GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }
  ApplyTextureParams(params, texture.type_);
  return texture;
}

Texture Texture::Create2DTexture(const Size2D &size, TextureFormat format,
                                 const TextureParams &params) {
  Texture texture;
  texture.type_ = TextureType::TEXTURE_2D;
  texture.size_ = size;
  texture.num_mips_ = 1;

  if (params.mip_generation == MipGeneration::ALWAYS) {
    texture.num_mips_ = std::min(GetNumMips(size), params.max_num_mip);
  }
  texture.internal_format_ = static_cast<GLenum>(format);

  glGenTextures(1, &texture.texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture.texture_id_);

  // TODO(rochan): Replace with glTexStorage2D.
  // Not sure if the format passed here is correct.
  // Ref: https://docs.gl/gl4/glTexImage2D
  glTexImage2D(GL_TEXTURE_2D, /*mip=*/0, texture.internal_format_,
               texture.size_.Width(), texture.size_.Height(), 0,
               /*format=*/texture.internal_format_, GL_FLOAT, nullptr);
  ApplyTextureParams(params, texture.type_);
  return texture;
}

Texture Texture::CreateCubemap(const int size, TextureFormat format,
                               const TextureParams &params) {
  Texture texture;
  texture.type_ = TextureType::CUBE_MAP;
  texture.size_ = Size2D(size, size);
  texture.num_mips_ = 1;

  if (params.mip_generation == MipGeneration::ALWAYS) {
    texture.num_mips_ = std::min(GetNumMips(texture.size_), params.max_num_mip);
  }
  texture.internal_format_ = static_cast<GLenum>(format);

  glGenTextures(1, &texture.texture_id_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture.texture_id_);

  // TODO(rochan): Replace with glTexStorage2D.
  // Not sure if the format passed here is correct.
  // Ref: https://docs.gl/gl4/glTexImage2D
  glTexImage2D(GL_TEXTURE_CUBE_MAP, /*mip=*/0, texture.internal_format_,
               texture.size_.Width(), texture.size_.Height(), 0,
               /*format=*/texture.internal_format_, GL_FLOAT, nullptr);
  ApplyTextureParams(params, texture.type_);
  return texture;
}

Texture Texture::CreateFromData(const Size2D &size, TextureFormat format,
                                const std::vector<glm::vec3> &data,
                                const TextureParams &params) {
  ASSERT(data.size() == static_cast<size_t>(size.Height() * size.Width()),
         "Invalid data size!");

  Texture texture = Texture::Create2DTexture(size, format, params);
  glTexSubImage2D(GL_TEXTURE_2D, /*level=*/0, /*xoffset=*/0,
                  /*yoffset=*/0, texture.size_.Width(), texture.size_.Height(),
                  /*format=*/GL_RGB, GL_FLOAT, data.data());
  return texture;
}

void Texture::BindToUnit(unsigned int texture_unit, TextureBindType bind_type) {
  // TODO(rochan): Take into account GL_MAX_TEXTURE_UNITS here.
  glActiveTexture(GL_TEXTURE0 + texture_unit);

  if (bind_type == TextureBindType::BY_TEXTURE_TYPE) {
    switch (type_) {
    case TextureType::TEXTURE_2D:
      bind_type = TextureBindType::TEXTURE_2D;
      break;
    case TextureType::CUBE_MAP:
      bind_type = TextureBindType::CUBEMAP;
      break;
    default:
      THROW_FATAL("TextureBindType was BY_TEXTURE_TYPE but TextureType is "
                  "un-supported {}",
                  type_);
    }
  }

  switch (bind_type) {
  case TextureBindType::TEXTURE_2D:
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    break;
  case TextureBindType::CUBEMAP:
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
    break;
  case TextureBindType::IMAGE_TEXTURE:
    // TODO(rochan): requires OpenGL 4.4 or newer
    // (https://docs.gl/gl4/glBindImageTextures).
    // Bind image unit.
    // glBindImageTexture(texture_unit, texture_id_, /*level=*/0,
    //                    /*layered=*/GL_FALSE, 0, GL_READ_WRITE,
    //                    internal_format_);
    // break;
  default:
    THROW_FATAL("Invalid TextureBindType {}", bind_type);
  }
}

void Texture::SetMipRange(const int min, const int max) {
  const auto target = static_cast<GLenum>(type_);
  glBindTexture(target, texture_id_);
  glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, min);
  glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, max);
}

void Texture::UnsetMipRange() {
  // OpenGL defaults.
  SetMipRange(kMinMip, kMaxMip);
}

Texture::~Texture() {
  if (texture_id_ > 0) {
    glDeleteTextures(1, &texture_id_);
  }
}

void Texture::GenerateMips(const int max_num_mip) {
  if (max_num_mip >= 0) {
    SetMipRange(0, max_num_mip);
  }

  const auto target = static_cast<GLenum>(type_);
  glBindTexture(target, texture_id_);
  glGenerateMipmap(target);

  if (max_num_mip >= 0) {
    UnsetMipRange();
  }
}

void Texture::ApplyTextureParams(const TextureParams &params,
                                 const TextureType type) {
  const auto target = static_cast<GLenum>(type);

  switch (params.filtering) {
  case TextureFiltering::NEAREST:
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    break;
  case TextureFiltering::BILINEAR:
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    break;
  case TextureFiltering::TRILINEAR:
    [[fallthrough]];
  case TextureFiltering::ANISOTROPIC:
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (params.filtering == TextureFiltering::ANISOTROPIC) {
      glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, kMaxAnisotropySamples);
    }
    break;
  }

  switch (params.warp_mode) {
  case TextureWrapMode::REPEAT:
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (type == TextureType::CUBE_MAP) {
      glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);
    }
    break;
  case TextureWrapMode::MIRRORED_REPEAT:
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    if (type == TextureType::CUBE_MAP) {
      glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    }
    break;
  case TextureWrapMode::CLAMP_TO_EDGE:
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (type == TextureType::CUBE_MAP) {
      glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    break;
  case TextureWrapMode::CLAMP_TO_BORDER:
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    if (type == TextureType::CUBE_MAP) {
      glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    }
    glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR,
                     glm::value_ptr(params.border_color));
    break;
  }
}

} // namespace gib