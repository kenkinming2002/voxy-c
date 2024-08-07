#include <libcommon/graphics/render.h>

void render_model(struct camera camera, transform_t transform, struct mesh mesh, struct gl_texture_2d texture, float light)
{
  struct gl_program program = GL_PROGRAM_LOAD(libcommon/assets/shaders/model);
  glUseProgram(program.id);

  fmat4_t MVP = fmat4_identity();
  MVP = fmat4_mul(transform_matrix(transform), MVP);
  MVP = fmat4_mul(camera_view_matrix(&camera), MVP);
  MVP = fmat4_mul(camera_projection_matrix(&camera), MVP);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "MVP"), 1, GL_TRUE, (const float *)&MVP);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture.id);

  glUniform1f(glGetUniformLocation(program.id, "light"), light);

  glBindVertexArray(mesh.vao);
  glDrawArrays(GL_TRIANGLES, 0, mesh.count);
}

