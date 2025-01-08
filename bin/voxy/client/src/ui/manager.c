#include "manager.h"

#include <libgfx/window.h>
#include <libgfx/gl.h>
#include <libui/ui.h>

void ui_manager_update(void)
{
  ui_reset();

  struct gl_texture_2d cursor = GL_TEXTURE_2D_LOAD("bin/mod/assets/textures/cursor.png");

  const fvec2_t center = fvec2_mul_scalar(ivec2_as_fvec2(window_size), 0.5f);
  const fvec2_t dimension = fvec2(30.0f, 30.0f);
  const fvec2_t position = fvec2_sub(center, fvec2_mul_scalar(dimension, 0.5f));

  ui_rect_textured(position, dimension, 3.0f, cursor.id);
}
