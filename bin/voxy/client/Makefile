CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Isrc
CFLAGS += -I../protocol/include
CFLAGS += -I../../../lib/net/include
CFLAGS += -I../../../lib/common/include
CFLAGS += -I../../../lib/common/bundled/include

LIBS += -L../../../lib/net
LIBS += -lnet-client

LIBS += -L../../../lib/common
LIBS += -lcommon

LIBS += -lm

CFLAGS += $(shell pkg-config --cflags glfw3)
LIBS   += $(shell pkg-config --libs   glfw3)

SRCS += src/main.c
SRCS += src/application.c

SRCS += src/block/registry.c

SRCS += src/input/manager.c
SRCS += src/camera/manager.c

SRCS += src/chunk/chunk.c
SRCS += src/chunk/manager.c

SRCS += src/entity/registry.c
SRCS += src/entity/manager.c

SRCS += src/render/world.c
SRCS += src/render/block/block.c
SRCS += src/render/block/mesh.c
SRCS += src/render/block/render_info.c

all: voxy-client

voxy-client: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

