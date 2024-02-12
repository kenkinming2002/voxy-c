CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

VOXY_SRCS += voxy/bundled/src/glad.c
VOXY_SRCS += voxy/bundled/src/stb_image.c

VOXY_SRCS += voxy/src/types/chunk_data.c
VOXY_SRCS += voxy/src/types/chunk_mesh.c
VOXY_SRCS += voxy/src/types/chunk_hash_table.c
VOXY_SRCS += voxy/src/types/entity.c
VOXY_SRCS += voxy/src/types/world.c

VOXY_SRCS += voxy/src/core/window.c
VOXY_SRCS += voxy/src/core/delta_time.c

VOXY_SRCS += voxy/src/graphics/gl.c
VOXY_SRCS += voxy/src/graphics/gl_programs.c
VOXY_SRCS += voxy/src/graphics/font_set.c
VOXY_SRCS += voxy/src/graphics/ui.c
VOXY_SRCS += voxy/src/graphics/camera.c

VOXY_SRCS += voxy/src/main_game/world.c
VOXY_SRCS += voxy/src/main_game/mod.c
VOXY_SRCS += voxy/src/main_game/mod_assets.c
VOXY_SRCS += voxy/src/main_game/chunk_generate.c
VOXY_SRCS += voxy/src/main_game/player_spawn.c
VOXY_SRCS += voxy/src/main_game/player_camera.c
VOXY_SRCS += voxy/src/main_game/player_movement.c
VOXY_SRCS += voxy/src/main_game/player_action.c
VOXY_SRCS += voxy/src/main_game/light.c
VOXY_SRCS += voxy/src/main_game/physics.c
VOXY_SRCS += voxy/src/main_game/chunk_remesh.c
VOXY_SRCS += voxy/src/main_game/ui.c
VOXY_SRCS += voxy/src/main_game/world_render.c
VOXY_SRCS += voxy/src/main_game/main_game.c

VOXY_SRCS += voxy/src/thread_pool.c
VOXY_SRCS += voxy/src/voxy.c

MOD_SRCS += mod/src/mod.c

voxy/voxy: CFLAGS += -Ivoxy/bundled/include -Ivoxy/include -Ivoxy/src
voxy/voxy: LIBS   += -lm

voxy/voxy: CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)
voxy/voxy: LIBS   += $(shell pkg-config --libs   glfw3 fontconfig freetype2)

mod/mod.so: CFLAGS  += -Ivoxy/include
mod/mod.so: CFLAGS  += -fPIC
mod/mod.so: LDFLAGS += -shared

.PHONY: clean

all: voxy/voxy mod/mod.so

voxy/voxy: $(VOXY_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(VOXY_SRCS:.c=.o) $(LIBS)

mod/mod.so: $(MOD_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(MOD_SRCS:.c=.o) $(LIBS)

clean:
	- rm -f voxy/voxy
	- rm -f mod/mod.so
	- rm -f $(VOXY_SRCS:.c=.o)
	- rm -f $(VOXY_SRCS:.c=.d)
	- rm -f $(MOD_SRCS:.c=.o)
	- rm -f $(MOD_SRCS:.c=.d)

-include $(VOXY_SRCS:.c=.d)
-include $(MOD_SRCS:.c=.d)
