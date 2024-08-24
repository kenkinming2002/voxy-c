.PHONY: common net
.PHONY: clean
.PHONY: depclean

all: common net

common:
	$(MAKE) -C common

net:
	$(MAKE) -C net

clean:
	$(MAKE) -C common clean
	$(MAKE) -C net clean

depclean:
	$(MAKE) -C common depclean
	$(MAKE) -C net depclean

