#
#  Makefile
#
#  Copyright (c) 2021 by Daniel Kelley
#

DEBUG ?= -g

PREFIX ?= /usr/local

# address thread undefined etc.
ifneq ($(SANITIZE),)
DEBUG += -fsanitize=$(SANITIZE)
endif

INC :=
CPPFLAGS := $(INC) -MP -MMD

LDLIBS := -lm

WARN := -Wall
WARN += -Wextra
WARN += -Wdeclaration-after-statement
WARN += -Werror
CFLAGS := $(WARN) $(DEBUG)

PROG := xrso12832
SRC := $(PROG).c
SRC += xoshiro128plus.c
OBJ := $(SRC:%.c=%.o)
DEP := $(SRC:%.c=%.d)

VG ?= valgrind --leak-check=full

.PHONY: all install uninstall clean

all: $(PROG)

$(PROG): $(OBJ)

check: $(PROG)
	./check.sh $(CHECK_ARGS)

clean:
	-rm -f $(PROG) $(OBJ) $(DEP) FIFO

-include $(DEP)
