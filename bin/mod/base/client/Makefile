CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Isrc
CFLAGS += -I../../../voxy/client/include
CFLAGS += -I../../../../lib/common/include
CFLAGS += -I../../../../lib/common/bundled/include

CFLAGS += -fPIC

SRCS += src/mod.c

all: base-client.so

base-client.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
