# pdwm - shared library configuration window manager for phyOS
# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c pdwm.c util.c
OBJ = ${SRC:.c=.o}
OBJ_PDWM = pdwmc/*
SRC_BLOCKS = dwmblocks.c
OBJ_BLOCKS = ${SRC_BLOCKS:.c=.o}

all: options pdwm dwmblocks dwm-conf

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

%.o: %.c
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

${OBJ_BLOCKS}: dwmblocks.h

pdwm: dwm-conf ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

dwm-conf: ${OBJ_PDWM}
	${CC} -c pdwmc/$@.c -fPIC
	${CC} -shared $@.o -o libdwm-conf.so

dwmblocks: ${OBJ_BLOCKS}
	${CC} -o $@ ${OBJ_BLOCKS} ${LDFLAGS_BLOCKS}

clean:
	rm -f pdwm dwmblocks ${OBJ} ${OBJ_BLOCKS} dwm-${VERSION}.tar.gz *.orig *.rej *.so

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.mk\
		dwm.1 drw.h util.h keys.h ${SRC} transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f pdwm dwmblocks ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/pdwm
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwmblocks
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f dwm.1 ${DESTDIR}${MANPREFIX}/man1/dwm.1
	mkdir -p ${DESTDIR}${PREFIX}/share/phyos/dwm/icons
	cp -f patches/icons/*.png ${DESTDIR}${PREFIX}/share/phyos/dwm/icons
	mkdir -p $(DESTDIR)${PREFIX}/share/xsessions
	cp -f patches/pdwm.desktop ${DESTDIR}${PREFIX}/share/xsessions/pdwm.desktop
	mkdir -p ${DESTDIR}${PREFIX}/lib
	cp -f libdwm-conf.so ${DESTDIR}${PREFIX}/lib
	mkdir -p ${DESTDIR}${PREFIX}/lib/pkgconfig
	cp -f dwm-conf.pc ${DESTDIR}${PREFIX}/lib/pkgconfig

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/pdwm \
		${DESTDIR}${PREFIX}/bin/dwmblocks \
		${DESTDIR}${MANPREFIX}/man1/dwm.1
	rm -rf ${DESTDIR}${PREFIX}/share/phyos/dwm
	rm -f ${DESTDIR}${PREFIX}/share/xsessions/pdwm.desktop
	rm -f ${DESTDIR}${PREFIX}/lib/libdwm-conf.so \
		${DESTDIR}${PREFIX}/lib/pkgconfig/dwm-conf.pc

.PHONY: all dwm-conf options clean dist install uninstall
