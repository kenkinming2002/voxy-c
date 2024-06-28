CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

VOXY_SRCS += voxy/bundled/src/glad.c
VOXY_SRCS += voxy/bundled/src/stb_image.c

VOXY_SRCS += voxy/src/core/window.c
VOXY_SRCS += voxy/src/core/thread_pool.c
VOXY_SRCS += voxy/src/core/delta_time.c

VOXY_SRCS += voxy/src/graphics/gl.c
VOXY_SRCS += voxy/src/graphics/font_set.c
VOXY_SRCS += voxy/src/graphics/ui.c
VOXY_SRCS += voxy/src/graphics/camera.c

VOXY_SRCS += voxy/src/main_game/assets.c
VOXY_SRCS += voxy/src/main_game/chunk.c
VOXY_SRCS += voxy/src/main_game/chunk_generate.c
VOXY_SRCS += voxy/src/main_game/chunk_hash_table.c
VOXY_SRCS += voxy/src/main_game/chunk_remesh.c
VOXY_SRCS += voxy/src/main_game/entity.c
VOXY_SRCS += voxy/src/main_game/generate.c
VOXY_SRCS += voxy/src/main_game/light.c
VOXY_SRCS += voxy/src/main_game/main_game.c
VOXY_SRCS += voxy/src/main_game/mod.c
VOXY_SRCS += voxy/src/main_game/physics.c
VOXY_SRCS += voxy/src/main_game/registry.c
VOXY_SRCS += voxy/src/main_game/world.c
VOXY_SRCS += voxy/src/main_game/world_camera.c
VOXY_SRCS += voxy/src/main_game/world_render.c
VOXY_SRCS += voxy/src/main_game/world_seed.c

VOXY_SRCS += voxy/src/main_game/entity/player.c
VOXY_SRCS += voxy/src/main_game/entity/weird.c

VOXY_SRCS += voxy/src/voxy.c

MOD_SRCS += mod/src/init.c
MOD_SRCS += mod/src/ids.c
MOD_SRCS += mod/src/generate.c

MOD_SRCS += mod/src/on_use_block_item.c

voxy/voxy: CFLAGS += -Ivoxy/bundled/include -Ivoxy/include -Ivoxy/src -fno-semantic-interposition
voxy/voxy: LIBS   += -lm
voxy/voxy: LDFLAGS += -rdynamic

voxy/voxy: CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)
voxy/voxy: LIBS   += $(shell pkg-config --libs   glfw3 fontconfig freetype2)

mod/mod.so: CFLAGS += -Ivoxy/bundled/include -Ivoxy/include -Ivoxy/src
mod/mod.so: CFLAGS  += -fPIC
mod/mod.so: LDFLAGS += -shared

.PHONY: clean depclean

all: voxy/voxy mod/mod.so

voxy/voxy: $(VOXY_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(VOXY_SRCS:.c=.o) $(LIBS)

mod/mod.so: $(MOD_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(MOD_SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f voxy/voxy
	- rm -f mod/mod.so
	- rm -f $(VOXY_SRCS:.c=.o)
	- rm -f $(MOD_SRCS:.c=.o)

depclean:
	- rm -f $(VOXY_SRCS:.c=.d)
	- rm -f $(MOD_SRCS:.c=.d)

-include $(VOXY_SRCS:.c=.d)
-include $(MOD_SRCS:.c=.d)
