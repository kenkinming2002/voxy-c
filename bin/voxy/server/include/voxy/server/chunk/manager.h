#ifndef VOXY_SERVER_CHUNK_MANAGER_H
#define VOXY_SERVER_CHUNK_MANAGER_H

/// Below is how mod can control which chunks are to be
/// generated/loaded/unloaded.
///
/// User can call voxy_add_chunk_region() to add a region to a global list of
/// chunk regions. The global list of chunk regions will be reset on each frame
/// so user must arrange to call voxy_add_chunk_region() on each frame such as
/// in the update() callback for entity.
///
/// Each chunk region is split into two parts - major and minor - where the
/// minor part always contain the major part. We will:
///   - try to generate/load chunks inside major part of any chunk region
///   - try to unload chunks outside of minor part of any chunk region
///
/// The rationale for doing so is to prevent constant loading/unloading of
/// chunks as players go back and forth between a small set of chunks.

#include <voxy/server/export.h>
#include <libmath/vector.h>

enum voxy_chunk_region_type
{
  VOXY_ACTIVE_CHUNK_REGION_TYPE_SPHERE,
};

struct voxy_chunk_region
{
  enum voxy_chunk_region_type type;
  union {
    struct {
      ivec3_t center;
      int major_radius;
      int minor_radius;
    } sphere;
  };

};

VOXY_SERVER_EXPORT void voxy_add_chunk_region(struct voxy_chunk_region region);

static inline void voxy_add_chunk_region_sphere(ivec3_t center, int major_radius, int minor_radius)
{
  struct voxy_chunk_region region;
  region.type = VOXY_ACTIVE_CHUNK_REGION_TYPE_SPHERE;
  region.sphere.center = center;
  region.sphere.major_radius = major_radius;
  region.sphere.minor_radius = minor_radius;
  voxy_add_chunk_region(region);
}

#endif // VOXY_SERVER_CHUNK_MANAGER_H
