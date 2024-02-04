#version 330 core

in vec2 fTexCoords;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
  outColor = texture(tex, fTexCoords).rgba;
}
