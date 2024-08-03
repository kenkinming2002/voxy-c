#include <voxy/scene/main_game/render/assets.h>

#include <voxy/core/log.h>
#include <voxy/graphics/gl.h>
#include <voxy/graphics/mesh.h>
#include <voxy/dynamic_array.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct block_array_texture_info
{
  uint32_t indices[DIRECTION_COUNT];
};

static struct gl_texture_2d            item_textures[ITEM_MAX];
static struct gl_array_texture_2d      block_array_texture;
static struct block_array_texture_info block_array_texture_info[BLOCK_MAX];

static struct gl_texture_2d entity_textures[ENTITY_MAX];
static struct mesh          entity_meshes[ENTITY_MAX];

static void ensure_item_texture(item_id_t item_id)
{
  if(item_textures[item_id].id == 0)
  {
    const struct item_info *item_info = query_item_info(item_id);
    if(gl_texture_2d_load(&item_textures[item_id], item_info->texture) != 0)
    {
      LOG_WARN("Failed to load texture for item %s:%s", item_info->mod, item_info->name);
      LOG_ERROR("Fallback texture not implemented");
      exit(EXIT_FAILURE);
    }
  }
}

static void ensure_block_array_texture(void)
{
  if(block_array_texture.id == 0)
  {
    // 1: Collect all textures into an array
    DYNAMIC_ARRAY_DECLARE(textures, const char *);
    for(item_id_t block_id=0; block_id<BLOCK_MAX; ++block_id)
    {
      const struct block_info *block_info = query_block_info(block_id);
      for(unsigned direction=0; direction<DIRECTION_COUNT; ++direction)
        if(block_info->textures[direction])
        {
          size_t i;
          for(i=0; i<textures.item_count; ++i)
            if(strcmp(block_info->textures[direction], textures.items[i]) == 0)
              break;

          block_array_texture_info[block_id].indices[direction] = i;
          if(i == textures.item_count)
            DYNAMIC_ARRAY_APPEND(textures, block_info->textures[direction]);
        }
    }

    LOG_INFO("Collected block textures:");
    for(size_t i=0; i<textures.item_count; ++i)
      LOG_INFO("  %s", textures.items[i]);

    // 2: Build texture array
    if(gl_array_texture_2d_load(&block_array_texture, textures.item_count, textures.items) != 0)
    {
      LOG_ERROR("Failed to load block array texture");
      exit(EXIT_FAILURE);
    }
  }
}

static bool ensure_entity_texture(entity_id_t entity_id)
{
  if(entity_textures[entity_id].id == 0)
  {
    const struct entity_info *entity_info = query_entity_info(entity_id);
    if(!entity_info->texture)
      return false;

    if(gl_texture_2d_load(&entity_textures[entity_id], entity_info->texture) != 0)
    {
      LOG_WARN("Failed to load texture for entity %s:%s", entity_info->mod, entity_info->name);
      LOG_ERROR("Fallback texture not implemented");
      exit(EXIT_FAILURE);
    }
  }
  return true;
}

static bool ensure_entity_mesh(entity_id_t entity_id)
{
  if(entity_meshes[entity_id].vao == 0)
  {
    const struct entity_info *entity_info = query_entity_info(entity_id);
    if(!entity_info->mesh)
      return false;

    if(mesh_load(&entity_meshes[entity_id], entity_info->mesh) != 0)
    {
      LOG_WARN("Failed to load mesh for entity %s:%s", entity_info->mod, entity_info->name);
      LOG_ERROR("Fallback mesh not implemented");
      exit(EXIT_FAILURE);
    }
  }
  return true;
}


struct gl_texture_2d assets_get_item_texture(item_id_t item_id)
{
  assert(item_id != ITEM_NONE);
  ensure_item_texture(item_id);
  return item_textures[item_id];
}

struct gl_array_texture_2d assets_get_block_texture_array(void)
{
  ensure_block_array_texture();
  return block_array_texture;
}

uint32_t assets_get_block_texture_array_index(block_id_t block_id, direction_t direction)
{
  assert(block_id != BLOCK_NONE);
  ensure_block_array_texture();
  return block_array_texture_info[block_id].indices[direction];
}

const struct gl_texture_2d *assets_get_entity_texture(entity_id_t id)
{
  assert(id != ENTITY_NONE);
  if(ensure_entity_texture(id))
    return &entity_textures[id];
  else
    return NULL;
}

const struct mesh *assets_get_entity_mesh(entity_id_t id)
{
  assert(id != ENTITY_NONE);
  if(ensure_entity_mesh(id))
    return &entity_meshes[id];
  else
    return NULL;
}

