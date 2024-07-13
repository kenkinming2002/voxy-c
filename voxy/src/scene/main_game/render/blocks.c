#include <voxy/scene/main_game/render/blocks.h>
#include <voxy/scene/main_game/render/assets.h>

#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/digger.h>

#include <voxy/graphics/camera.h>
#include <voxy/graphics/gl.h>

#include <voxy/dynamic_array.h>

#include <time.h>

struct vertex
{
  ivec3_t center;

  // Bits 0..2: normal index
  // Bits 3.15: texture index
  uint32_t normal_index_and_texture_index;

  // Bits 0..3:   base light level
  // Bits 4..7:   occlusion count 0
  // Bits 8..11:  occlusion count 1
  // Bits 12..15: occlusion count 2
  // Bits 16..20: occlusion count 3
  uint32_t light_level_and_occlusion_counts;

  float damage;
};
DYNAMIC_ARRAY_DEFINE(vertices, struct vertex);

static inline ivec3_t face_get_normal(enum block_face face)
{
  switch(face)
  {
  case BLOCK_FACE_LEFT:   return ivec3(-1,  0,  0);
  case BLOCK_FACE_RIGHT:  return ivec3( 1,  0,  0);
  case BLOCK_FACE_BACK:   return ivec3( 0, -1,  0);
  case BLOCK_FACE_FRONT:  return ivec3( 0,  1,  0);
  case BLOCK_FACE_BOTTOM: return ivec3( 0,  0, -1);
  case BLOCK_FACE_TOP:    return ivec3( 0,  0,  1);
  default:
    assert(0 && "Unreachable");
  }
}

static void chunk_mesh_init(struct chunk_mesh *chunk_mesh)
{
  glGenVertexArrays(1, &chunk_mesh->vao);
  glGenBuffers(1, &chunk_mesh->vbo);
  chunk_mesh->count = 0;
}

static void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct vertices vertices)
{
  glBindVertexArray(chunk_mesh->vao);

  // Vertices
  {
    glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.item_count * sizeof *vertices.items, vertices.items, GL_DYNAMIC_DRAW);
  }

  // Indices
  {
    static GLuint ebo = 0;
    if(ebo == 0)
    {
      static uint8_t indices[] = { 0, 2, 1, 1, 2, 3 };
      glGenBuffers(1, &ebo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
    }
    else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  }

  // Vertex attributes
  {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(struct vertex), (void *)offsetof(struct vertex, center));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(struct vertex), (void *)offsetof(struct vertex, normal_index_and_texture_index));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(struct vertex), (void *)offsetof(struct vertex, light_level_and_occlusion_counts));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, damage));

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
  }

  chunk_mesh->count = vertices.item_count;
}

static void chunk_mesh_render(struct chunk_mesh *chunk_mesh)
{
  if(chunk_mesh->count != 0)
  {
    glBindVertexArray(chunk_mesh->vao);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, chunk_mesh->count);
  }
}

