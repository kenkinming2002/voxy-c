#ifndef ENTITY_WEIRD_WEIRD_H
#define ENTITY_WEIRD_WEIRD_H

#include <voxy/scene/main_game/types/entity.h>

void weird_entity_register(void);
entity_id_t weird_entity_id_get(void);

void weird_entity_init(struct entity *entity);
void weird_entity_fini(struct entity *entity);

void weird_entity_update(struct entity *entity, float dt);
void weird_entity_render(const struct entity *entity, const struct camera *camera);

#endif // ENTITY_WEIRD_WEIRD_H
