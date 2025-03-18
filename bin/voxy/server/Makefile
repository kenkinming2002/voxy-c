CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fopenmp
CFLAGS += -Iinclude
CFLAGS += -Isrc
CFLAGS += -I../config/include
CFLAGS += -I../protocol/include

CFLAGS += -I../../../lib/core/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/core'
LIBS += -L../../../lib/core
LIBS += -lcore

CFLAGS += -I../../../lib/math/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/math'
LIBS += -L../../../lib/math
LIBS += -lmath

CFLAGS += -I../../../lib/serde/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/serde'
LIBS += -L../../../lib/serde
LIBS += -lserde

CFLAGS += -I../../../lib/net/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/net'
LIBS += -L../../../lib/net
LIBS += -lnet-server

CFLAGS += $(shell pkg-config --cflags sqlite3 liburing)
LIBS   += $(shell pkg-config --libs   sqlite3 liburing)
LIBS += -lm

CFLAGS += -fvisibility=hidden
LDFLAGS += -rdynamic

SRCS += src/main.c
SRCS += src/application.c

SRCS += src/sqlite3_utils.c

SRCS += src/hash_table/ivec3.c

SRCS += src/registry/block.c
SRCS += src/registry/entity.c
SRCS += src/registry/item.c

SRCS += src/registry/entity_info.c

SRCS += src/chunk/manager.c
SRCS += src/chunk/coordinates.c

SRCS += src/chunk/block/group.c
SRCS += src/chunk/block/manager.c
SRCS += src/chunk/block/database.c
SRCS += src/chunk/block/network.c
SRCS += src/chunk/block/generator.c

SRCS += src/chunk/entity/entity.c
SRCS += src/chunk/entity/allocator.c
SRCS += src/chunk/entity/network.c
SRCS += src/chunk/entity/database.c
SRCS += src/chunk/entity/manager.c

SRCS += src/player/player.c
SRCS += src/player/manager.c

SRCS += src/light/manager.c

SRCS += src/mod/mod.c
SRCS += src/mod/manager.c

SRCS += src/physics/physics.c
SRCS += src/physics/swept.c

.PHONY: all
all: voxy-server

voxy-server: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

.PHONY: clean
clean: depclean
	- rm -f $(GENERATED)
	- rm -f $(SRCS:.c=.o)

.PHONY: depclean
depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

define GENERATED_template =

include/voxy/server/registry/$(1).h:include/voxy/server/registry/registry.h
	< $$< sed s/name/$(1)/g | sed s/NAME/$(2)/g > $$@

src/registry/$(1).h:src/registry/registry.h
	< $$< sed s/name/$(1)/g | sed s/NAME/$(2)/g > $$@

src/registry/$(1).c:src/registry/registry.c
	< $$< sed s/name/$(1)/g | sed s/NAME/$(2)/g > $$@

GENERATED += include/voxy/server/registry/$(1).h
GENERATED += src/registry/$(1).h
GENERATED += src/registry/$(1).c

endef

$(foreach prog,entity block item,$(eval $(call GENERATED_template,$(prog),$(shell echo $(prog) |tr [a-z] [A-Z]))))
$(SRCS:.c=.o): | $(GENERATED)
