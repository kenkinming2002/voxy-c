#ifndef LIBCOMMON_GRAPHICS_RENDER_H
#define LIBCOMMON_GRAPHICS_RENDER_H

#include <libcommon/math/transform.h>

#include "camera.h"
#include "mesh.h"
#include "gl.h"

/// Render a simple 3D model comprised of a single mesh and a single texture at
/// position specified by transform.
///
/// The rendering is done only at the end when render_end() is called.
void render(const struct camera *camera, const struct mesh *mesh, const struct gl_texture_2d *texture, transform_t transform, float light);

/// End rendering.
///
/// This is when actual rendering occurs.
void render_end(void);

#endif // LIBCOMMON_GRAPHICS_RENDER_H
