#ifndef RENDER_ENTITY_ENTITY_H
#define RENDER_ENTITY_ENTITY_H

#include "registry/entity.h"
#include "entity/manager.h"
#include "camera/manager.h"

void entity_renderer_render(struct voxy_entity_registry *entity_registry, struct entity_manager *entity_manager, struct camera_manager *camera_manager);

#endif // RENDER_ENTITY_ENTITY_H
