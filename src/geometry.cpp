#include "geometry.hpp"

#include <algorithm>
#include <cmath>

#include <glm/gtc/constants.hpp>

namespace geometry {
namespace {

// Adds one triangle. `inside` is any point inside the (convex) shape; it is
// used to make the face normal point outwards regardless of vertex order.
void add_triangle(Mesh &mesh, const glm::vec3 &a, const glm::vec3 &b,
                  const glm::vec3 &c, const glm::vec3 &inside,
                  const glm::vec3 &color) {
  glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
  const glm::vec3 centroid = (a + b + c) / 3.0f;
  if (glm::dot(normal, centroid - inside) < 0.0f)
    normal = -normal;

  for (const glm::vec3 &vertex : {a, b, c}) {
    mesh.positions.emplace_back(vertex, 1.0f);
    mesh.colors.emplace_back(color, 1.0f);
    mesh.normals.push_back(normal);
    if (std::abs(normal.y) > 0.8f)
      mesh.uvs.emplace_back(vertex.x, vertex.z);
    else if (std::abs(normal.x) > 0.8f)
      mesh.uvs.emplace_back(vertex.z, vertex.y);
    else
      mesh.uvs.emplace_back(vertex.x, vertex.y);
  }
}

// Adds the quad a-b-c-d as two triangles.
void add_quad(Mesh &mesh, const glm::vec3 &a, const glm::vec3 &b,
              const glm::vec3 &c, const glm::vec3 &d, const glm::vec3 &inside,
              const glm::vec3 &color) {
  add_triangle(mesh, a, b, c, inside, color);
  add_triangle(mesh, a, c, d, inside, color);
}

} // namespace

void Mesh::append(const Mesh &other) {
  positions.insert(positions.end(), other.positions.begin(),
                   other.positions.end());
  colors.insert(colors.end(), other.colors.begin(), other.colors.end());
  normals.insert(normals.end(), other.normals.begin(), other.normals.end());
  uvs.insert(uvs.end(), other.uvs.begin(), other.uvs.end());
}

Mesh box(const glm::vec3 &center, float width_bottom, float width_top,
         float height, float depth, const glm::vec3 &color) {
  const float hb = width_bottom / 2.0f;
  const float ht = width_top / 2.0f;
  const float hh = height / 2.0f;
  const float hd = depth / 2.0f;

  const glm::vec3 corners[8] = {
      center + glm::vec3(-hb, -hh, hd),  center + glm::vec3(hb, -hh, hd),
      center + glm::vec3(ht, hh, hd),    center + glm::vec3(-ht, hh, hd),
      center + glm::vec3(-hb, -hh, -hd), center + glm::vec3(hb, -hh, -hd),
      center + glm::vec3(ht, hh, -hd),   center + glm::vec3(-ht, hh, -hd)};

  // front, back, left, right, top, bottom
  const int faces[6][4] = {{0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7},
                           {1, 5, 6, 2}, {3, 2, 6, 7}, {4, 5, 1, 0}};

  Mesh mesh;
  for (const auto &face : faces) {
    add_quad(mesh, corners[face[0]], corners[face[1]], corners[face[2]],
             corners[face[3]], center, color);
  }
  return mesh;
}

Mesh box(const glm::vec3 &center, const glm::vec3 &size,
         const glm::vec3 &color) {
  return box(center, size.x, size.x, size.y, size.z, color);
}

Mesh cylinder(const glm::vec3 &base_center, float radius, float height,
              const glm::vec3 &color, int segments) {
  const glm::vec3 top_center = base_center + glm::vec3(0.0f, height, 0.0f);
  const glm::vec3 inside = base_center + glm::vec3(0.0f, height / 2.0f, 0.0f);

  Mesh mesh;
  for (int i = 0; i < segments; ++i) {
    const float angle0 = glm::two_pi<float>() * i / segments;
    const float angle1 = glm::two_pi<float>() * (i + 1) / segments;
    const glm::vec3 rim0(radius * std::sin(angle0), 0.0f,
                         radius * std::cos(angle0));
    const glm::vec3 rim1(radius * std::sin(angle1), 0.0f,
                         radius * std::cos(angle1));

    add_quad(mesh, base_center + rim0, base_center + rim1, top_center + rim1,
             top_center + rim0, inside, color); // side
    add_triangle(mesh, top_center, top_center + rim0, top_center + rim1, inside,
                 color); // top
    add_triangle(mesh, base_center, base_center + rim1, base_center + rim0,
                 inside, color); // bottom
  }
  return mesh;
}

} // namespace geometry
