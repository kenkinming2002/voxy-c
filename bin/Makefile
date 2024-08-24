.PHONY: voxy mod tools/render_block
.PHONY: clean
.PHONY: depclean

all: voxy mod tools/render_block

voxy:
	$(MAKE) -C voxy

mod:
	$(MAKE) -C mod

tools/render_block:
	$(MAKE) -C tools/render_block

clean:
	$(MAKE) -C voxy clean
	$(MAKE) -C mod clean
	$(MAKE) -C tools/render_block clean

depclean:
	$(MAKE) -C voxy depclean
	$(MAKE) -C mod depclean
	$(MAKE) -C tools/render_block depclean
