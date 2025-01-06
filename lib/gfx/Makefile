CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC
CFLAGS += -Iinclude

CFLAGS += -I../core/include
CFLAGS += -I../math/include

CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)
LIBS   += $(shell pkg-config --libs   glfw3 fontconfig freetype2)

SRCS += src/camera.c
SRCS += src/delta_time.c
SRCS += src/font_set.c
SRCS += src/gl.c
SRCS += src/glad.c
SRCS += src/mesh.c
SRCS += src/render.c
SRCS += src/stb_image.c
SRCS += src/time.c
SRCS += src/window.c

.PHONY: clean depclean

all: libgfx.so

libgfx.so: $(SRCS:.c=.o)
	$(CC) -shared -o $@ $(LDFLAGS) $(CFLAGS) $(SRCS:.c=.o) $(LIBS)

clean: depclean
	- rm -f libgfx.so
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
