#version 460 core
in vec2 f_uv;

out vec4 out_color;

uniform sampler2D sampler;

void main()
{
  vec4 color = texture(sampler, f_uv);
  if(color.a <= 0.0f)
    discard;

  out_color = color;
}

