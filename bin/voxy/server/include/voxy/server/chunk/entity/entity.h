#ifndef VOXY_SERVER_CHUNK_ENTITY_ENTITY_H
#define VOXY_SERVER_CHUNK_ENTITY_ENTITY_H

#include <voxy/server/export.h>

#include <libmath/vector.h>
#include <libmath/transform.h>

typedef uint32_t entity_handle_t;

struct voxy_entity;

VOXY_SERVER_EXPORT transform_t voxy_entity_get_transform(const struct voxy_entity *entity);
VOXY_SERVER_EXPORT fvec3_t voxy_entity_get_position(const struct voxy_entity *entity);
VOXY_SERVER_EXPORT fvec3_t voxy_entity_get_rotation(const struct voxy_entity *entity);

VOXY_SERVER_EXPORT void voxy_entity_set_transform(struct voxy_entity *entity, transform_t transform);
VOXY_SERVER_EXPORT void voxy_entity_set_position(struct voxy_entity *entity, fvec3_t position);
VOXY_SERVER_EXPORT void voxy_entity_set_rotation(struct voxy_entity *entity, fvec3_t rotation);

VOXY_SERVER_EXPORT bool voxy_entity_is_grounded(const struct voxy_entity *entity);
VOXY_SERVER_EXPORT void voxy_entity_apply_impulse(struct voxy_entity *entity, fvec3_t impulse);

VOXY_SERVER_EXPORT void *voxy_entity_get_opaque(const struct voxy_entity *entity);
VOXY_SERVER_EXPORT void voxy_entity_set_opaque(struct voxy_entity *entity, void *opaque);

#endif // VOXY_SERVER_CHUNK_ENTITY_ENTITY_H
