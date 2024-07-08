#include <voxy/math/direction.h>

fvec3_t direction_as_fvec(enum direction direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return fvec3(-1.0,  0.0,  0.0);
  case DIRECTION_RIGHT:  return fvec3( 1.0,  0.0,  0.0);
  case DIRECTION_BACK:   return fvec3( 0.0, -1.0,  0.0);
  case DIRECTION_FRONT:  return fvec3( 0.0,  1.0,  0.0);
  case DIRECTION_BOTTOM: return fvec3( 0.0,  0.0, -1.0);
  case DIRECTION_TOP:    return fvec3( 0.0,  0.0,  1.0);
  default:
    assert(0 && "Unreachable");
  }
}

ivec3_t direction_as_ivec(enum direction direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return ivec3(-1,  0,  0);
  case DIRECTION_RIGHT:  return ivec3( 1,  0,  0);
  case DIRECTION_BACK:   return ivec3( 0, -1,  0);
  case DIRECTION_FRONT:  return ivec3( 0,  1,  0);
  case DIRECTION_BOTTOM: return ivec3( 0,  0, -1);
  case DIRECTION_TOP:    return ivec3( 0,  0,  1);
  default:
    assert(0 && "Unreachable");
  }
}
