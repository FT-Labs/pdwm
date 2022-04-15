# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dwm.c util.c
OBJ = ${SRC:.c=.o}

all: options dwm

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

%.o: %.c
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h keys.h config.mk

dwm: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dwm ${OBJ} dwm-${VERSION}.tar.gz *.orig *.rej

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.mk\
		dwm.1 drw.h util.h keys.h ${SRC} transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	mkdir -p ${DESTDIR}${PREFIX}/share/phyos/dwm/icons
	cp -f icons/*.png ${DESTDIR}${PREFIX}/share/phyos/dwm/icons
	cp -f keys-dwm.pdf ${DESTDIR}${PREFIX}/share/phyos/dwm/
	mkdir -p $(DESTDIR)${PREFIX}/share/xsessions
	cp -f dwm.desktop ${DESTDIR}${PREFIX}/share/xsessions/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/man1/dwm.1
	rm -rf ${DESTDIR}${PREFIX}/share/phyos/dwm
	rm -f ${DESTDIR}${PREFIX}/share/xsessions/dwm.desktop

.PHONY: all options clean dist install uninstall
