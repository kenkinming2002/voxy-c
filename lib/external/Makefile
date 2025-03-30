CFLAGS += -MMD
CFLAGS += -Wall -Wextra
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC
CFLAGS += -Iinclude

SRCS += src/stb_ds.c

.PHONY: clean depclean

all: libexternal.a

libexternal.a: $(SRCS:.c=.o)
	$(AR) rcs $@ $(SRCS:.c=.o)

clean: depclean
	- rm -f libexternal.a
	- rm -f $(SRCS:.c=.o)

depclean:
	- rm -f $(SRCS:.c=.d)

-include $(SRCS:.c=.d)
