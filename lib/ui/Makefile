CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC
CFLAGS += -Iinclude

CFLAGS += $(shell pkg-config --cflags fontconfig)

CFLAGS += -I ../external/include
CFLAGS += -I ../core/include
CFLAGS += -I ../math/include
CFLAGS += -I ../gfx/include

SRCS += src/ui.c

.PHONY: clean depclean

all: libui.so

libui.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f libui.so
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
