SUBDIRS = bin lib

all clean depclean: $(SUBDIRS)
bin: lib

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
