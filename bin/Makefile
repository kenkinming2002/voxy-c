SUBDIRS = tools voxy mod

all clean depclean: $(SUBDIRS)
mod: voxy
voxy: tools

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
