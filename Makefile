CFLAGS+=-MMD
CFLAGS+=-Iinclude
CFLAGS+=-Wall -Wextra
LIBS+=-lm

VOXY_CFLAGS += $(shell pkg-config --cflags glfw3)
VOXY_LIBS   += $(shell pkg-config --libs   glfw3)

VOXY_SRCS += src/camera.c
VOXY_SRCS += src/cube_map.c
VOXY_SRCS += src/glad.c
VOXY_SRCS += src/renderer.c
VOXY_SRCS += src/shader.c
VOXY_SRCS += src/stb_image.c
VOXY_SRCS += src/transform.c
VOXY_SRCS += src/voxy.c
VOXY_SRCS += src/window.c
VOXY_SRCS += src/world.c

CUBE_GEN_SRCS += src/cube_gen.c

.PHONY: clean gen

all: voxy

voxy: $(VOXY_SRCS:.c=.o)
	$(CC) -o $@ $(CFLAGS) $(VOXY_CFLAGS) $(LIBS) $(VOXY_LIBS) $(VOXY_SRCS:.c=.o)

gen: include/voxy/cube.h

include/voxy/cube.h: cube_gen
	./cube_gen > include/voxy/cube.h

cube_gen: $(CUBE_GEN_SRCS:.c=.o)
	$(CC) -o $@ $(CFLAGS) $(CUBE_GEN_CFLAGS) $(LIBS) $(CUBE_GEN_LIBS) $(CUBE_GEN_SRCS:.c=.o)

clean:
	- rm voxy
	- rm cube_gen
	- rm include/voxy/cube.h
	- rm $(VOXY_SRCS:.c=.o)
	- rm $(VOXY_SRCS:.c=.d)
	- rm $(CUBE_GEN_SRCS:.c=.o)
	- rm $(CUBE_GEN_SRCS:.c=.d)

-include $(VOXY_SRCS:.c=.d)
-include $(CUBE_GEN_SRCS:.c=.d)
