SUBDIRS = server client

all clean depclean: $(SUBDIRS)

export CFLAGS

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

