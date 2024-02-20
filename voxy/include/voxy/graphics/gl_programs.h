#ifndef VOXY_GRAPHICS_GL_PROGRAMS_H
#define VOXY_GRAPHICS_GL_PROGRAMS_H

#define GL_PROGRAMS \
  X(ui_quad_rounded) \
  X(ui_texture) \
  X(chunk) \
  X(outline)

struct gl_program;

#define X(name) struct gl_program *gl_program_##name##_get();
GL_PROGRAMS
#undef X

#endif // VOXY_GRAPHICS_GL_PROGRAMS_H
