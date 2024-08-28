CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Isrc
CFLAGS += -I../protocol/include
CFLAGS += -I../../../lib/net/include
CFLAGS += -I../../../lib/common/include
CFLAGS += -I../../../lib/common/bundled/include

LIBS += -L../../../lib/net
LIBS += -lnet-server

LIBS += -lm

SRCS += src/main.c
SRCS += src/application.c

SRCS += src/chunk/chunk.c
SRCS += src/chunk/manager.c

all: voxy-server

voxy-server: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

