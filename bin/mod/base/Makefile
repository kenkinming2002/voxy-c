.PHONY: server client depclean clean

all: server client

server:
	$(MAKE) -C server

client:
	$(MAKE) -C client

depclean:
	$(MAKE) -C server depclean
	$(MAKE) -C client depclean

clean:
	$(MAKE) -C server clean
	$(MAKE) -C client clean
