SUBDIRS = external core math gfx ui net serde

all clean depclean: $(SUBDIRS)

export CFLAGS

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

