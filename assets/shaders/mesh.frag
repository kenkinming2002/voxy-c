#version 460 core
in vec2 f_uv;

out vec4 color;

uniform sampler2D sampler;

void main()
{
  color = texture(sampler, f_uv);
}

