CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC
CFLAGS += -Iinclude

SRCS += src/profile.c
SRCS += src/thread_pool.c
SRCS += src/fs.c

.PHONY: clean depclean

all: libcore.so

libcore.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f libcore.so
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
