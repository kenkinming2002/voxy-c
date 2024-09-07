CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Iinclude
CFLAGS += -Isrc
CFLAGS += -I../protocol/include
CFLAGS += -I../../../lib/net/include
CFLAGS += -I../../../lib/common/include
CFLAGS += -I../../../lib/common/bundled/include

LIBS += -L../../../lib/net
LIBS += -lnet-server
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/net'

LIBS += -L../../../lib/common
LIBS += -lcommon
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/common'

LIBS += -lm

CFLAGS += -fvisibility=hidden
LDFLAGS += -rdynamic

SRCS += src/main.c
SRCS += src/application.c

SRCS += src/block/registry.c
SRCS += src/entity/registry.c

SRCS += src/chunk/chunk.c
SRCS += src/chunk/coordinates.c
SRCS += src/chunk/manager.c
SRCS += src/chunk/generator.c

SRCS += src/entity/entity.c
SRCS += src/entity/manager.c

SRCS += src/player/player.c
SRCS += src/player/manager.c

SRCS += src/light/manager.c

SRCS += src/mod/mod.c
SRCS += src/mod/manager.c

SRCS += src/physics/physics.c
SRCS += src/physics/swept.c

all: voxy-server

voxy-server: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

