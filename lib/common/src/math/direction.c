#include <libcommon/math/direction.h>

direction_t direction_from_sign_axis(bool sign, unsigned axis)
{
  switch(axis)
  {
  case 0: return sign ? DIRECTION_RIGHT : DIRECTION_LEFT;
  case 1: return sign ? DIRECTION_FRONT : DIRECTION_BACK;
  case 2: return sign ? DIRECTION_TOP : DIRECTION_BOTTOM;
  default:
    assert(0 && "Unreachable");
  }
}

direction_t direction_reverse(direction_t direction)
{
  return direction_from_sign_axis(!direction_sign(direction), direction_axis(direction));
}

bool direction_sign(direction_t direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return false;
  case DIRECTION_RIGHT:  return true;
  case DIRECTION_BACK:   return false;
  case DIRECTION_FRONT:  return true;
  case DIRECTION_BOTTOM: return false;
  case DIRECTION_TOP:    return true;
  default:
    assert(0 && "Unreachable");
  }
}

int direction_signi(direction_t direction)
{
  return direction_sign(direction) ? 1 : -1;
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

