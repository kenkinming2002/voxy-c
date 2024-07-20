#include <voxy/math/direction.h>

int direction_signi(direction_t direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return -1.0;
  case DIRECTION_RIGHT:  return 1.0;
  case DIRECTION_BACK:   return -1.0;
  case DIRECTION_FRONT:  return 1.0;
  case DIRECTION_BOTTOM: return -1.0;
  case DIRECTION_TOP:    return 1.0;
  default:
    assert(0 && "Unreachable");
  }
}

float direction_signf(direction_t direction)
{
  return direction_signi(direction);
}

unsigned direction_axis(direction_t direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return 0;
  case DIRECTION_RIGHT:  return 0;
  case DIRECTION_BACK:   return 1;
  case DIRECTION_FRONT:  return 1;
  case DIRECTION_BOTTOM: return 2;
  case DIRECTION_TOP:    return 2;
  default:
    assert(0 && "Unreachable");
  }
}

ivec3_t direction_as_ivec(direction_t direction)
{
  ivec3_t result = ivec3_zero();
  result.values[direction_axis(direction)] = direction_signi(direction);
  return result;
}

fvec3_t direction_as_fvec(direction_t direction)
{
  return ivec3_as_fvec3(direction_as_ivec(direction));
}

