#ifndef VOXY_GL_H
#define VOXY_GL_H

#include <glad/glad.h>
#include <stddef.h>

GLuint gl_shader_load(GLenum target, const char *filepath);
GLuint gl_program_link(GLuint vertex_shader, GLuint fragment_shader);
GLuint gl_program_load(const char *vertex_shader_filepath, const char *fragment_shader_filepath);

GLuint gl_array_texture_load(size_t count, const char *filepaths[count]);

#endif // VOXY_GL_H
