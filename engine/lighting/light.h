#pragma once

#include "engine/lighting/light_base.h"

#include <glm/glm.hpp>

namespace gib {

enum class LightType : char {
  DIRECTIONAL,
  POINT,
  AREA,
};

struct Attenuation {
  float constant;
  float linear;
  float quadratic;
};

constexpr Attenuation kDefaultAttenuation = {1.0f, 0.09f, 0.032f};
constexpr glm::vec3 kDefaultDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
constexpr glm::vec3 kDefaultSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

constexpr float kDefaultInnerAngle = glm::radians(10.5f);
constexpr float kDefaultOuterAngle = glm::radians(19.5f);

class DirectionalLight final : LightBase<DirectionalLight> {
public:
  DirectionalLight() = default;
  ~DirectionalLight() = default;
};

class PointLight final : LightBase<PointLight> {
public:
  PointLight() = default;
  ~PointLight() = default;
};

class AreaLight final : LightBase<AreaLight> {
public:
  AreaLight() = default;
  ~AreaLight() = default;
};

// Geometry for an area or point light source.
class SphereLightMesh final : LightMeshBase<SphereLightMesh> {
public:
  SphereLightMesh() = default;
  ~SphereLightMesh() = default;
};

} // namespace gib