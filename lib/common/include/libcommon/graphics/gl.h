#ifndef LIBCOMMON_GRAPHICS_GL_H
#define LIBCOMMON_GRAPHICS_GL_H

#include <libcommon/core/log.h>
#include <glad/glad.h>

#include <stdlib.h>
#include <stddef.h>

struct gl_shader { GLuint id; };
struct gl_program { GLuint id; };
struct gl_texture_2d { GLuint id; };
struct gl_array_texture_2d { GLuint id; };

int gl_shader_load(struct gl_shader *shader, GLenum target, const char *filepath);
void gl_shader_fini(struct gl_shader *shader);

int gl_program_load(struct gl_program *program, size_t count, GLenum targets[count], const char *filepaths[count]);
int gl_program_link(struct gl_program *program, size_t count, struct gl_shader shaders[count]);
void gl_program_fini(struct gl_program *program);

int gl_texture_2d_load(struct gl_texture_2d *texture_2d, const char *filepath);
void gl_texture_2d_fini(struct gl_texture_2d *texture_2d);

int gl_array_texture_2d_load(struct gl_array_texture_2d *array_texture_2d, size_t count, const char *filepaths[count]);
void gl_array_texture_2d_fini(struct gl_array_texture_2d *array_texture_2d);

#define GL_PROGRAM_LOAD(name)                                                                       \
({                                                                                                  \
    static struct gl_program instance;                                                              \
    if(instance.id == 0)                                                                            \
    {                                                                                               \
      GLenum      targets[]   = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};                             \
      const char *filepaths[] = {#name ".vert", #name ".frag"}; \
      if(gl_program_load(&instance, 2, targets, filepaths) != 0)                                    \
      {                                                                                             \
        LOG_ERROR("Failed to load %s shader", #name);                                               \
        exit(EXIT_FAILURE);                                                                         \
      }                                                                                             \
    }                                                                                               \
    instance;                                                                                       \
})

#endif // LIBCOMMON_GRAPHICS_GL_H
