#include "animation.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
std::vector<Keyframe> load_keyframes(const std::string &path) {
  std::ifstream in(path);
  if (!in)
    throw std::runtime_error("Cannot open " + path);
  std::vector<Keyframe> result;
  std::string line;
  while (std::getline(in, line)) {
    if (line.find_first_not_of(" \t\r") == std::string::npos)
      continue;
    std::istringstream row(line);
    Keyframe k;
    if (!(row >> k.frame) || k.frame < 0)
      throw std::runtime_error("Invalid timestamp");
    for (float &f : k.v)
      if (!(row >> f) || !std::isfinite(f))
        throw std::runtime_error("Expected 47 finite fields");
    std::string extra;
    if (row >> extra)
      throw std::runtime_error("Extra keyframe fields");
    for (int i : {0, 1, 2, 3, 4, 11, 42, 44, 45, 46}) {
      int limit = (i == 4 || i == 42) ? 2 : 1;
      if (k.v[i] != std::floor(k.v[i]) || k.v[i] < 0 || k.v[i] > limit)
        throw std::runtime_error("Invalid discrete field");
    }
    if (!result.empty() && k.frame <= result.back().frame)
      throw std::runtime_error("Timestamps must increase");
    result.push_back(k);
  }
  if (result.empty())
    throw std::runtime_error("No keyframes");
  return result;
}
void append_keyframe(const std::string &path, const Keyframe &k) {
  std::ofstream out(path, std::ios::app);
  if (!out)
    throw std::runtime_error("Cannot save keyframe");
  out.precision(9);
  out << k.frame;
  for (float v : k.v)
    out << ' ' << v;
  out << '\n';
  if (!out)
    throw std::runtime_error("Keyframe write failed");
}
namespace {
float foot_bottom(float hip, float knee) {
  constexpr float radians = 3.14159265f / 180.f;
  hip *= radians;
  knee = hip + knee * radians;
  float ankle = -.3f - .7f * std::cos(hip) - .6f * std::cos(knee);
  float corner = 1e6f;
  for (float y : {-.2f, 0.f})
    for (float z : {-.4f, .6f})
      corner = std::min(corner, y * std::cos(knee) - z * std::sin(knee));
  return ankle + corner;
}
void walk_cycle(Keyframe &pose, const std::vector<Keyframe> &keys, int segment,
                int figure) {
  int flag = 45 + figure, base = 12 + 15 * figure;
  if (pose.v[flag] == 0)
    return;
  int first = segment, last = segment;
  while (first > 0 && keys[first - 1].v[flag] != 0)
    --first;
  while (last + 1 < int(keys.size()) && keys[last].v[flag] != 0)
    ++last;
  auto distance = [&](const Keyframe &a, const Keyframe &b) {
    return std::hypot(b.v[base + 3] - a.v[base + 3],
                      b.v[base + 5] - a.v[base + 5]);
  };
  float travelled = 0, total = 0;
  for (int i = first; i < last; i++) {
    total += distance(keys[i], keys[i + 1]);
    if (i < segment)
      travelled += distance(keys[i], keys[i + 1]);
  }
  travelled += distance(keys[segment], pose);
  if (total < .001f)
    return;
  constexpr float pi = 3.14159265f;
  float stride = 4 * 1.4f * std::sin(14 * pi / 180);
  float envelope =
      std::clamp(std::min(travelled, total - travelled) / .35f, 0.f, 1.f);
  for (int side = 0; side < 2; side++) {
    float phase = std::fmod(travelled / stride + side * .5f, 1.f);
    float hip, knee = 0;
    if (phase < .5f)
      hip = 14 * (4 * phase - 1);
    else {
      float u = (phase - .5f) * 2;
      hip = 14 * (1 - 2 * u * u * (3 - 2 * u));
      knee = 35 * std::sin(pi * u);
    }
    pose.v[base + 9 + side] += hip * envelope;
    pose.v[base + 11 + side] += knee * envelope;
  }
  // A1 has no ankles. Let the fixed feet rock on their lowest edge.
  pose.v[base + 4] +=
      -1.8f - std::min(foot_bottom(pose.v[base + 9], pose.v[base + 11]),
                       foot_bottom(pose.v[base + 10], pose.v[base + 12]));
}
} // namespace
Keyframe sample(const std::vector<Keyframe> &keys, int frame) {
  if (frame <= keys.front().frame)
    return keys.front();
  if (frame >= keys.back().frame)
    return keys.back();
  auto next =
      std::upper_bound(keys.begin(), keys.end(), frame,
                       [](int f, const Keyframe &k) { return f < k.frame; });
  const auto &b = *next;
  const auto &a = *(next - 1);
  float t = float(frame - a.frame) / (b.frame - a.frame);
  Keyframe out = a;
  out.frame = frame;
  for (int i = 0; i < Fields; i++) {
    if (i < 5 || i == 11 || i == 42 || i >= 44)
      continue; // discrete values use the preceding key
    float delta = b.v[i] - a.v[i];
    if (i == 12 || i == 13 || i == 14 || i == 27 || i == 28 || i == 29)
      delta = std::remainder(delta, 360.f);
    out.v[i] = a.v[i] + t * delta;
  }
  int segment = int(next - keys.begin()) - 1;
  for (int f = 0; f < 2; f++)
    walk_cycle(out, keys, segment, f);
  return out;
}
