#pragma once

namespace gib {
template <typename LightModel> class LightBase {
public:
  LightBase() = default;
  ~LightBase() = default;
};

template <typename LightMesh> class LightMeshBase {
public:
  LightMeshBase() = default;
  ~LightMeshBase() = default;
};

} // namespace gib