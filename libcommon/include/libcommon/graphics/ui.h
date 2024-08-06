#ifndef LIBCOMMON_GRAPHICS_UI_H
#define LIBCOMMON_GRAPHICS_UI_H

#include <libcommon/math/vector.h>
#include <glad/glad.h>

/*
 * My implementation of some form of immediate mode GUI system. There are two
 * major functions ui_reset() and ui_render(), which should be called at the
 * beginning of update and before the end of render respectively. The system is
 * designed in such a way that update and render need not be synchronized. There
 * can be multiple updates before a single render and vice versa.
 */

/*
 * Reset the current UI State. This should be called at the beginning of each
 * update.
 */
void ui_reset(void);

/*
 * Rendering of textured/color quads. Basis of all UI elements and can also be
 * useful on its own. The rendered quad is anchored on bottom-left.
 */
void ui_quad_colored(fvec2_t position, fvec2_t dimension, float rounding, fvec4_t color);
void ui_rect_textured(fvec2_t position, fvec2_t dimension, float rounding, GLuint texture);

/*
 * Rendering of text, which in reality is a bunch of textured quads. Again, the
 * rendered text is anchored on bottom-left. Use the function ui_text_width() to
 * compute the proper position to center the text if desired.
 */
float ui_text_width(unsigned height, const char *str);
void ui_text(fvec2_t position, unsigned height, unsigned outline, const char *str);

/*
 * Render the current UI State. This is when OpenGL draw calls are actually
 * issued and should be called before the end of rendering.
 */
void ui_render(void);

/*
 * Buttons. The return result is bitwise-or of value from enum ui_button_result
 * to indicate if there is any mouse click on the button.
 */
enum ui_button_result
{
  UI_BUTTON_RESULT_HOVERED      = 1 << 0,
  UI_BUTTON_RESULT_CLICK_LEFT   = 1 << 1,
  UI_BUTTON_RESULT_CLICK_RIGHT  = 1 << 2,
  UI_BUTTON_RESULT_CLICK_MIDDLE = 1 << 3,
};
int ui_button(fvec2_t position, fvec2_t dimension);

#endif // LIBCOMMON_GRAPHICS_UI_H
