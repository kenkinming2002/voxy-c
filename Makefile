CFLAGS+=-MMD

all: voxy
voxy: src/voxy.o
	$(CC) $(CFLAGS) -o $@ src/voxy.o

-include src/voxy.d
