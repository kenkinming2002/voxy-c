SUBDIRS = tools voxy mod

all clean depclean: $(SUBDIRS)
mod: voxy
voxy: tools

export CFLAGS

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
