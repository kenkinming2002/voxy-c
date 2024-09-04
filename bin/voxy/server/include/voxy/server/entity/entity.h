#ifndef VOXY_SERVER_ENTITY_ENTITY_H
#define VOXY_SERVER_ENTITY_ENTITY_H

#include <voxy/server/export.h>

#include <libcommon/math/vector.h>

struct voxy_entity;

VOXY_SERVER_EXPORT fvec3_t voxy_entity_get_position(const struct voxy_entity *entity);
VOXY_SERVER_EXPORT fvec3_t voxy_entity_get_rotation(const struct voxy_entity *entity);

VOXY_SERVER_EXPORT void voxy_entity_set_position(struct voxy_entity *entity, fvec3_t position);
VOXY_SERVER_EXPORT void voxy_entity_set_rotation(struct voxy_entity *entity, fvec3_t rotation);

#endif // VOXY_SERVER_ENTITY_ENTITY_H
