W32CC  ?= i686-w64-mingw32-gcc
W64CC  ?= x86_64-w64-mingw32-gcc

include Makefile
%32.exe: %.c
	$(W32CC) -m32 $(CPPFLAGS) $(CFLAGS) -o $@ $<
%64.exe: %.c
	$(W64CC) -m64 $(CPPFLAGS) $(CFLAGS) -o $@ $<

all: beta32.exe beta64.exe hist32.exe hist64.exe
clean: cleanw

cleanw:
	rm -f beta32.exe beta64.exe hist32.exe hist64.exe
