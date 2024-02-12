#include <voxy/types/chunk_mesh.h>
#include <stdlib.h>

void chunk_mesh_dispose(struct chunk_mesh *chunk_mesh)
{
  if(chunk_mesh)
  {
    glDeleteVertexArrays(1, &chunk_mesh->vao_opaque);
    glDeleteBuffers(1, &chunk_mesh->vbo_opaque);
    glDeleteBuffers(1, &chunk_mesh->ibo_opaque);

    free(chunk_mesh);
  }
}
