#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

/// Different scene for the game. By default, the game has no current scene
/// (i.e. SCENE_NONE).
enum Scene
{
  SCENE_NONE,
  SCENE_MAIN_MENU,
  SCENE_MAIN_GAME,
  SCENE_EXIT,
};

/// Switch to another scene. This would call *_leave() for the previous scene if
/// it exist and call *_enter() for the new scene. The scene switch will not
/// happen immediatly. The scene switch will only happen during the call to
/// scene_commit().
void scene_switch(enum Scene scene);

/// Dispatch to the underlying *_update function.
void scene_update(void);

/// Dispatch to the underlying *_render function.
void scene_render(void);

/// Commit switching scene, returning true if we should exit.
bool scene_commit(void);

#endif // SCENE_H
