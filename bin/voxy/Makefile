CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fopenmp

CFLAGS += -I../../lib/common/bundled/include
CFLAGS += -I../../lib/common/include
CFLAGS += -Iinclude
CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)

LIBS += -lm
LIBS += $(shell pkg-config --libs glfw3 fontconfig freetype2)

LDFLAGS += -rdynamic

SRCS += src/voxy.c

SRCS += src/scene/scene.c

SRCS += src/scene/main_menu/main_menu.c

SRCS += src/scene/main_game/main_game.c
SRCS += src/scene/main_game/mod.c

SRCS += src/scene/main_game/types/chunk.c
SRCS += src/scene/main_game/types/registry.c

SRCS += src/scene/main_game/types/block_data.c
SRCS += src/scene/main_game/types/block_datas.c

SRCS += src/scene/main_game/types/entity.c
SRCS += src/scene/main_game/types/entities.c

SRCS += src/scene/main_game/types/item.c
SRCS += src/scene/main_game/types/container.c

SRCS += src/scene/main_game/states/camera.c
SRCS += src/scene/main_game/states/chunks.c
SRCS += src/scene/main_game/states/digger.c
SRCS += src/scene/main_game/states/seed.c
SRCS += src/scene/main_game/states/cursor.c

SRCS += src/scene/main_game/states/entity_query.c
SRCS += src/scene/main_game/states/entity_query_info.c

SRCS += src/scene/main_game/update/chunk_database.c
SRCS += src/scene/main_game/update/chunk_generate.c
SRCS += src/scene/main_game/update/chunk_manager.c
SRCS += src/scene/main_game/update/generate.c
SRCS += src/scene/main_game/update/light.c

SRCS += src/scene/main_game/physics/physics.c
SRCS += src/scene/main_game/physics/swept.c

SRCS += src/scene/main_game/render/assets.c
SRCS += src/scene/main_game/render/render.c

SRCS += src/scene/main_game/render/blocks.c
SRCS += src/scene/main_game/render/blocks_render_info.c
SRCS += src/scene/main_game/render/blocks_mesh.c

SRCS += src/scene/main_game/render/entities.c

SRCS += src/scene/main_game/render/debug.c
SRCS += src/scene/main_game/render/debug_overlay.c
SRCS += src/scene/main_game/render/debug_gizmos.c

SRCS += src/scene/main_game/crafting/crafting.c
SRCS += src/scene/main_game/crafting/recipe.c

.PHONY: clean depclean
.PHONY: server client

all: voxy server client

server:
	$(MAKE) -C server

client:
	$(MAKE) -C client

voxy: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS) -L../../lib/common -Wl,--whole-archive -lcommon -Wl,--no-whole-archive

clean: depclean
	- rm -f $(SRCS:.c=.o)
	$(MAKE) -C server clean
	$(MAKE) -C client clean

depclean:
	- rm -f $(SRCS:.c=.d)
	$(MAKE) -C server depclean
	$(MAKE) -C client depclean

-include $(SRCS:.c=.d)

