#ifndef RENDER_ENTITY_ENTITY_H
#define RENDER_ENTITY_ENTITY_H

#include <voxy/client/registry/entity.h>

#include "chunk/entity/manager.h"
#include "camera/manager.h"

void entity_renderer_render(struct entity_manager *entity_manager, struct camera_manager *camera_manager);

#endif // RENDER_ENTITY_ENTITY_H
