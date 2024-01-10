CFLAGS+=-MMD
CFLAGS+=-Iinclude

CFLAGS += $(shell pkg-config --cflags glfw3)
LIBS   += $(shell pkg-config --libs   glfw3)

all: voxy
voxy: src/voxy.o src/glad.o
	$(CC) -o $@ src/voxy.o src/glad.o $(CFLAGS) $(LIBS)

-include src/voxy.d
-include src/glad.d
