#pragma once

#include <string>
#include <vector>

#include "engine/core/types.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

namespace gib {

// Type of texture.
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
enum class MipFiltering : GLenum {
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

// Returns the number of mips for an image of a given width/height.
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
// at mip level-0.
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