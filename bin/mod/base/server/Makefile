CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC

CFLAGS += -Isrc
CFLAGS += -I../../../voxy/config/include
CFLAGS += -I../../../voxy/server/include
CFLAGS += -I../../../../lib/core/include
CFLAGS += -I../../../../lib/math/include
CFLAGS += -I../../../../lib/net/include
CFLAGS += -I../../../../lib/serde/include

SRCS += src/mod.c

all: base-server.so

base-server.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
