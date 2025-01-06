CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -I../../../lib/core/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/core'
LIBS += -L../../../lib/core
LIBS += -lcore

CFLAGS += -I../../../lib/math/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/math'
LIBS += -L../../../lib/math
LIBS += -lmath

CFLAGS += -I../../../lib/gfx/include
LDFLAGS += -Wl,--rpath,'$$ORIGIN/../../../lib/gfx'
LIBS += -L../../../lib/gfx
LIBS += -lgfx

CFLAGS += $(shell pkg-config --cflags libpng)
LIBS += $(shell pkg-config --libs libpng)

LIBS += -lm

SRCS += src/render_block.c

.PHONY: clean depclean

all: render_block

render_block: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
