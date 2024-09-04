#ifndef VOXY_CLIENT_ENTITY_ENTITY_H
#define VOXY_CLIENT_ENTITY_ENTITY_H

#include <voxy/client/export.h>

#include <libcommon/math/vector.h>

struct voxy_entity;

VOXY_CLIENT_EXPORT fvec3_t voxy_entity_get_position(const struct voxy_entity *entity);
VOXY_CLIENT_EXPORT fvec3_t voxy_entity_get_rotation(const struct voxy_entity *entity);

VOXY_CLIENT_EXPORT void voxy_entity_set_position(struct voxy_entity *entity, fvec3_t position);
VOXY_CLIENT_EXPORT void voxy_entity_set_rotation(struct voxy_entity *entity, fvec3_t rotation);

#endif // VOXY_CLIENT_ENTITY_ENTITY_H