static bool remesh_chunk(struct chunk *chunk)
{
  if(!chunk->data)
    return false;

  if(!chunk->mesh_invalidated)
    return false;

  chunk->mesh_invalidated = false;

  // We have decided not to use greedy meshing here as in we are unlikely to
  // have a lot of face that we could merge as our each of our blocks have an
  // associated light level.
  struct vertices opaque_vertices = {0};
  struct vertices transparent_vertices = {0};

  // Meshing
  {
    // Cache block lookup for good measure. This may or may not be a good idea as
    // the call to chunk_get_block() should be resonably fast, but it does make
    // the code easier to write. For blocks that are not present (because the
    // corresponding neighbour chunk is not loaded), the block id is set to
    // BLOCK_NONE.
    struct block blocks[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
    for(int z = -1; z<CHUNK_WIDTH+1; ++z)
      for(int y = -1; y<CHUNK_WIDTH+1; ++y)
        for(int x = -1; x<CHUNK_WIDTH+1; ++x)
        {
          struct block *block = chunk_get_block(chunk, ivec3(x, y, z));
          if(!block)
          {
            blocks[z+1][y+1][x+1].id = BLOCK_NONE;
            blocks[z+1][y+1][x+1].ether = false;
            blocks[z+1][y+1][x+1].light_level = 0;
          }
          else
            blocks[z+1][y+1][x+1] = *block;
        }

    // Cache block types also. Again, this may not be a good idea, but it does
    // make the code easier to write.
    enum block_type block_types[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
    for(int z = -1; z<CHUNK_WIDTH+1; ++z)
      for(int y = -1; y<CHUNK_WIDTH+1; ++y)
        for(int x = -1; x<CHUNK_WIDTH+1; ++x)
        {
          const block_id_t block_id = blocks[z+1][y+1][x+1].id;
          if(block_id != BLOCK_NONE)
            block_types[z+1][y+1][x+1] = query_block_info(block_id)->type;
          else
            block_types[z+1][y+1][x+1] = BLOCK_TYPE_OPAQUE;
        }

    // Compute occlusion. This one is actually a good idea.
    uint8_t occlusions[CHUNK_WIDTH+1][CHUNK_WIDTH+1][CHUNK_WIDTH+1];
    for(int z = 0; z<CHUNK_WIDTH+1; ++z)
      for(int y = 0; y<CHUNK_WIDTH+1; ++y)
        for(int x = 0; x<CHUNK_WIDTH+1; ++x)
        {
          uint8_t occlusion = 0;
          for(int dz = 0; dz<2; ++dz)
            for(int dy = 0; dy<2; ++dy)
              for(int dx = 0; dx<2; ++dx)
                if(block_types[z+dz][y+dy][x+dx] == BLOCK_TYPE_OPAQUE)
                  occlusion += 1;

          occlusions[z][y][x] = occlusion;
        }

    for(int z = 0; z<CHUNK_WIDTH; ++z)
      for(int y = 0; y<CHUNK_WIDTH; ++y)
        for(int x = 0; x<CHUNK_WIDTH; ++x)
        {
          /// Is this block being currently digged?
          const ivec3_t local_position = ivec3(x, y, z);
          const ivec3_t global_position = ivec3_add(ivec3_mul_scalar(chunk->position, CHUNK_WIDTH), local_position);
          const float damage = ivec3_eql(g_digger.position, global_position) ? g_digger.damage : 0.0f;

          for(enum block_face face = 0; face<BLOCK_FACE_COUNT; ++face)
          {
            const ivec3_t normal = face_get_normal(face);

            const struct block block = blocks[z+1][y+1][x+1];
            const enum block_type block_type = block_types[z+1][y+1][x+1];

            const struct block neighbour_block = blocks[z+normal.z+1][y+normal.y+1][x+normal.x+1];
            const enum block_type neighbour_block_type = block_types[z+normal.z+1][y+normal.y+1][x+normal.x+1];

            if(block_type != BLOCK_TYPE_OPAQUE && block_type != BLOCK_TYPE_TRANSPARENT)
              continue;

            if(block_type == BLOCK_TYPE_OPAQUE && neighbour_block_type == BLOCK_TYPE_OPAQUE)
              continue;

            if(block_type == BLOCK_TYPE_TRANSPARENT && neighbour_block_type == BLOCK_TYPE_TRANSPARENT)
              continue;

            if(block_type == BLOCK_TYPE_TRANSPARENT && neighbour_block_type == BLOCK_TYPE_OPAQUE)
              continue;

            struct vertex vertex;
            vertex.center = chunk->position;
            vertex.center = ivec3_mul_scalar(vertex.center, CHUNK_WIDTH);
            vertex.center = ivec3_add(vertex.center, ivec3(x, y, z));

            const uint32_t normal_index = face;
            const uint32_t texture_index = assets_get_block_texture_array_index(block.id, face);
            vertex.normal_index_and_texture_index = normal_index | texture_index << 3;

            uint16_t light_level = neighbour_block.ether ? 15 : neighbour_block.light_level;
            vertex.light_level_and_occlusion_counts = light_level;

            uint16_t occlusion_counts[2][2];
            const ivec3_t axis2 = normal.z != 0 ? ivec3(1, 0, 0) : ivec3(0, 0, 1);
            const ivec3_t axis1 = ivec3_cross(normal, axis2);
            for(int v = 0; v < 2; ++v)
              for(int u = 0; u < 2; ++u)
              {
                // Fixed-point!?
                ivec3_t position;
                position = ivec3_mul_scalar(ivec3(x, y, z), 2);
                position = ivec3_add(position, normal);
                position = ivec3_add(position, ivec3_mul_scalar(axis1, u * 2 - 1));
                position = ivec3_add(position, ivec3_mul_scalar(axis2, v * 2 - 1));
                position = ivec3_add(position, ivec3(1, 1, 1));
                position = ivec3_div_scalar(position, 2);

                occlusion_counts[v][u] = occlusions[position.z][position.y][position.x];
              }

            vertex.light_level_and_occlusion_counts |= occlusion_counts[0][0] << 4;
            vertex.light_level_and_occlusion_counts |= occlusion_counts[0][1] << 8;
            vertex.light_level_and_occlusion_counts |= occlusion_counts[1][0] << 12;
            vertex.light_level_and_occlusion_counts |= occlusion_counts[1][1] << 16;

            vertex.damage = damage;

            switch(block_type)
            {
              case BLOCK_TYPE_OPAQUE:      DYNAMIC_ARRAY_APPEND(opaque_vertices, vertex);      break;
              case BLOCK_TYPE_TRANSPARENT: DYNAMIC_ARRAY_APPEND(transparent_vertices, vertex); break;
              default:
                                           assert(0 && "Unreachable");
            }
          }
        }
  }

  // Mesh upload
  {
    if(!chunk->render_info)
    {
      chunk->render_info = malloc(sizeof *chunk->render_info);
      chunk_mesh_init(&chunk->render_info->opaque_mesh);
      chunk_mesh_init(&chunk->render_info->transparent_mesh);
    }

    chunk_mesh_update(&chunk->render_info->opaque_mesh, opaque_vertices);
    chunk_mesh_update(&chunk->render_info->transparent_mesh, transparent_vertices);
  }

  // Mesh cleanup
  {
    free(opaque_vertices.items);
    free(transparent_vertices.items);
  }

  return true;
}

static void remesh(void)
{
  size_t count = 0;
  clock_t begin = clock();

  // FIXME: Reintroduce the use of OpenMP.
  world_for_each_chunk(chunk)
    count += remesh_chunk(chunk);

  clock_t end = clock();
  if(count != 0)
    LOG_INFO("Chunk Mesher: Processed %zu chunks in %fs - Average %fs", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);
}

static void cull(void)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&world_camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&world_camera), VP);

  world_for_each_chunk(chunk)
    if(chunk->render_info)
    {
      fvec3_t min = fvec3(+INFINITY, +INFINITY, +INFINITY);
      fvec3_t max = fvec3(-INFINITY, -INFINITY, -INFINITY);
      for(int k=0; k<2; ++k)
        for(int j=0; j<2; ++j)
          for(int i=0; i<2; ++i)
          {
            fvec3_t point_world_space = fvec3_sub(ivec3_as_fvec3(ivec3_mul_scalar(ivec3_add(chunk->position, ivec3(i, j, k)), CHUNK_WIDTH)), fvec3(0.5f, 0.5f, 0.5f));
            fvec3_t point_clip_space  = fmat4_apply_fvec3_perspective_divide(VP, point_world_space);

            min = fvec3_min(min, point_clip_space);
            max = fvec3_max(max, point_clip_space);
          }

      chunk->render_info->culled = true;
      chunk->render_info->culled = chunk->render_info->culled && (min.x >= 1.0f || max.x <= -1.0f);
      chunk->render_info->culled = chunk->render_info->culled && (min.y >= 1.0f || max.y <= -1.0f);
      chunk->render_info->culled = chunk->render_info->culled && (min.z >= 1.0f || max.z <= -1.0f);
    }
}

