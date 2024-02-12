#ifndef GRAPHICS_GL_PROGRAMS_H
#define GRAPHICS_GL_PROGRAMS_H

#define GL_PROGRAMS \
  X(ui_quad) \
  X(ui_quad_rounded) \
  X(ui_texture) \
  X(ui_texture_mono) \
  X(chunk) \
  X(outline)

struct gl_program;

#define X(name) struct gl_program *gl_program_##name##_get();
GL_PROGRAMS
#undef X

#endif // GRAPHICS_GL_PROGRAMS_H
