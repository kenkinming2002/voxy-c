#ifndef LIBCOMMON_GRAPHICS_RENDER_H
#define LIBCOMMON_GRAPHICS_RENDER_H

#include <libcommon/math/transform.h>

#include "camera.h"
#include "mesh.h"
#include "gl.h"

/// Render a simple 3D model comprised of a single mesh and a single texture at
/// position specified by transform.
void render_model(struct camera camera, transform_t transform, struct mesh mesh, struct gl_texture_2d texture);

#endif // LIBCOMMON_GRAPHICS_RENDER_H
