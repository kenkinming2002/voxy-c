.PHONY: clean
.PHONY: depclean

all:
	$(MAKE) -C lib
	$(MAKE) -C bin

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C bin clean


depclean:
	$(MAKE) -C lib depclean
	$(MAKE) -C bin depclean
