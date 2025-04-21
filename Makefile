SUBDIRS = bin lib

CFLAGS += -I $(shell realpath abomination)

all clean depclean: $(SUBDIRS)
bin: lib

export CFLAGS

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
