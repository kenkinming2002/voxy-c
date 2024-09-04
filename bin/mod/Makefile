CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -I../../lib/common/bundled/include
CFLAGS += -I../../lib/common/include
CFLAGS += -I../voxy/include
CFLAGS += -Isrc
CFLAGS += -fPIC

LDFLAGS += -shared

SRCS += src/mod.c
SRCS += src/generate.c

SRCS += src/block/block.c
SRCS += src/block/empty/empty.c
SRCS += src/block/ether/ether.c
SRCS += src/block/grass/grass.c
SRCS += src/block/lamp/lamp.c
SRCS += src/block/leave/leave.c
SRCS += src/block/log/log.c
SRCS += src/block/ore_coal/ore_coal.c
SRCS += src/block/ore_copper/ore_copper.c
SRCS += src/block/ore_iron/ore_iron.c
SRCS += src/block/ore_tin/ore_tin.c
SRCS += src/block/stone/stone.c
SRCS += src/block/water/water.c
SRCS += src/block/plank/plank.c
SRCS += src/block/chest/chest.c

SRCS += src/item/item.c
SRCS += src/item/grass/grass.c
SRCS += src/item/lamp/lamp.c
SRCS += src/item/leave/leave.c
SRCS += src/item/log/log.c
SRCS += src/item/ore_coal_block/ore_coal_block.c
SRCS += src/item/ore_copper_block/ore_copper_block.c
SRCS += src/item/ore_iron_block/ore_iron_block.c
SRCS += src/item/ore_tin_block/ore_tin_block.c
SRCS += src/item/stone/stone.c
SRCS += src/item/mysterious_food/mysterious_food.c
SRCS += src/item/dynamite/dynamite.c
SRCS += src/item/plank/plank.c
SRCS += src/item/chest/chest.c

SRCS += src/entity/item/item.c
SRCS += src/entity/dynamite/dynamite.c

SRCS += src/entity/player/ui/layout.c
SRCS += src/entity/player/actions.c
SRCS += src/entity/player/camera_follow.c
SRCS += src/entity/player/chunk_loader.c
SRCS += src/entity/player/controls.c
SRCS += src/entity/player/inventory.c
SRCS += src/entity/player/player.c
SRCS += src/entity/player/health_ui.c

SRCS += src/entity/weird/weird.c

SRCS += src/update/spawn_player.c
SRCS += src/update/spawn_weird.c

.PHONY: base clean depclean

all: base mod.so

base:
	$(MAKE) -C base

mod.so: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	$(MAKE) -C base clean
	- rm -f $(SRCS:.c=.o)

depclean:
	$(MAKE) -C base depclean
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

