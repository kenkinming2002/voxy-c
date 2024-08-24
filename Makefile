CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

RENDER_BLOCK_SRCS += render_block/src/render_block.c

VOXY_SRCS += voxy/src/voxy.c

VOXY_SRCS += voxy/src/scene/scene.c

VOXY_SRCS += voxy/src/scene/main_menu/main_menu.c

VOXY_SRCS += voxy/src/scene/main_game/main_game.c
VOXY_SRCS += voxy/src/scene/main_game/mod.c

VOXY_SRCS += voxy/src/scene/main_game/types/chunk.c
VOXY_SRCS += voxy/src/scene/main_game/types/registry.c

VOXY_SRCS += voxy/src/scene/main_game/types/block_data.c
VOXY_SRCS += voxy/src/scene/main_game/types/block_datas.c

VOXY_SRCS += voxy/src/scene/main_game/types/entity.c
VOXY_SRCS += voxy/src/scene/main_game/types/entities.c

VOXY_SRCS += voxy/src/scene/main_game/types/item.c
VOXY_SRCS += voxy/src/scene/main_game/types/container.c

VOXY_SRCS += voxy/src/scene/main_game/states/camera.c
VOXY_SRCS += voxy/src/scene/main_game/states/chunks.c
VOXY_SRCS += voxy/src/scene/main_game/states/digger.c
VOXY_SRCS += voxy/src/scene/main_game/states/seed.c
VOXY_SRCS += voxy/src/scene/main_game/states/cursor.c

VOXY_SRCS += voxy/src/scene/main_game/states/entity_query.c
VOXY_SRCS += voxy/src/scene/main_game/states/entity_query_info.c

VOXY_SRCS += voxy/src/scene/main_game/update/chunk_database.c
VOXY_SRCS += voxy/src/scene/main_game/update/chunk_generate.c
VOXY_SRCS += voxy/src/scene/main_game/update/chunk_manager.c
VOXY_SRCS += voxy/src/scene/main_game/update/generate.c
VOXY_SRCS += voxy/src/scene/main_game/update/light.c

VOXY_SRCS += voxy/src/scene/main_game/physics/physics.c
VOXY_SRCS += voxy/src/scene/main_game/physics/swept.c

VOXY_SRCS += voxy/src/scene/main_game/render/assets.c
VOXY_SRCS += voxy/src/scene/main_game/render/render.c

VOXY_SRCS += voxy/src/scene/main_game/render/blocks.c
VOXY_SRCS += voxy/src/scene/main_game/render/blocks_render_info.c
VOXY_SRCS += voxy/src/scene/main_game/render/blocks_mesh.c

VOXY_SRCS += voxy/src/scene/main_game/render/entities.c

VOXY_SRCS += voxy/src/scene/main_game/render/debug.c
VOXY_SRCS += voxy/src/scene/main_game/render/debug_overlay.c
VOXY_SRCS += voxy/src/scene/main_game/render/debug_gizmos.c

VOXY_SRCS += voxy/src/scene/main_game/crafting/crafting.c
VOXY_SRCS += voxy/src/scene/main_game/crafting/recipe.c

MOD_SRCS += mod/src/mod.c
MOD_SRCS += mod/src/generate.c

MOD_SRCS += mod/src/block/block.c
MOD_SRCS += mod/src/block/empty/empty.c
MOD_SRCS += mod/src/block/ether/ether.c
MOD_SRCS += mod/src/block/grass/grass.c
MOD_SRCS += mod/src/block/lamp/lamp.c
MOD_SRCS += mod/src/block/leave/leave.c
MOD_SRCS += mod/src/block/log/log.c
MOD_SRCS += mod/src/block/ore_coal/ore_coal.c
MOD_SRCS += mod/src/block/ore_copper/ore_copper.c
MOD_SRCS += mod/src/block/ore_iron/ore_iron.c
MOD_SRCS += mod/src/block/ore_tin/ore_tin.c
MOD_SRCS += mod/src/block/stone/stone.c
MOD_SRCS += mod/src/block/water/water.c
MOD_SRCS += mod/src/block/plank/plank.c
MOD_SRCS += mod/src/block/chest/chest.c

