CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Iinclude
CFLAGS += -Isrc
CFLAGS += -I../config/include
CFLAGS += -I../protocol/include
CFLAGS += -I../../../lib/net/include
CFLAGS += -I../../../lib/common/include
CFLAGS += -I../../../lib/common/bundled/include

LIBS += -L../../../lib/net
LIBS += -lnet-client
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/net'

LIBS += -L../../../lib/common
LIBS += -lcommon
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/common'

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

SRCS += src/mod/mod.c
SRCS += src/mod/manager.c

.PHONY: gen

all: voxy-client

voxy-client: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

gen:
	./gen

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

