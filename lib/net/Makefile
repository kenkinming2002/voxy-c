CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -Iinclude

libnet-server.so: CFLAGS += -fPIC
libnet-client.so: CFLAGS += -fPIC

.PHONY: clean depclean

all: libnet-server.so libnet-client.so test/server test/client

libnet-server.so: src/server.o
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $< -shared

libnet-client.so: src/client.o
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $< -shared

test/server: test/server.o
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $< -Wl,-rpath,./lib/net -L. -lnet-server

test/client: test/client.o
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $< -Wl,-rpath,./lib/net -L. -lnet-client

clean: depclean
	- rm -f src/server.o
	- rm -f src/client.o
	- rm -f libnet-server.so
	- rm -f libnet-client.so
	- rm -f test/server.o
	- rm -f test/client.o
	- rm -f test/server
	- rm -f test/client

depclean:
	- rm -f src/server.d
	- rm -f src/client.d
	- rm -f test/server.d
	- rm -f test/client.d

-include $(SRCS:.c=.d)

