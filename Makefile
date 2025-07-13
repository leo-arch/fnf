CPPFLAGS=-D_GNU_SOURCE
CFLAGS+=-MD -Wall -Wextra -std=c99 -O3 -pedantic -Ideps -Werror=vla
PREFIX?=/usr/local
MANDIR?=$(PREFIX)/share/man
BINDIR?=$(PREFIX)/bin
DEBUGGER?=

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=${INSTALL} -m 644

LIBS=-lpthread
OBJECTS=src/fnf.o src/match.o src/tty.o src/choices.o src/options.o src/tty_interface.o src/colors.o
THEFTDEPS = deps/theft/theft.o deps/theft/theft_bloom.o deps/theft/theft_mt.o deps/theft/theft_hash.o
TESTOBJECTS=test/fnftest.c test/test_properties.c test/test_choices.c test/test_match.c src/match.o src/choices.o src/options.o $(THEFTDEPS)

all: fnf

test/fnftest: $(TESTOBJECTS)
	$(CC) $(CFLAGS) $(CCFLAGS) -Isrc -o $@ $(TESTOBJECTS) $(LIBS)

acceptance: fnf
	cd test/acceptance && bundle --quiet && bundle exec ruby acceptance_test.rb

test: check
check: test/fnftest
	$(DEBUGGER) ./test/fnftest

fnf: $(OBJECTS)
	$(CC) $(CFLAGS) $(CCFLAGS) -o $@ $(OBJECTS) $(LIBS)

install: fnf
	mkdir -p $(DESTDIR)$(BINDIR)
	cp fnf $(DESTDIR)$(BINDIR)/
	chmod 755 ${DESTDIR}${BINDIR}/fnf
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	cp fnf.1 $(DESTDIR)$(MANDIR)/man1/
	chmod 644 ${DESTDIR}${MANDIR}/man1/fnf.1

uninstall:
	rm -- $(DESTDIR)$(BINDIR)/fnf
	rm -- $(DESTDIR)$(MANDIR)/man1/fnf.1

fmt:
	clang-format -i src/*.c src/*.h

clean:
	rm -f fnf test/fnftest src/*.o src/*.d deps/*/*.o

.PHONY: test check all clean install fmt acceptance

-include $(OBJECTS:.o=.d)
