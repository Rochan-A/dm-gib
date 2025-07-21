#pragma once

#include "engine/lighting/light_base.h"

namespace gib {

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