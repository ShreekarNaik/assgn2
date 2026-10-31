#pragma once

#include <GL/glew.h>
#include <string>

namespace shader_util {

// Loads, compiles and links a shader program from file paths.
GLuint load_program(const std::string &vertex_path,
                    const std::string &fragment_path);

} // namespace shader_util
