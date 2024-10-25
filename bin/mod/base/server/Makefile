CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Isrc
CFLAGS += -I../../../voxy/config/include
CFLAGS += -I../../../voxy/server/include
CFLAGS += -I../../../../lib/net/include
CFLAGS += -I../../../../lib/serde/include
CFLAGS += -I../../../../lib/common/include
CFLAGS += -I../../../../lib/common/bundled/include

CFLAGS += -fPIC

SRCS += src/mod.c

all: base-server.so

base-server.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
