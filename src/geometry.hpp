#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace geometry {

// A list of triangles with one colour per vertex, ready to upload to a VBO.
struct Mesh {
  std::vector<glm::vec4> positions;
  std::vector<glm::vec4> colors;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> uvs;

  void append(const Mesh &other);
};

// Box whose bottom face is `width_bottom` wide and top face is `width_top`
// wide (equal widths give a cuboid). `center` is the centre of the box.
Mesh box(const glm::vec3 &center, float width_bottom, float width_top,
         float height, float depth, const glm::vec3 &color);

// Plain cuboid of the given size (width, height, depth).
Mesh box(const glm::vec3 &center, const glm::vec3 &size,
         const glm::vec3 &color);

// Upright cylinder standing on `base_center` and extending `height` along +Y.
Mesh cylinder(const glm::vec3 &base_center, float radius, float height,
              const glm::vec3 &color, int segments = 32);

} // namespace geometry
