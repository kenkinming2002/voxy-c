CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -I../../../lib/common/bundled/include
CFLAGS += -I../../../lib/common/include
CFLAGS += -Iinclude
CFLAGS += $(shell pkg-config --cflags glfw3 libpng)

LIBS += -lm
LIBS += $(shell pkg-config --libs glfw3 libpng)

SRCS += src/render_block.c

.PHONY: clean depclean

all: render_block

render_block: $(SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS) -L../../../lib/common -lcommon

clean: depclean
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
