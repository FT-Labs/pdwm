# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dwm.c util.c
OBJ = ${SRC:.c=.o}
SRC_BLOCKS = dwmblocks.c
OBJ_BLOCKS = ${SRC_BLOCKS:.c=.o}

all: options dwm dwmblocks

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

%.o: %.c
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h keys.h config.mk

${OBJ_BLOCKS}: dwmblocks.h

dwm: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

dwmblocks: ${OBJ_BLOCKS}
	${CC} -o $@ ${OBJ_BLOCKS} ${LDFLAGS_BLOCKS}

clean:
	rm -f dwm dwmblocks ${OBJ} ${OBJ_BLOCKS} dwm-${VERSION}.tar.gz *.orig *.rej

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.mk\
		dwm.1 drw.h util.h keys.h ${SRC} transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwm dwmblocks ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwmblocks
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f dwm.1 ${DESTDIR}${MANPREFIX}/man1/dwm.1
	#mkdir -p ${DESTDIR}${PREFIX}/share/phyos/dwm/icons
	#cp -f patches/icons/*.png ${DESTDIR}${PREFIX}/share/phyos/dwm/icons
	#mkdir -p $(DESTDIR)${PREFIX}/share/xsessions
	#cp -f patches/dwm.desktop ${DESTDIR}${PREFIX}/share/xsessions/dwm.desktop

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm \
		${DESTDIR}${PREFIX}/bin/dwmblocks \
		${DESTDIR}${MANPREFIX}/man1/dwm.1
	rm -rf ${DESTDIR}${PREFIX}/share/phyos/dwm
	#rm -f ${DESTDIR}${PREFIX}/share/xsessions/dwm.desktop

.PHONY: all options clean dist install uninstall
