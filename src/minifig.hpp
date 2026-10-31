#pragma once

#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "geometry.hpp"
#include "node.hpp"

// Colours and extra details that make one minifigure look different from
// another. Both minifigures share exactly the same body hierarchy.
struct FigureStyle {
  std::string name;
  glm::vec3 skin;
  glm::vec3 torso;
  glm::vec3 arms;
  glm::vec3 hips;
  glm::vec3 legs;
  glm::vec3 feet;
  geometry::Mesh torso_details; // printed on the torso front (torso frame)
  geometry::Mesh hat;           // in the frame at the top of the head
};

FigureStyle police_officer_style();
FigureStyle chef_style();

// A LEGO minifigure: a hierarchy of Nodes plus the list of its degrees of
// freedom (dofs) and the keys that change them.
class Minifig {
public:
  explicit Minifig(const FigureStyle &style);

  void render(const glm::mat4 &view_projection, GLint mvp_location) const;

  // Changes the dof mapped to `key` by one step (Shift reverses the
  // direction for single-key dofs). Returns false if the key is unmapped.
  bool handle_key(int key, bool shift);

  // Puts every dof back to its default value.
  void reset();

  const std::string &name() const { return name_; }

  glm::mat4 hand_transform(bool left) const;
  glm::mat4 root_transform() const { return root_->world_transform(); }
  std::vector<float> pose() const;
  void set_pose(const std::vector<float> &values);
  void texture(GLuint id) { root_->set_texture(id); }

private:
  enum class DofType { Rotation, Translation };
  enum Axis { kX = 0, kY = 1, kZ = 2 };

  struct Dof {
    Node *node;
    DofType type;
    Axis axis;
    int key_increase;
    int key_decrease; // same as key_increase => Shift+key decreases
    float default_value;
    float min_value;
    float max_value;
    float step;
    bool wraps; // full-turn rotation without limits
    float value = 0.0f;
  };

  // The joints of one side of the body.
  struct Limbs {
    Node *shoulder;
    Node *wrist;
    Node *hip;
    Node *knee;
  };

  Limbs build_limbs(Node *torso, Node *pelvis, const FigureStyle &style,
                    float side);
  static void apply(const Dof &dof);

  Node *left_hand_ = nullptr;
  Node *right_hand_ = nullptr;
  std::string name_;
  std::unique_ptr<Node> root_;
  std::vector<Dof> dofs_;
};
