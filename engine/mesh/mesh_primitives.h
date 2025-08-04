#pragma once

#include "engine/mesh/mesh.h"

namespace gib {

// Plane
class PlaneMesh : public Mesh {
public:
  PlaneMesh();

protected:
};

// Unit cube
class CubeMesh : public Mesh {
public:
  CubeMesh();

protected:
};

// Like CubeMesh, but with normals pointing inward.
class RoomMesh : public Mesh {
public:
  RoomMesh();

protected:
};

// Unit sphere, with the given number of meridians & parallels.
class SphereMesh : public Mesh {
public:
  SphereMesh();

protected:
};

class SkyboxMesh : public Mesh {
public:
  // Creates an unbound skybox mesh.
  SkyboxMesh();
  // Creates a new skybox mesh from a set of 6 textures for the faces. Textures
  // must be passed in order starting with GL_TEXTURE_CUBE_MAP_POSITIVE_X and
  // incrementing from there; namely, in the order right, left, top, bottom,
  // front, and back.
  explicit SkyboxMesh(std::vector<std::string> faces);

protected:
};

class ScreenQuadMesh : public Mesh {
public:
  // Creates an unbound screen quad mesh.
  ScreenQuadMesh();
  // Creates a new screen quad mesh from a texture.
  explicit ScreenQuadMesh(Texture texture);

protected:
};

} // namespace gib