MOD_SRCS += mod/src/item/item.c
MOD_SRCS += mod/src/item/grass/grass.c
MOD_SRCS += mod/src/item/lamp/lamp.c
MOD_SRCS += mod/src/item/leave/leave.c
MOD_SRCS += mod/src/item/log/log.c
MOD_SRCS += mod/src/item/ore_coal_block/ore_coal_block.c
MOD_SRCS += mod/src/item/ore_copper_block/ore_copper_block.c
MOD_SRCS += mod/src/item/ore_iron_block/ore_iron_block.c
MOD_SRCS += mod/src/item/ore_tin_block/ore_tin_block.c
MOD_SRCS += mod/src/item/stone/stone.c
MOD_SRCS += mod/src/item/mysterious_food/mysterious_food.c
MOD_SRCS += mod/src/item/dynamite/dynamite.c
MOD_SRCS += mod/src/item/plank/plank.c
MOD_SRCS += mod/src/item/chest/chest.c

MOD_SRCS += mod/src/entity/item/item.c
MOD_SRCS += mod/src/entity/dynamite/dynamite.c

MOD_SRCS += mod/src/entity/player/ui/layout.c
MOD_SRCS += mod/src/entity/player/actions.c
MOD_SRCS += mod/src/entity/player/camera_follow.c
MOD_SRCS += mod/src/entity/player/chunk_loader.c
MOD_SRCS += mod/src/entity/player/controls.c
MOD_SRCS += mod/src/entity/player/inventory.c
MOD_SRCS += mod/src/entity/player/player.c
MOD_SRCS += mod/src/entity/player/health_ui.c

MOD_SRCS += mod/src/entity/weird/weird.c

MOD_SRCS += mod/src/update/spawn_player.c
MOD_SRCS += mod/src/update/spawn_weird.c

render_block/render_block: CFLAGS += -Ilib/common/bundled/include
render_block/render_block: CFLAGS += -Ilib/common/include
render_block/render_block: CFLAGS += $(shell pkg-config --cflags glfw3 libpng)
render_block/render_block: LIBS   += $(shell pkg-config --libs   glfw3 libpng)
render_block/render_block: LIBS   += -lm

render_block/render_block: CFLAGS  += -Irender_block/include

voxy/voxy: CFLAGS += -Ilib/common/bundled/include
voxy/voxy: CFLAGS += -Ilib/common/include

voxy/voxy: CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)
voxy/voxy: LIBS   += $(shell pkg-config --libs   glfw3 fontconfig freetype2)
voxy/voxy: LIBS   += -lm

voxy/voxy: CFLAGS  += -Ivoxy/include
voxy/voxy: LDFLAGS += -rdynamic

mod/mod.so: CFLAGS += -Ilib/common/bundled/include
mod/mod.so: CFLAGS += -Ilib/common/include

mod/mod.so: CFLAGS += -Ivoxy/include
mod/mod.so: CFLAGS += -Imod/src

mod/mod.so: CFLAGS  += -fPIC
mod/mod.so: LDFLAGS += -shared

.PHONY: lib/common/libcommon.a clean depclean

all: voxy/voxy render_block/render_block mod/mod.so

lib/common/libcommon.a:
	$(MAKE) -C lib/common

render_block/render_block: $(RENDER_BLOCK_SRCS:.c=.o) lib/common/libcommon.a
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(RENDER_BLOCK_SRCS:.c=.o) $(LIBS) -Llib/common -lcommon

voxy/voxy: $(VOXY_SRCS:.c=.o) lib/common/libcommon.a
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(VOXY_SRCS:.c=.o) $(LIBS) -Llib/common -Wl,--whole-archive -lcommon -Wl,--no-whole-archive

mod/mod.so: $(MOD_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(MOD_SRCS:.c=.o) $(LIBS)

clean: depclean
	$(MAKE) -C lib/common clean
	- rm -f voxy/voxy
	- rm -f mod/mod.so
	- rm -f $(RENDER_BLOCK_SRCS:.c=.o)
	- rm -f $(VOXY_SRCS:.c=.o)
	- rm -f $(MOD_SRCS:.c=.o)

depclean:
	$(MAKE) -C lib/common depclean
	- rm -f $(RENDER_BLOCK_SRCS:.c=.d)
	- rm -f $(VOXY_SRCS:.c=.d)
	- rm -f $(MOD_SRCS:.c=.d)

-include $(RENDER_BLOCK_SRCS:.c=.d)
-include $(VOXY_SRCS:.c=.d)
-include $(MOD_SRCS:.c=.d)
