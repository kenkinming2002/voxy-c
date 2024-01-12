#include <voxy/gl.h>

#include <stb_image.h>
#include <stdio.h>

static char *read_file(const char *filepath)
{
  FILE *file   = NULL;
  char *buffer = NULL;

  if(!(file = fopen(filepath, "r")))
    goto error;

  long begin = ftell(file); fseek(file, 0, SEEK_END);
  long end   = ftell(file); fseek(file, 0, SEEK_SET);
  long size = end - begin;

  buffer = malloc(size+1);
  if(fread(buffer, size, 1, file) != 1)
    goto error;
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
  GLuint  shader        = 0;
  char   *shader_source = NULL;

  if((shader = glCreateShader(target)) == 0)
    goto error;

  if(!(shader_source = read_file(filepath)))
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

    fprintf(stderr, "ERROR: Failed to compile OpenGL shader: %s\n", info_log);

    free(info_log);
    goto error;
  }

  free(shader_source);
  return shader;

error:
  if(shader) glDeleteShader(shader);
  if(shader_source) free(shader_source);
  return 0;
}

GLuint gl_program_link(GLuint vertex_shader, GLuint fragment_shader)
{
  GLuint program = 0;

  if((program = glCreateProgram()) == 0)
    goto error;

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

    fprintf(stderr, "ERROR: Failed to link OpenGL program: %s\n", info_log);

    free(info_log);
    goto error;
  }

  return program;

error:
  if(program) glDeleteProgram(program);
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


GLuint gl_cube_map_load(const char *filepaths[6])
{
  GLuint cube_map;
  glGenTextures(1, &cube_map);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  for(unsigned i=0; i<6; ++i)
  {
    int x, y, n;
    unsigned char *data = stbi_load(filepaths[i], &x, &y, &n, 3);
    if(!data)
    {
      fprintf(stderr, "ERROR: Failed to load cube map image from %s: %s\n", filepaths[i], stbi_failure_reason());
      glDeleteTextures(1, &cube_map);
      return 0;
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }

  return cube_map;
}

