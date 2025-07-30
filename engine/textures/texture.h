#pragma once

#include "engine/textures/texture_utils.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "engine/core/types.h"
#include "engine/shaders/shader.h"
#include <string>
#include <vector>

#include <stb/stb_image.h>

#include "util/report/report.h"
#include <glm/glm.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace gib {

static constexpr int kMaxMip = 1000;
static constexpr int kMinMip = 1;
static constexpr float kMaxAnisotropySamples = 4.0f;

// Texture parameter struct.
struct TextureParams {
  // NOTE: OpenGL texture coordinates start at the bottom-right of the image, so
  // we flip vertically by default.
  bool flip_vertical_on_load{true};
  // Texture filtering.
  TextureFiltering filtering{TextureFiltering::ANISOTROPIC};
  // Texture wrap mode.
  TextureWrapMode wrap_mode{TextureWrapMode::REPEAT};
  glm::vec4 border_color{0.0f, 0.0f, 0.0f, 1.0f};
  MipGeneration mip_generation{MipGeneration::ON_LOAD};
  MipFiltering mip_filtering{MipFiltering::BASED_ON_TEXTURE_FILTERING};
  // Maximum number of mips to allocate. If negative, kMaxMip is used.
  int max_num_mip{-1};
};

static constexpr TextureParams kDefault2DTextureParam;

class Texture {
public:
  ~Texture();

  // Loads a 2D texture from a given path.
  static Texture Load2D(const std::string &path, const TextureParams &params,
                        bool is_srgb = false);

  // Loads a 2D HDR texture from the given path.
  // RGBE or Radiance HDR (.hdr) file.
  static Texture Load2DHDR(const std::string &path,
                           const TextureParams &params);

  // Loads a cubemap from a set of 6 textures for the faces. Textures must be
  // passed in the order: right, left, top, bottom, front, and back (i.e., xp,
  // xn, yp, yn, zp, zn)
  static Texture LoadCubemap(const std::vector<std::string> &paths,
                             const TextureParams &params);

  // Creates a custom 2D texture of the given size and format.
  static Texture Create2DTexture(const Size2D &size, TextureFormat format,
                                 const TextureParams &params);

  // Creates a custom cubemap texture of the given size and format.
  static Texture CreateCubemap(int size, TextureFormat format,
                               const TextureParams &params);

  // Creates a custom texture based on the given input data.
  static Texture CreateFromData(const Size2D &size, TextureFormat format,
                                const std::vector<glm::vec3> &data,
                                const TextureParams &params);

  // Binds the texture to the given texture unit.
  // Unit should be a number starting from 0, not the actual texture unit's
  // GLenum.
  void BindToUnit(unsigned int texture_unit,
                  TextureBindType bind_type = TextureBindType::BY_TEXTURE_TYPE);

  // Generates mipmaps for the current texture. Note that this will not succeed
  // for textures with immutable storage.
  void GenerateMips(int max_num_mip = -1);

  // Sets a min/max mip level allowed when sampling from this texture. This is
  // important to avoid undefined behavior when drawing to a mip level while
  // sampling from another.
  void SetMipRange(int min, int max);

  // Resets the allowed mip range to default values.
  void UnsetMipRange();

  [[nodiscard]] unsigned int GetTextureId() const { return texture_id_; }
  [[nodiscard]] TextureType GetTextureType() const { return type_; }
  // Returns the path to a texture. Not applicable for cubemaps or generated
  // textures.
  [[nodiscard]] std::string GetPath() const {
    return !path_.empty() ? path_ : "NA (likely cubemap or generated texture)";
  }
  [[nodiscard]] Size2D GetSize2D() const { return size_; }
  [[nodiscard]] int GetNumChannels() const { return num_channels_; }
  [[nodiscard]] int NumMips() const { return num_mips_; }
  // TODO(rochan): Remove GLenum from this API.
  [[nodiscard]] GLenum GetInternalFormat() const { return internal_format_; }

private:
  // TODO(rochan): Manage texture lifetimes to support unloading textures.
  unsigned int texture_id_{0};
  TextureType type_{TextureType::TEXTURE_2D};
  std::string path_;
  Size2D size_{0, 0};
  int num_channels_{0};
  int num_mips_{0};
  GLenum internal_format_{GL_INVALID_ENUM};

  // Applies the given params to the currently-active texture.
  static void ApplyTextureParams(const TextureParams &params, TextureType type);
};

class TextureSource {
public:
  // Binds one or more textures to the next available texture unit, and
  // assigns shader uniforms. It must return the next texture unit that can be
  // used.
  // TODO(rochan): Maybe instead allow the source to call getNextTextureUnit().
  virtual unsigned int BindTexture(unsigned int next_texture_unit,
                                   Shader &shader) = 0;
};

// Thin wrapper around a texture with packed and texture map type properties.
class TextureMap {
public:
  TextureMap(const Texture &texture, const TextureMapType type,
             bool is_packed = false)
      : texture_(texture), type_(type), is_packed_(is_packed) {}

  Texture &GetTexture() { return texture_; }
  [[nodiscard]] TextureMapType GetType() const { return type_; }
  [[nodiscard]] bool IsPacked() const { return is_packed_; }
  void SetPacked(const bool packed) { is_packed_ = packed; }

private:
  Texture texture_;
  const TextureMapType type_;
  // Whether the texture type is part of a packed texture.
  bool is_packed_;
};

// A manager of "texture-like" objects, in relation to how they are rendered.
// Rendering code should set up any textures that won't change between draw
// calls (such as shadow maps) as part of a TextureSource added to this
// registry. Then for each draw call, code should push a usage block, call
// GetNextTextureUnit repeatedly to set up texture, and then pop once done.
class TextureRegistry {
public:
  virtual ~TextureRegistry() = default;

  void AddTextureSource(std::shared_ptr<TextureSource> source) {
    texture_sources_.push_back(source);
  }

  // Returns the next available texture unit, and increments the number.
  unsigned int GetNextTextureUnit() { return next_texture_unit_++; }
  void PushUsageBlock();
  void PopUsageBlock();

private:
  unsigned int next_texture_unit_ = 0;
  std::vector<unsigned int> last_available_units_;
  std::vector<std::shared_ptr<TextureSource>> texture_sources_;
};

} // namespace gib