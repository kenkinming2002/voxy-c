#include <graphics/gl_programs.h>
#include <graphics/gl.h>

#include <stdlib.h>

#define X(name)                                                                            \
  static struct gl_program gl_program_##name##_instance;                                   \
                                                                                           \
  static void gl_program_##name##_atexit(void)                                             \
  {                                                                                        \
    gl_program_fini(&gl_program_##name##_instance);                                        \
  }                                                                                        \
                                                                                           \
  struct gl_program *gl_program_##name##_get()                                             \
  {                                                                                        \
    if(gl_program_##name##_instance.id == 0)                                               \
    {                                                                                      \
      static GLenum      targets[]   = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};             \
      static const char *filepaths[] = {"assets/" #name ".vert", "assets/" #name ".frag"}; \
      if(gl_program_load(&gl_program_##name##_instance, 2, targets, filepaths) != 0)       \
        exit(EXIT_FAILURE);                                                                \
                                                                                           \
      atexit(gl_program_##name##_atexit);                                                  \
    }                                                                                      \
    return &gl_program_##name##_instance;                                                  \
  }

GL_PROGRAMS
