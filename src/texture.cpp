#include "texture.hpp"
#include <fstream>
#include <stdexcept>
#include <vector>
GLuint load_texture(const std::string &path) {
  std::ifstream in(path);
  std::string magic;
  int w, h, max;
  if (!(in >> magic >> w >> h >> max) || magic != "P3" || w <= 0 || h <= 0 ||
      max != 255)
    throw std::runtime_error("Invalid P3 texture: " + path);
  std::vector<unsigned char> data(w * h * 3);
  for (auto &c : data) {
    int value;
    if (!(in >> value) || value < 0 || value > 255)
      throw std::runtime_error("Truncated texture");
    c = value;
  }
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
               data.data());
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return id;
}
