CFLAGS+=-MMD
CFLAGS+=-Iinclude
CFLAGS+=-Wall -Wextra
LIBS+=-lm

CFLAGS += $(shell pkg-config --cflags glfw3)
LIBS   += $(shell pkg-config --libs   glfw3)

SRCS += src/camera.c
SRCS += src/gl.c
SRCS += src/glad.c
SRCS += src/noise.c
SRCS += src/random.c
SRCS += src/renderer.c
SRCS += src/stb_image.c
SRCS += src/transform.c
SRCS += src/voxy.c
SRCS += src/window.c
SRCS += src/world.c

.PHONY: clean

all: voxy

voxy: $(SRCS:.c=.o)
	$(CC) -o $@ $(CFLAGS) $(LIBS) $(SRCS:.c=.o)

clean:
	- rm -f voxy
	- rm -f $(SRCS:.c=.o)
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
