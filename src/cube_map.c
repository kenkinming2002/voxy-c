#include <voxy/cube_map.h>

#include <stb_image.h>
#include <stdio.h>

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
