#include "ui.h"

#include "gl.h"
#include "lin.h"

struct ui_vertex
{
  struct vec2 position;
  struct vec2 texture_coords;
};

int ui_init(struct ui *ui)
{
  if((ui->program = gl_program_load("assets/ui.vert", "assets/ui.frag")) == 0)
    goto error1;

  glGenVertexArrays(1, &ui->vao);
  glBindVertexArray(ui->vao);
  return 0;

error1:
  return -1;
}

void ui_deinit(struct ui *ui)
{
  glDeleteVertexArrays(1, &ui->vao);
}

void ui_draw(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, GLuint texture)
{
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program);

  glUniform2f(glGetUniformLocation(ui->program, "window_size"), window_size.x, window_size.y);
  glUniform2f(glGetUniformLocation(ui->program, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program, "dimension"  ), dimension  .x, dimension  .y);

  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(ui->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
