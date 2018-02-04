.PHONY: all clean
.SUFFIXES: .html .1

NATIVE ?= -mtune=generic
CFLAGS ?= -std=c99 -g -O2 $(NATIVE) -Wall -Wextra -Wpedantic -DNDEBUG -DNTRACE
LDLIBS  = -lm

all: beta hist
clean:
	rm -f beta hist

.1.html:
	groff -man -T html -P -l -P -r $< | \
	  awk '{ print } /<style/ { print "       body    { font-family: monospace }" }' >$@
