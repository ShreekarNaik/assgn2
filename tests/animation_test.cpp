#include "animation.hpp"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
int main() {
  auto keys = load_keyframes("keyframes.txt");
  assert(keys.front().frame == 0 && keys.back().frame >= 1800);
  for (int f = 0; f <= keys.back().frame; f++)
    for (float v : sample(keys, f).v)
      assert(std::isfinite(v));
  Keyframe a, b;
  a.frame = 0;
  b.frame = 10;
  a.v[0] = 1;
  b.v[0] = 0;
  a.v[12] = 170;
  b.v[12] = -170;
  b.v[15] = 4;
  auto mid = sample({a, b}, 5);
  assert(mid.v[0] == 1 && mid.v[12] == 180 && mid.v[15] == 2);
  assert(sample({a, b}, 10).v[0] == 0);
  assert(sample({a, b}, -1).frame == 0 && sample({a, b}, 20).frame == 10);
  Keyframe start, end;
  start.frame = 0;
  end.frame = 30;
  start.v[16] = end.v[16] = -.1f;
  end.v[15] = 2;
  start.v[45] = 1;
  auto walking = sample({start, end}, 15);
  assert(std::abs(walking.v[21]) + std::abs(walking.v[22]) > .1f);
  assert(walking.v[15] == 1.f); // cycle must not alter the authored route
  start.v[45] = 0;
  auto posed = sample({start, end}, 15);
  assert(posed.v[21] == 0 && posed.v[22] == 0 && posed.v[16] == -.1f);
  const std::string path = "/tmp/cs675-a2-test-keyframes.txt";
  {
    std::ofstream clear(path);
  }
  append_keyframe(path, a);
  append_keyframe(path, b);
  assert(load_keyframes(path).size() == 2);
  {
    std::ofstream corrupt(path);
    corrupt << "0 1 2\n";
  }
  bool failed = false;
  try {
    load_keyframes(path);
  } catch (...) {
    failed = true;
  }
  assert(failed);
  std::remove(path.c_str());
  std::cout << "Animation: timeline, interpolation, angle wrap, light cuts, "
               "round-trip and malformed input passed\n";
}
