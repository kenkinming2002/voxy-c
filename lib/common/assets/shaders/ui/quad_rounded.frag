#version 330 core

in vec2 fLocalPosition;

out vec4 outColor;

uniform vec4  color;
uniform float radius;
uniform vec2  dimension;

void main()
{
  vec2 d1 = min(
      abs(fLocalPosition),
      abs(dimension - fLocalPosition)
  );

  if(d1.x < radius && d1.y < radius)
  {
    vec2 d2 = vec2(radius, radius) - d1;
    if(dot(d2, d2) > radius * radius)
      discard;
  }

  outColor = color;
}
