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

VOXY_SRCS += voxy/src/voxy.c

VOXY_SRCS += voxy/src/scene/scene.c

VOXY_SRCS += voxy/src/scene/main_menu/main_menu.c

VOXY_SRCS += voxy/src/scene/main_game/main_game.c
VOXY_SRCS += voxy/src/scene/main_game/mod.c

VOXY_SRCS += voxy/src/scene/main_game/types/chunk_hash_table.c
VOXY_SRCS += voxy/src/scene/main_game/types/chunk.c
VOXY_SRCS += voxy/src/scene/main_game/types/chunk_data.c
VOXY_SRCS += voxy/src/scene/main_game/types/registry.c
VOXY_SRCS += voxy/src/scene/main_game/types/entity.c

VOXY_SRCS += voxy/src/scene/main_game/entity/weird.c
VOXY_SRCS += voxy/src/scene/main_game/entity/player.c
VOXY_SRCS += voxy/src/scene/main_game/entity/item.c

VOXY_SRCS += voxy/src/scene/main_game/states/camera.c
VOXY_SRCS += voxy/src/scene/main_game/states/chunks.c
VOXY_SRCS += voxy/src/scene/main_game/states/invalidate.c
VOXY_SRCS += voxy/src/scene/main_game/states/seed.c

VOXY_SRCS += voxy/src/scene/main_game/update/chunk_generate.c
VOXY_SRCS += voxy/src/scene/main_game/update/chunk_remesh.c
VOXY_SRCS += voxy/src/scene/main_game/update/generate.c
VOXY_SRCS += voxy/src/scene/main_game/update/light.c
VOXY_SRCS += voxy/src/scene/main_game/update/physics.c

VOXY_SRCS += voxy/src/scene/main_game/render/debug.c
VOXY_SRCS += voxy/src/scene/main_game/render/render.c
VOXY_SRCS += voxy/src/scene/main_game/render/assets.c

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
