#ifndef ENTITY_DYNAMITE_DYNAMITE_H
#define ENTITY_DYNAMITE_DYNAMITE_H

#include <voxy/scene/main_game/types/entity.h>

struct dynamite_opaque
{
  float fuse;
};

void dynamite_entity_register(void);
entity_id_t dynamite_entity_id_get(void);

void dynamite_entity_init(struct entity *entity, float fuse);
void dynamite_entity_fini(struct entity *entity);

int dynamite_entity_serialize(const struct entity *entity, struct serializer *serializer);
int dynamite_entity_deserialize(struct entity *entity, struct deserializer *deserializer);

void dynamite_entity_update(struct entity *entity, float dt);
void dynamite_entity_render(const struct entity *entity, const struct camera *camera);

#endif // ENTITY_DYNAMITE_DYNAMITE_H
