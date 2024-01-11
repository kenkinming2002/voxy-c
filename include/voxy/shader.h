#ifndef VOXY_SHADER_H
#define VOXY_SHADER_H

#include <glad/glad.h>

GLuint gl_shader_load(GLenum target, const char *filepath);
GLuint gl_program_link(GLuint vertex_shader, GLuint fragment_shader);
GLuint gl_program_load(const char *vertex_shader_filepath, const char *fragment_shader_filepath);

#endif // VOXY_SHADER_H
