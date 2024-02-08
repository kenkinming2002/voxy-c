#include "gl.h"

#include <stb_image.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

static char *read_file(const char *filepath)
{
  FILE *file   = NULL;
  char *buffer = NULL;

  if(!(file = fopen(filepath, "r")))
  {
    fprintf(stderr, "ERROR: Failed to open file %s: %s\n", filepath, strerror(errno));
    goto error;
  }

  long begin = ftell(file); fseek(file, 0, SEEK_END);
  long end   = ftell(file); fseek(file, 0, SEEK_SET);
  long size = end - begin;

  buffer = malloc(size+1);
  if(fread(buffer, size, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: Failed to read from file %s\n", filepath);
    goto error;
  }
  buffer[size] = '\0';

  fclose(file);
  return buffer;

error:
  if(file) fclose(file);
  if(buffer) free(buffer);
  return NULL;
}

int gl_shader_load(struct gl_shader *shader, GLenum target, const char *filepath)
{
  char *shader_source;
  if(!(shader_source = read_file(filepath)))
    return -1;

  shader->id = glCreateShader(target);
  glShaderSource(shader->id, 1, (const char *const *)&shader_source, NULL);
  glCompileShader(shader->id);
  free(shader_source);

  GLint success;
  glGetShaderiv(shader->id, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    GLint  info_log_length;
    char  *info_log;

    glGetShaderiv(shader->id, GL_INFO_LOG_LENGTH, &info_log_length);
    info_log = malloc(info_log_length);
    glGetShaderInfoLog(shader->id, info_log_length, NULL, info_log);

    fprintf(stderr, "ERROR: Failed to compile shader: %s\n", info_log);
    free(info_log);

    glDeleteShader(shader->id);
    return -1;
  }

  return 0;
}

void gl_shader_fini(struct gl_shader *shader)
{
  glDeleteShader(shader->id);
}

int gl_program_load(struct gl_program *program, size_t count, GLenum targets[count], const char *filepaths[count])
{
  int result;

  struct gl_shader shaders[count];
  size_t           shader_count = 0;

  for(shader_count=0; shader_count<count; ++shader_count)
    if(gl_shader_load(&shaders[shader_count], targets[shader_count], filepaths[shader_count]) != 0)
    {
      result = -1;
      goto out;
    }

  result = gl_program_link(program, count, shaders);

out:
  for(size_t i=0; i<shader_count; ++i)
    gl_shader_fini(&shaders[i]);
  return result;
}

int gl_program_link(struct gl_program *program, size_t count, struct gl_shader shaders[count])
{
  program->id = glCreateProgram();
  for(size_t i=0; i<count; ++i)
    glAttachShader(program->id, shaders[i].id);
  glLinkProgram(program->id);

  GLint success;
  glGetProgramiv(program->id, GL_LINK_STATUS, &success);
  if(!success)
  {
    GLint  info_log_length;
    char  *info_log;

    glGetProgramiv(program->id, GL_INFO_LOG_LENGTH, &info_log_length);
    info_log = malloc(info_log_length);
    glGetProgramInfoLog(program->id, info_log_length, NULL, info_log);

    fprintf(stderr, "ERROR: Failed to link program: %s\n", info_log);
    free(info_log);

    glDeleteProgram(program->id);
    return -1;
  }

  return 0;
}

void gl_program_fini(struct gl_program *program)
{
  glDeleteProgram(program->id);
}

int gl_texture_2d_load(struct gl_texture_2d *texture_2d, const char *filepath)
{
  stbi_set_flip_vertically_on_load(1);

  int x, y, n;
  unsigned char *bytes = stbi_load(filepath, &x, &y, &n, 4);
  if(!bytes)
  {
    fprintf(stderr, "ERROR: Failed to load image for texture from %s: %s\n", filepath, stbi_failure_reason());
    stbi_image_free(bytes);
    return -1;
  }

  glGenTextures(1, &texture_2d->id);
  glBindTexture(GL_TEXTURE_2D, texture_2d->id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

  stbi_image_free(bytes);
  return 0;
}

void gl_texture_2d_fini(struct gl_texture_2d *texture_2d)
{
  glDeleteTextures(1, &texture_2d->id);
}

int gl_array_texture_2d_load(struct gl_array_texture_2d *array_texture_2d, size_t count, const char *filepaths[count])
{
  stbi_set_flip_vertically_on_load(1);

  int width, height;
  unsigned char *texels = NULL;
  for(size_t i=0; i<count; ++i)
  {
    int x, y, n;
    unsigned char *bytes = stbi_load(filepaths[i], &x, &y, &n, 4);
    if(!bytes)
    {
      fprintf(stderr, "ERROR: Failed to load image for array texture from %s: %s\n", filepaths[i], stbi_failure_reason());

      free(texels);
      stbi_image_free(bytes);
      return -1;
    }

    if(!texels)
    {
      width  = x;
      height = y;
      texels = malloc(count * width * height * 4 * sizeof *texels);
    }

    if(width != x || height != y)
    {
      fprintf(stderr, "ERROR: Array texture must be comprised of images of same dimension: Expected %dx%d: Got %dx%d from %s\n", width, height, x, y, filepaths[i]);

      free(texels);
      stbi_image_free(bytes);
      return -1;
    }

    memcpy(&texels[i * width * height * 4], bytes, width * height * 4);
    stbi_image_free(bytes);
  }

  glGenTextures(1, &array_texture_2d->id);
  glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture_2d->id);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, count, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);

  free(texels);
  return 0;
}

void gl_array_texture_2d_fini(struct gl_array_texture_2d *array_texture_2d)
{
  glDeleteTextures(1, &array_texture_2d->id);
}
