CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
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

CFLAGS += -I../../../lib/gfx/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/gfx'
LIBS += -L../../../lib/gfx
LIBS += -lgfx

CFLAGS += -I../../../lib/ui/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/ui'
LIBS += -L../../../lib/ui
LIBS += -lui

CFLAGS += -I../../../lib/net/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/net'
LIBS += -L../../../lib/net
LIBS += -lnet-client

LIBS += -lm

CFLAGS += -fvisibility=hidden
LDFLAGS += -rdynamic

SRCS += src/main.c
SRCS += src/application.c

SRCS += src/registry/block.c
SRCS += src/registry/entity.c
SRCS += src/registry/item.c

SRCS += src/input/manager.c
SRCS += src/camera/manager.c

SRCS += src/chunk/chunk.c
SRCS += src/chunk/manager.c

SRCS += src/entity/entity.c
SRCS += src/entity/manager.c

SRCS += src/render/world.c
SRCS += src/render/block/block.c
SRCS += src/render/block/mesh.c
SRCS += src/render/block/render_info.c
SRCS += src/render/entity/entity.c

SRCS += src/ui/manager.c

SRCS += src/mod/mod.c
SRCS += src/mod/manager.c

.PHONY: all
all: voxy-client

voxy-client: $(SRCS:.c=.o)
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

include/voxy/client/registry/$(1).h:include/voxy/client/registry/registry.h
	< $$< sed s/name/$(1)/g | sed s/NAME/$(2)/g > $$@

src/registry/$(1).h:src/registry/registry.h
	< $$< sed s/name/$(1)/g | sed s/NAME/$(2)/g > $$@

src/registry/$(1).c:src/registry/registry.c
	< $$< sed s/name/$(1)/g | sed s/NAME/$(2)/g > $$@

GENERATED += include/voxy/client/registry/$(1).h
GENERATED += src/registry/$(1).h
GENERATED += src/registry/$(1).c

endef

$(foreach prog,entity block item,$(eval $(call GENERATED_template,$(prog),$(shell echo $(prog) |tr [a-z] [A-Z]))))
$(SRCS:.c=.o): | $(GENERATED)
