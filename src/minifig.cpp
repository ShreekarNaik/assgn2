#include "minifig.hpp"

#include <algorithm>

#include <GLFW/glfw3.h>

using geometry::box;
using geometry::cylinder;
using geometry::Mesh;
using glm::vec3;

namespace {

const vec3 kLegoYellow(1.0f, 0.8f, 0.1f);
const vec3 kBlack(0.07f, 0.07f, 0.07f);
const vec3 kGold(0.95f, 0.7f, 0.15f);
const vec3 kWhite(0.95f, 0.95f, 0.95f);

// Two eyes and a mouth on the front of the head (head frame).
Mesh face_mesh() {
  Mesh face = box(vec3(-0.17f, 0.5f, 0.5f), vec3(0.1f, 0.14f, 0.06f), kBlack);
  face.append(box(vec3(0.17f, 0.5f, 0.5f), vec3(0.1f, 0.14f, 0.06f), kBlack));
  face.append(box(vec3(0.0f, 0.27f, 0.49f), vec3(0.3f, 0.05f, 0.06f), kBlack));
  return face;
}

} // namespace

FigureStyle police_officer_style() {
  FigureStyle style;
  style.name = "Police Officer";
  style.skin = kLegoYellow;
  style.torso = vec3(0.1f, 0.15f, 0.45f);
  style.arms = style.torso;
  style.hips = kBlack;
  style.legs = vec3(0.13f, 0.13f, 0.16f);
  style.feet = kBlack;

  // Tie and badge.
  style.torso_details = box(vec3(0.0f, 1.1f, 0.43f), 0.22f, 0.12f, 0.9f, 0.04f,
                            vec3(0.85f, 0.85f, 0.9f));
  style.torso_details.append(
      box(vec3(0.38f, 1.2f, 0.43f), vec3(0.18f, 0.2f, 0.04f), kGold));

  // Police cap: crown, peak and badge.
  style.hat = cylinder(vec3(0.0f, -0.05f, 0.0f), 0.53f, 0.35f, style.torso);
  style.hat.append(
      box(vec3(0.0f, 0.0f, 0.5f), vec3(0.8f, 0.06f, 0.35f), kBlack));
  style.hat.append(
      box(vec3(0.0f, 0.15f, 0.54f), vec3(0.16f, 0.16f, 0.04f), kGold));
  return style;
}

FigureStyle chef_style() {
  FigureStyle style;
  style.name = "Chef";
  style.skin = kLegoYellow;
  style.torso = kWhite;
  style.arms = kWhite;
  style.hips = vec3(0.25f, 0.25f, 0.28f);
  style.legs = vec3(0.25f, 0.25f, 0.28f);
  style.feet = kBlack;

  // Red neckerchief and two rows of buttons.
  style.torso_details = box(vec3(0.0f, 1.5f, 0.43f), 0.3f, 0.7f, 0.2f, 0.04f,
                            vec3(0.8f, 0.1f, 0.1f));
  for (float y : {1.15f, 0.8f, 0.45f}) {
    for (float x : {-0.2f, 0.2f}) {
      style.torso_details.append(
          box(vec3(x, y, 0.43f), vec3(0.1f, 0.1f, 0.04f), kBlack));
    }
  }

  // Chef hat: band plus a wider puffed top.
  style.hat = cylinder(vec3(0.0f, -0.05f, 0.0f), 0.53f, 0.3f, kWhite);
  style.hat.append(cylinder(vec3(0.0f, 0.25f, 0.0f), 0.68f, 0.5f, kWhite));
  return style;
}

