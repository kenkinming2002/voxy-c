#version 460 core
in vec2 f_uv;

out vec4 out_color;

uniform float light;
uniform sampler2D sampler;

void main()
{
  vec4 color = texture(sampler, f_uv);
  if(color.a <= 0.0f)
    discard;

  float factor = mix(light, 1.0, 0.01);
  out_color = vec4(vec3(color) * light, 1.0);
}

