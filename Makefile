CFLAGS += -MMD
CFLAGS += -Wall -Wextra

VOXY_SRCS += voxy/src/camera.c
VOXY_SRCS += voxy/src/font_set.c
VOXY_SRCS += voxy/src/gl.c
VOXY_SRCS += voxy/src/glad.c
VOXY_SRCS += voxy/src/noise.c
VOXY_SRCS += voxy/src/random.c
VOXY_SRCS += voxy/src/resource_pack.c
VOXY_SRCS += voxy/src/stb_image.c
VOXY_SRCS += voxy/src/thread_pool.c
VOXY_SRCS += voxy/src/transform.c
VOXY_SRCS += voxy/src/ui.c
VOXY_SRCS += voxy/src/voxy.c
VOXY_SRCS += voxy/src/window.c
VOXY_SRCS += voxy/src/world.c
VOXY_SRCS += voxy/src/world_generate.c
VOXY_SRCS += voxy/src/world_generator.c
VOXY_SRCS += voxy/src/world_light.c
VOXY_SRCS += voxy/src/world_player_control.c
VOXY_SRCS += voxy/src/world_renderer.c

RESOURCE_PACK_SRCS += resource_pack/src/resource_pack.c

voxy/voxy: CFLAGS += -Ivoxy/include
voxy/voxy: LIBS   += -lm

voxy/voxy: CFLAGS += $(shell pkg-config --cflags glfw3 fontconfig freetype2)
voxy/voxy: LIBS   += $(shell pkg-config --libs   glfw3 fontconfig freetype2)

resource_pack/resource_pack.so: CFLAGS  += -Ivoxy/include
resource_pack/resource_pack.so: CFLAGS  += -fPIC
resource_pack/resource_pack.so: LDFLAGS += -shared

.PHONY: clean

all: voxy/voxy resource_pack/resource_pack.so

voxy/voxy: $(VOXY_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(VOXY_SRCS:.c=.o) $(LIBS)

resource_pack/resource_pack.so: $(RESOURCE_PACK_SRCS:.c=.o)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $(RESOURCE_PACK_SRCS:.c=.o) $(LIBS)

clean:
	- rm -f voxy/voxy
	- rm -f resource_pack/resource_pack.so
	- rm -f $(VOXY_SRCS:.c=.o)
	- rm -f $(VOXY_SRCS:.c=.d)
	- rm -f $(RESOURCE_PACK_SRCS:.c=.o)
	- rm -f $(RESOURCE_PACK_SRCS:.c=.d)

-include $(VOXY_SRCS:.c=.d)
-include $(RESOURCE_PACK_SRCS:.c=.d)
