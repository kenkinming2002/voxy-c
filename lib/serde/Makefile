CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -Iinclude
CFLAGS += -fPIC

SRCS += src/serializer.c
SRCS += src/deserializer.c

.PHONY: clean depclean

all: libserde.so

libserde.so: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) -shared

voxy-server: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)
	- rm -f libserde.so

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)

