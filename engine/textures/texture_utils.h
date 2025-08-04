#pragma once

#include <string>
#include <vector>

#include "engine/core/types.h"
#include "util/report/report.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/texture.h>

namespace gib {

// Type of texture. Currently supports 2D and CUBEMAP.
enum class TextureType : GLenum {
  TEXTURE_2D = GL_TEXTURE_2D,
  CUBE_MAP = GL_TEXTURE_CUBE_MAP
};

// Type of texture binding.
enum class TextureBindType : unsigned char {
  BY_TEXTURE_TYPE = 0,
  TEXTURE_2D,
  TEXTURE_3D,
  CUBEMAP,
  // Image texture that is directly indexed, rather than sampled.
  IMAGE_TEXTURE,
};

// Texture wrap mode.
enum class TextureWrapMode : GLenum {
  REPEAT = GL_REPEAT,
  MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
  CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
  CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
};

// Mipmap generation scheme.
enum class MipGeneration : unsigned char {
  // Never generates or reserves mipmaps.
  NEVER = 0,
  // Generates mipmaps when loading, but not when creating empty textures.
  ON_LOAD,
  // Always generate mipmaps when possible.
  ALWAYS,
};

// Texture filtering scheme.
enum class TextureFiltering : unsigned char {
  // Uses nearest-neighbor sampling.
  NEAREST = 0,
  // Uses linear interpolation between texels.
  BILINEAR,
  // Uses linear interpolation between mipmap levels and their texels.
  TRILINEAR,
  // Handles anisotropy when sampling.
  ANISOTROPIC,
};

// Mipmap filtering scheme
enum class MipFiltering {
  BASED_ON_TEXTURE_FILTERING = 0,
  // Takes the nearest mipmap to match the pixel size and uses nearest neighbor
  // interpolation for texture sampling.
  NEAREST_MINMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
  // Takes the nearest mipmap level and samples that level using linear
  // interpolation.
  LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
  // Linearly interpolates between the two mipmaps that most closely match the
  // size of a pixel and samples the interpolated level via nearest neighbor
  // interpolation.
  NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
  // Linearly interpolates between the two closest mipmaps and samples the
  // interpolated level via linear interpolation.
  LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
};

enum class TextureFormat : GLenum {
  R_8 = GL_R8,
  RG_8 = GL_RG8,
  SRGB_8 = GL_SRGB8,
  RGB_8 = GL_RGB8,
  SRGBA_8 = GL_SRGB8_ALPHA8,
  RGBA_8 = GL_RGBA8,
  // HDR formats
  HDR_R = GL_R16F,
  HDR_RG = GL_RG16F,
  HDR_RGB = GL_RGB16F,
  HDR_RGBA = GL_RGBA16F,
};

// The type of map, i.e. how the underlying texture is meant to be used.
enum class TextureMapType {
  DIFFUSE = 0,
  SPECULAR,
  ROUGHNESS,
  METALLIC,
  AO,
  EMISSION,
  NORMAL,
  CUBEMAP,
};

inline std::vector<aiTextureType>
TextureMapTypeToAssimpTextureTypes(const TextureMapType type) {
  switch (type) {
  case TextureMapType::DIFFUSE:
    return {aiTextureType_DIFFUSE};
  case TextureMapType::SPECULAR:
    // Use metalness for specularity as well. When this is loaded as a
    // combined "metalnessRoughnessTexture", shaders should read the blue
    // channel.
    return {aiTextureType_SPECULAR, aiTextureType_METALNESS};
  case TextureMapType::ROUGHNESS:
    return {aiTextureType_DIFFUSE_ROUGHNESS};
  case TextureMapType::METALLIC:
    return {aiTextureType_METALNESS};
  case TextureMapType::AO:
    // For whatever reason, assimp stores AO maps as "lightmaps", even though
    // there's AssimpTextureType_AMBIENT_OCCLUSION...
    return {aiTextureType_LIGHTMAP};
  case TextureMapType::EMISSION:
    return {aiTextureType_EMISSIVE};
  case TextureMapType::NORMAL:
    return {aiTextureType_NORMALS};
  default:
    THROW_FATAL("ERROR::TEXTURE_MAP::INVALID_TEXTURE_MAP_TYPE {}",
                static_cast<int>(type));
  }
}

inline std::string TextureMapTypeToString(const TextureMapType type) {
  switch (type) {
  case TextureMapType::DIFFUSE:
    return "u_Diffuse";
  case TextureMapType::SPECULAR:
    // Use metalness for specularity as well. When this is loaded as a
    // combined "metalnessRoughnessTexture", shaders should read the blue
    // channel.
    return "u_Specular";
  case TextureMapType::ROUGHNESS:
    return "u_Roughness";
  case TextureMapType::METALLIC:
    return "u_Metallic";
  case TextureMapType::AO:
    // For whatever reason, assimp stores AO maps as "lightmaps", even though
    // there's AssimpTextureType_AMBIENT_OCCLUSION...
    return "u_AmbientOcclusion";
  case TextureMapType::EMISSION:
    return "u_Emmision";
  case TextureMapType::NORMAL:
    return "u_Normal";
  default:
    THROW_FATAL("ERROR::TEXTURE_MAP::INVALID_TEXTURE_MAP_TYPE {}",
                static_cast<int>(type));
  }
}

// Returns the number of mips given width and height. Includes the original
// level.
inline int GetNumMips(const Size2D &size) {
  return 1 + static_cast<int>(
                 std::floor(std::log2(std::max(size.Width(), size.Height()))));
}

// Returns the next mip size given an initial size.
inline Size2D GetNextMipSize(const Size2D &mip_size) {
  return Size2D{std::max(mip_size.Width() / 2, 1),
                std::max(mip_size.Height() / 2, 1)};
}

// Returns the calculated size for a specified mip level given width and height
// at mip level-0 (i.e., original size).
inline Size2D GetMipLevel(const Size2D &base_size, const int level) {
  Size2D size{base_size.Width(), base_size.Height()};
  if (level == 0) {
    return size;
  }
  for (int mip = 0; mip < level; ++mip) {
    size = GetNextMipSize(size);
  }
  return size;
}

} // namespace gib