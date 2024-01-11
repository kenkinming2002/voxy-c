CFLAGS+=-MMD
CFLAGS+=-Iinclude
CFLAGS+=-Wall -Wextra
LIBS+=-lm

VOXY_CFLAGS += $(shell pkg-config --cflags glfw3)
VOXY_LIBS   += $(shell pkg-config --libs   glfw3)

VOXY_SRCS += src/camera.c
VOXY_SRCS += src/chunk.c
VOXY_SRCS += src/glad.c
VOXY_SRCS += src/renderer.c
VOXY_SRCS += src/shader.c
VOXY_SRCS += src/transform.c
VOXY_SRCS += src/voxy.c
VOXY_SRCS += src/world.c

CUBE_SRCS += src/cube.c

.PHONY: clean

all: voxy cube

voxy: $(VOXY_SRCS:.c=.o)
	$(CC) -o $@ $(CFLAGS) $(VOXY_CFLAGS) $(LIBS) $(VOXY_LIBS) $(VOXY_SRCS:.c=.o)

cube: $(CUBE_SRCS:.c=.o)
	$(CC) -o $@ $(CFLAGS) $(CUBE_CFLAGS) $(LIBS) $(CUBE_LIBS) $(CUBE_SRCS:.c=.o)

clean:
	rm -f voxy
	rm -f cube
	rm -f $(VOXY_SRCS:.c=.o)
	rm -f $(CUBE_SRCS:.c=.o)

-include $(VOXY_SRCS:.c=.d)
-include $(CUBE_SRCS:.c=.d)
