#include "gl.h"

#include "stb_image.h"

#include <assert.h>
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

GLuint gl_shader_load(GLenum target, const char *filepath)
{
  GLuint shader = glCreateShader(target);

  char *shader_source = read_file(filepath);
  if(!shader_source)
    goto error;

  glShaderSource(shader, 1, (const char *const *)&shader_source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    GLint  info_log_length;
    char  *info_log;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    info_log = malloc(info_log_length);
    glGetShaderInfoLog(shader, info_log_length, NULL, info_log);

    fprintf(stderr, "ERROR: Failed to compile shader: %s\n", info_log);
    free(info_log);
    goto error;
  }

  free(shader_source);
  return shader;

error:
  glDeleteShader(shader);
  if(shader_source)
    free(shader_source);

  return 0;
}

GLuint gl_program_link(GLuint vertex_shader, GLuint fragment_shader)
{
  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if(!success)
  {
    GLint  info_log_length;
    char  *info_log;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
    info_log = malloc(info_log_length);
    glGetProgramInfoLog(program, info_log_length, NULL, info_log);

    fprintf(stderr, "ERROR: Failed to link program: %s\n", info_log);
    free(info_log);
    goto error;
  }

  return program;

error:
  glDeleteProgram(program);
  return 0;
}

GLuint gl_program_load(const char *vertex_shader_filepath, const char *fragment_shader_filepath)
{
  GLuint program         = 0;
  GLuint vertex_shader   = 0;
  GLuint fragment_shader = 0;

  if((vertex_shader = gl_shader_load(GL_VERTEX_SHADER, vertex_shader_filepath)) == 0)
    goto error;

  if((fragment_shader = gl_shader_load(GL_FRAGMENT_SHADER, fragment_shader_filepath)) == 0)
    goto error;

  if((program = gl_program_link(vertex_shader, fragment_shader)) == 0)
    goto error;

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  return program;

error:
  if(vertex_shader)   glDeleteShader(vertex_shader);
  if(fragment_shader) glDeleteShader(fragment_shader);
  return 0;
}

GLuint gl_array_texture_load(size_t count, const char *filepaths[count])
{
  stbi_set_flip_vertically_on_load(1);

  size_t width = 0, height = 0;
  unsigned char *texels = NULL;
  for(size_t i=0; i<count; ++i)
  {
    int x, y, n;
    unsigned char *data = stbi_load(filepaths[i], &x, &y, &n, 3);
    if(!data)
    {
      fprintf(stderr, "ERROR: Failed to load image for array texture from %s: %s\n", filepaths[i], stbi_failure_reason());
      free(texels);
      return 0;
    }

    if(!texels)
    {
      width  = x;
      height = y;
      texels = malloc(count * width * height * 3);
    }
    else if((size_t)x != width || (size_t)y != height)
    {
      fprintf(stderr, "ERROR: Array texture must be comprised of images of same dimension: Expected %zux%zu: Got %dx%d from %s\n", width, height, x, y, filepaths[i]);
      stbi_image_free(data);
      free(texels);
      return 0;
    }

    memcpy(&texels[i * width * height * 3], data, width * height * 3);
    stbi_image_free(data);
  }

  GLuint cube_map;

  glGenTextures(1, &cube_map);
  glBindTexture(GL_TEXTURE_2D_ARRAY, cube_map);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, count, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);

  free(texels);
  return cube_map;
}
