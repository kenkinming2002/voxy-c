.PHONY: common net serde
.PHONY: clean
.PHONY: depclean

all: common net serde

common:
	$(MAKE) -C common

net:
	$(MAKE) -C net

serde:
	$(MAKE) -C serde

clean:
	$(MAKE) -C common clean
	$(MAKE) -C net clean
	$(MAKE) -C serde clean

depclean:
	$(MAKE) -C common depclean
	$(MAKE) -C net depclean
	$(MAKE) -C serde depclean

