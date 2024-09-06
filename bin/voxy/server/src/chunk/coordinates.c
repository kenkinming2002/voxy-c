#include "coordinates.h"

#include <voxy/protocol/chunk.h>

ivec3_t get_chunk_position_i(ivec3_t position)
{
  return ivec3_div_scalar(ivec3_sub(position, global_position_to_local_position_i(position)), VOXY_CHUNK_WIDTH);
}

ivec3_t get_chunk_position_f(fvec3_t position)
{
  return get_chunk_position_i(fvec3_as_ivec3_round(position));
}

ivec3_t local_position_to_global_position_i(ivec3_t position, ivec3_t chunk_position)
{
  return ivec3_add(ivec3_mul_scalar(chunk_position, VOXY_CHUNK_WIDTH), position);
}

fvec3_t local_position_to_global_position_f(fvec3_t position, ivec3_t chunk_position)
{
  return fvec3_add(ivec3_as_fvec3(ivec3_mul_scalar(chunk_position, VOXY_CHUNK_WIDTH)), position);
}

ivec3_t global_position_to_local_position_i(ivec3_t position)
{
  for(int i=0; i<3; ++i)
  {
    position.values[i] %= VOXY_CHUNK_WIDTH;
    position.values[i] += VOXY_CHUNK_WIDTH;
    position.values[i] %= VOXY_CHUNK_WIDTH;
  }
  return position;
}

fvec3_t global_position_to_local_position_f(fvec3_t position)
{
  for(int i=0; i<3; ++i)
  {
    position.values[i] += 0.5f;
    position.values[i] = fmodf(position.values[i], VOXY_CHUNK_WIDTH);
    position.values[i] -= 0.5f;
  }
  return position;
}

