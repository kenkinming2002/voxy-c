#include <libcore/log.h>

#include <libgfx/window.h>
#include <libgfx/gl.h>
#include <libgfx/camera.h>

#include <glad/glad.h>
#include <png.h>

#include <stdio.h>
#include <string.h>

static void init_opengl(int width, int height);

static void framebuffer_setup(int width, int height);
static void framebuffer_save(const char *output, int width, int height);

static void render_block(const char *output, const char *left, const char *right, const char *top, int width, int height);

void usage(const char *progname)
{
  fprintf(stderr, "Usage: %s -o OUTPUT -w width -h height LEFT RIGHT TOP\n", progname);
  exit(EXIT_FAILURE);
}

int main(int argc, const char *argv[])
{
  const char *output = NULL;

  int width = -1;
  int height = -1;

  const char *left = NULL;
  const char *right = NULL;
  const char *top = NULL;

  for (int i=1; i<argc; ++i)
  {
    if(strcmp(argv[i], "-o") == 0)
    {
      if(output)
      {
        LOG_ERROR("-o already specified");
        usage(argv[0]);
      }
      output = argv[++i];
      if(!output)
      {
        LOG_ERROR("-o must be followed by an argument");
        usage(argv[0]);
      }
    }
    else if(strcmp(argv[i], "-w") == 0)
    {
      if(width != -1)
      {
        LOG_ERROR("-w already specified");
        usage(argv[0]);
      }

      const char *s = argv[++i];
      if(!s)
      {
        LOG_ERROR("-w must be followed by an argument");
        usage(argv[0]);
      }

      width = atoi(s);
      if(width <= 0)
      {
        LOG_ERROR("width must be positive");
        usage(argv[0]);
      }
    }
    else if(strcmp(argv[i], "-h") == 0)
    {
      if(height != -1)
      {
        LOG_ERROR("-h already specified");
        usage(argv[0]);
      }

      const char *s = argv[++i];
      if(!s)
      {
        LOG_ERROR("-h must be followed by an argument");
        usage(argv[0]);
      }

      height = atoi(s);
      if(height <= 0)
      {
        LOG_ERROR("height must be positive");
        usage(argv[0]);
      }
    }
    else if(!left)
      left = argv[i];
    else if(!right)
      right = argv[i];
    else if(!top)
      top = argv[i];
    else
    {
      LOG_ERROR("too many arguments");
      usage(argv[0]);
    }
  }

  if(!output)
  {
      LOG_ERROR("output must be specified");
      usage(argv[0]);
  }

  if(width == -1)
  {
      LOG_ERROR("width must be specified");
      usage(argv[0]);
  }

  if(height == -1)
  {
      LOG_ERROR("height must be specified");
      usage(argv[0]);
  }

  if(!left)
  {
      LOG_ERROR("left must be specified");
      usage(argv[0]);
  }

  if(!right)
  {
      LOG_ERROR("right must be specified");
      usage(argv[0]);
  }

  if(!top)
  {
      LOG_ERROR("top must be specified");
      usage(argv[0]);
  }

  LOG_INFO("Rendering block with:");
  LOG_INFO("  output = %s", output);
  LOG_INFO("  width = %u", width);
  LOG_INFO("  height = %u", height);
  LOG_INFO("  left = %s", left);
  LOG_INFO("  right = %s", right);
  LOG_INFO("  top = %s", top);

  init_opengl(width, height);
  render_block(output, left, right, top, width, height);
}

static void init_opengl(int width, int height)
{
  // We do not care, we just need an OpenGL context, and unforunately, we need
  // to create a window for it.
  window_init("hidden", width, height);
}

static void framebuffer_setup(int width, int height)
{
  GLuint fbo;
  glCreateFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
}

static void framebuffer_save(const char *output, int width, int height)
{
  unsigned char data[height][width][4];
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  for(int y=0; y<height/2; ++y)
    for(int x=0; x<width; ++x)
      for(size_t i=0; i<4; ++i)
      {
        const unsigned char tmp = data[y][x][i];
        data[y][x][i] = data[height-1-y][x][i];
        data[height-1-y][x][i] = tmp;
      }

  png_image image;
  memset(&image, 0, sizeof image);
  image.version = PNG_IMAGE_VERSION;
  image.width = width;
  image.height = height;
  image.format = PNG_FORMAT_RGBA;
  png_image_write_to_file(&image, output, 0, data, 0, NULL);
  if(image.warning_or_error & PNG_IMAGE_ERROR)
    LOG_ERROR("Failed to save %s: %s\n", output, image.message);
  else if(image.warning_or_error & PNG_IMAGE_WARNING)
    LOG_WARN("Failed to save %s: %s\n", output, image.message);
}

static float degree_to_radian(float angle)
{
  return angle / 180.0f * M_PI;
}

static void render_block(const char *output, const char *left, const char *right, const char *top, int width, int height)
{
  framebuffer_setup(width, height);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  struct gl_program program;

  GLenum targets[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
  const char *filepaths[] = {"bin/tools/render_block/src/render_block.vert", "bin/tools/render_block/src/render_block.frag"};
  if(gl_program_load(&program, 2, targets, filepaths) != 0)
  {
    LOG_ERROR("Failed to load shader program");
    return;
  }

  glUseProgram(program.id);

  struct camera camera;

  camera.fovy   = degree_to_radian(60.0f);
  camera.near   = 0.1f;
  camera.far    = 1000.0f;
  camera.aspect = (float)width / (float)height;

  camera.transform.translation = fvec3(0.8f, 0.8f, 1.4f);
  camera.transform.rotation.yaw = degree_to_radian(135.0f);
  camera.transform.rotation.pitch = degree_to_radian(-50.0f);
  camera.transform.rotation.roll = degree_to_radian(0.0f);

  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&camera), VP);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

  struct gl_texture_2d left_texture;
  if(gl_texture_2d_load(&left_texture, left) != 0)
  {
    LOG_ERROR("Failed to load left texture");
    return;
  }

  struct gl_texture_2d right_texture;
  if(gl_texture_2d_load(&right_texture, right) != 0)
  {
    LOG_ERROR("Failed to load right texture");
    return;
  }

  struct gl_texture_2d top_texture;
  if(gl_texture_2d_load(&top_texture, top) != 0)
  {
    LOG_ERROR("Failed to load top texture");
    return;
  }

  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, left_texture.id);
  glUniform1i(glGetUniformLocation(program.id, "left"), 0);

  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, right_texture.id);
  glUniform1i(glGetUniformLocation(program.id, "right"), 1);

  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, top_texture.id);
  glUniform1i(glGetUniformLocation(program.id, "top"), 2);

  GLuint vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 18);

  framebuffer_save(output, width, height);
}