Minifig::Minifig(const FigureStyle &style) : name_(style.name) {
  // Root: the torso. Its origin is at the waist and the figure faces +Z.
  Mesh torso_mesh =
      box(vec3(0.0f, 0.8f, 0.0f), 1.6f, 1.2f, 1.6f, 0.8f, style.torso);
  torso_mesh.append(style.torso_details);
  root_ = std::make_unique<Node>(torso_mesh, vec3(0.0f));
  Node *torso = root_.get();

  Node *pelvis = torso->add_child(
      box(vec3(0.0f, -0.15f, 0.0f), vec3(1.6f, 0.3f, 0.8f), style.hips),
      vec3(0.0f));

  // Neck -> head -> hat.
  Node *neck = torso->add_child(cylinder(vec3(0.0f), 0.28f, 0.15f, style.skin),
                                vec3(0.0f, 1.6f, 0.0f));
  Mesh head_mesh = cylinder(vec3(0.0f), 0.5f, 0.8f, style.skin);
  head_mesh.append(face_mesh());
  Node *head = neck->add_child(head_mesh, vec3(0.0f, 0.15f, 0.0f));
  head->add_child(style.hat, vec3(0.0f, 0.8f, 0.0f));

  // The figure's own left side is +X (it faces the viewer).
  const Limbs left = build_limbs(torso, pelvis, style, +1.0f);
  const Limbs right = build_limbs(torso, pelvis, style, -1.0f);

  left_hand_ = left.wrist;
  right_hand_ = right.wrist;

  using DT = DofType;
  // clang-format off
  dofs_ = {
    // node,          type,           axis, key+,             key-,          default, min,  max,  step, wraps
    {torso,           DT::Rotation,    kX, GLFW_KEY_X,     GLFW_KEY_X,     0.0f, -180.0f, 180.0f, 5.0f, true},
    {torso,           DT::Rotation,    kY, GLFW_KEY_Y,     GLFW_KEY_Y,    25.0f, -180.0f, 180.0f, 5.0f, true},
    {torso,           DT::Rotation,    kZ, GLFW_KEY_Z,     GLFW_KEY_Z,     0.0f, -180.0f, 180.0f, 5.0f, true},
    {torso,           DT::Translation, kX, GLFW_KEY_RIGHT, GLFW_KEY_LEFT,  0.0f,   -5.0f,   5.0f, 0.1f, false},
    {torso,           DT::Translation, kY, GLFW_KEY_UP,    GLFW_KEY_DOWN,  0.0f,   -4.0f,   4.0f, 0.1f, false},
    {torso,           DT::Translation, kZ, GLFW_KEY_EQUAL, GLFW_KEY_MINUS, 0.0f,   -8.0f,   5.0f, 0.1f, false},
    {neck,            DT::Rotation,    kY, GLFW_KEY_1,     GLFW_KEY_1,     0.0f,  -90.0f,  90.0f, 5.0f, false},
    {left.shoulder,   DT::Rotation,    kX, GLFW_KEY_2,     GLFW_KEY_2,     0.0f, -180.0f, 180.0f, 5.0f, false},
    {right.shoulder,  DT::Rotation,    kX, GLFW_KEY_3,     GLFW_KEY_3,     0.0f, -180.0f, 180.0f, 5.0f, false},
    {left.hip,        DT::Rotation,    kX, GLFW_KEY_4,     GLFW_KEY_4,     0.0f,  -90.0f,  90.0f, 5.0f, false},
    {right.hip,       DT::Rotation,    kX, GLFW_KEY_5,     GLFW_KEY_5,     0.0f,  -90.0f,  90.0f, 5.0f, false},
    {left.knee,       DT::Rotation,    kX, GLFW_KEY_6,     GLFW_KEY_6,     0.0f,    0.0f, 120.0f, 5.0f, false},
    {right.knee,      DT::Rotation,    kX, GLFW_KEY_7,     GLFW_KEY_7,     0.0f,    0.0f, 120.0f, 5.0f, false},
    {left.wrist,      DT::Rotation,    kY, GLFW_KEY_8,     GLFW_KEY_8,     0.0f, -180.0f, 180.0f, 5.0f, false},
    {right.wrist,     DT::Rotation,    kY, GLFW_KEY_9,     GLFW_KEY_9,     0.0f, -180.0f, 180.0f, 5.0f, false},
  };
  // clang-format on

  reset();
}

Minifig::Limbs Minifig::build_limbs(Node *torso, Node *pelvis,
                                    const FigureStyle &style, float side) {
  Limbs limbs;

  // Arm hangs down from the shoulder joint; the hand hangs from the wrist.
  limbs.shoulder = torso->add_child(
      box(vec3(0.0f, -0.45f, 0.0f), vec3(0.35f, 1.2f, 0.45f), style.arms),
      vec3(side * 0.8f, 1.4f, 0.0f));
  limbs.wrist = limbs.shoulder->add_child(
      cylinder(vec3(0.0f, -0.3f, 0.0f), 0.17f, 0.3f, style.skin),
      vec3(0.0f, -1.05f, 0.0f));

  // Thigh from the hip joint, lower leg from the knee, foot fixed below it.
  limbs.hip = pelvis->add_child(
      box(vec3(0.0f, -0.35f, 0.0f), vec3(0.76f, 0.7f, 0.8f), style.legs),
      vec3(side * 0.4f, -0.3f, 0.0f));
  limbs.knee = limbs.hip->add_child(
      box(vec3(0.0f, -0.3f, 0.0f), vec3(0.76f, 0.6f, 0.8f), style.legs),
      vec3(0.0f, -0.7f, 0.0f));
  limbs.knee->add_child(
      box(vec3(0.0f, -0.1f, 0.1f), vec3(0.76f, 0.2f, 1.0f), style.feet),
      vec3(0.0f, -0.6f, 0.0f));

  return limbs;
}

void Minifig::render(const glm::mat4 &view_projection,
                     GLint mvp_location) const {
  root_->render(view_projection, mvp_location);
}

bool Minifig::handle_key(int key, bool shift) {
  for (Dof &dof : dofs_) {
    float direction = 0.0f;
    if (dof.key_increase == dof.key_decrease) {
      if (key == dof.key_increase)
        direction = shift ? -1.0f : 1.0f;
    } else if (key == dof.key_increase) {
      direction = 1.0f;
    } else if (key == dof.key_decrease) {
      direction = -1.0f;
    }
    if (direction == 0.0f)
      continue;

    dof.value += direction * dof.step;
    if (dof.wraps) {
      if (dof.value > 180.0f)
        dof.value -= 360.0f;
      if (dof.value < -180.0f)
        dof.value += 360.0f;
    } else {
      dof.value = std::clamp(dof.value, dof.min_value, dof.max_value);
    }
    apply(dof);
    return true;
  }
  return false;
}

void Minifig::reset() {
  for (Dof &dof : dofs_) {
    dof.value = dof.default_value;
    apply(dof);
  }
}

void Minifig::apply(const Dof &dof) {
  if (dof.type == DofType::Rotation) {
    dof.node->set_rotation(dof.axis, dof.value);
  } else {
    dof.node->set_translation(dof.axis, dof.value);
  }
}

std::vector<float> Minifig::pose() const {
  std::vector<float> values;
  for (const auto &d : dofs_)
    values.push_back(d.value);
  return values;
}
void Minifig::set_pose(const std::vector<float> &values) {
  for (std::size_t i = 0; i < dofs_.size(); ++i) {
    dofs_[i].value = values.at(i);
    apply(dofs_[i]);
  }
}

glm::mat4 Minifig::hand_transform(bool left) const {
  return (left ? left_hand_ : right_hand_)->world_transform();
}
