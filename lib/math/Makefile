CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC
CFLAGS += -Iinclude

SRCS += src/direction.c

.PHONY: clean depclean

all: libmath.so

libmath.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f libmath.so
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
