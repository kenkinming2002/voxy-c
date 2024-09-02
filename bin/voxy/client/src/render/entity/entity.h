#ifndef RENDER_ENTITY_ENTITY_H
#define RENDER_ENTITY_ENTITY_H

#include "entity/registry.h"
#include "entity/manager.h"

void entity_renderer_render(struct entity_registry *entity_registry, struct entity_manager *entity_manager, const struct camera *camera);

#endif // RENDER_ENTITY_ENTITY_H
