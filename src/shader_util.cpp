#include "shader_util.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace shader_util {
namespace {

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file: " + path);
  }
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

GLuint compile_shader(GLenum type, const std::string &source) {
  const GLuint shader = glCreateShader(type);
  const char *src_ptr = source.c_str();
  glShaderSource(shader, 1, &src_ptr, nullptr);
  glCompileShader(shader);

  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok != GL_TRUE) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    glDeleteShader(shader);
    throw std::runtime_error(std::string("Shader compile error:\n") + log);
  }
  return shader;
}

} // namespace

GLuint load_program(const std::string &vertex_path,
                    const std::string &fragment_path) {
  std::string vert_src = read_file(vertex_path);
  std::string frag_src = read_file(fragment_path);

  const GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vert_src);
  const GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, frag_src);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLint ok = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (ok != GL_TRUE) {
    char log[1024];
    glGetProgramInfoLog(program, sizeof(log), nullptr, log);
    glDeleteProgram(program);
    throw std::runtime_error(std::string("Shader link error:\n") + log);
  }
  return program;
}

} // namespace shader_util