static void render(void)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&world_camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&world_camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(&world_camera), V);

  struct gl_program program = GL_PROGRAM_LOAD(chunk);
  glUseProgram(program.id);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_2D_ARRAY, assets_get_block_texture_array().id);

  world_for_each_chunk(chunk)
    if(chunk->render_info && !chunk->render_info->culled)
      chunk_mesh_render(&chunk->render_info->opaque_mesh);

  world_for_each_chunk(chunk)
    if(chunk->render_info && !chunk->render_info->culled)
      chunk_mesh_render(&chunk->render_info->transparent_mesh);
}

void main_game_render_blocks(void)
{
  // It is obvious we do want to be remeshing a same chunk again every single
  // frame, so we must cache our mesh somewhere. The question is do we store it
  // directly in struct chunk like we currently do using the render_info field
  // which is a pointer to struct render_info, or do we maintain a separate hash
  // table.
  //
  // Advantage of the first method:
  //  - Less hash table lookup. (We can directly go from a chunk pointer to a
  //    render info pointer without doing a lookup in our separate hash table).
  //
  // Advantage of the second method:
  //  - Less coupling(That is subjective).
  //  - We do not need to have a one-to-one relationship from chunk to mesh. We
  //    can have more liberty in deciding which chunk meshes we want to be
  //    keeping around.
  //  - Faster iteration. We do not need to iterate through all loaded chunk and
  //    check if it has an associated mesh during rendering. By construction,
  //    the separate hash table only have an entry if an associated mesh exist.

  remesh();
  cull();
  render();
}
