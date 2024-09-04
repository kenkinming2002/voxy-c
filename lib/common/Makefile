CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE

CFLAGS += -Iinclude
CFLAGS += -Ibundled/include

CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)
LIBS   += $(shell pkg-config --libs   glfw3 fontconfig freetype2)

CFLAGS += -fPIC

SRCS += bundled/src/glad.c
SRCS += bundled/src/stb_image.c

SRCS += src/math/direction.c

SRCS += src/core/window.c
SRCS += src/core/thread_pool.c
SRCS += src/core/delta_time.c
SRCS += src/core/fs.c
SRCS += src/core/time.c
SRCS += src/core/serde.c

SRCS += src/graphics/gl.c
SRCS += src/graphics/font_set.c
SRCS += src/graphics/mesh.c
SRCS += src/graphics/camera.c
SRCS += src/graphics/render.c

SRCS += src/ui/ui.c

.PHONY: clean depclean

all: libcommon.so

libcommon.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f libcommon.so
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
