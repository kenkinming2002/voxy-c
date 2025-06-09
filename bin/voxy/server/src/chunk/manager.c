#include "manager.h"
#include "libcore/unreachable.h"

#include <stb_ds.h>
#include <empty.h>

struct voxy_chunk_region *chunk_regions;

void voxy_reset_chunk_regions(void)
{
  arrsetlen(chunk_regions, 0);
}

void voxy_add_chunk_region(struct voxy_chunk_region region)
{
  arrput(chunk_regions, region);
}

void iterate_major_chunk(void(*iterate)(ivec3_t chunk_position, void *data), void *data)
{
  for(size_t i=0; i<arrlenu(chunk_regions); ++i)
  {
    const struct voxy_chunk_region chunk_region = chunk_regions[i];
    switch(chunk_region.type)
    {
    case VOXY_ACTIVE_CHUNK_REGION_TYPE_SPHERE:
      for(int pz=0; pz*pz<=chunk_region.sphere.major_radius * chunk_region.sphere.major_radius; ++pz)
        for(int py=0; py*py+pz*pz<=chunk_region.sphere.major_radius * chunk_region.sphere.major_radius; ++py)
          for(int px=0; px*px+py*py+pz*pz<=chunk_region.sphere.major_radius * chunk_region.sphere.major_radius; ++px)
          {
            // This is a hacky way to visit all 8 quadrants of a sphere while
            // avoiding visiting boundary between quadrants more than once. We
            // really really want the compiler to unroll this small loop and
            // realize that there is a lot of opportunity for constant
            // propagation.
            #pragma omp unroll full
            for(int sz=0; sz<2; ++sz)
              #pragma omp unroll full
              for(int sy=0; sy<2; ++sy)
                #pragma omp unroll full
                  for(int sx=0; sx<2; ++sx)
                  {
                    if(sz && pz == 0) continue;
                    if(sy && py == 0) continue;
                    if(sx && px == 0) continue;

                    const int z = sz ? -pz : pz;
                    const int y = sy ? -py : py;
                    const int x = sx ? -px : px;

                    iterate(ivec3_add(chunk_region.sphere.center, ivec3(x, y, z)), data);
                  }
          }

      break;
    default:
      LIBCORE_UNREACHABLE;
    }
  }
}

bool is_minor_chunk(ivec3_t chunk_position)
{

  for(size_t i=0; i<arrlenu(chunk_regions); ++i)
  {
    const struct voxy_chunk_region chunk_region = chunk_regions[i];
    switch(chunk_region.type)
    {
    case VOXY_ACTIVE_CHUNK_REGION_TYPE_SPHERE:
      if(ivec3_length_squared(ivec3_sub(chunk_position, chunk_region.sphere.center)) <= chunk_regions->sphere.minor_radius * chunk_regions->sphere.minor_radius)
        return true;
      break;
    default:
      LIBCORE_UNREACHABLE;
    }
  }

  return false;
}

