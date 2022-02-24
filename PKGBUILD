# Maintainer: Arda Atci<phystecharda@gmail.com>
pkgname=dwm-phyOS
pkgver=6.2
pkgrel=2
pkgdesc="dwm build for phyOS"
arch=(x86_64)
url="git://github.com/PhyTech-R0/dwm-phyOS"
license=('MIT')
depends=('libxcb' 'libxft-bgra' 'libpng')
makedepends=('git' 'make')
optdepends=('fonts-phyOS' 'dmenu-phyOS' 'st-phyOS' 'dwmblocks-phyOS')
provides=("dwm")
conflicts=("dwm")
options=('zipman')
source=('git://github.com/PhyTech-R0/dwm-phyOS')
md5sums=('SKIP')

build() {
	cd "$pkgname"
	[ -f $HOME/.config/phyos/dwm/keys.h ] && yes | cp -f $HOME/.config/phyos/dwm/keys.h ./keys.h
	[ -f $HOME/.config/phyos/dwm/*.png ] && yes | cp -f $HOME/.config/phyos/dwm/*.png icons/
	make
}

package() {
	cd "$pkgname"
	make PREFIX=/usr/local DESTDIR="$pkgdir/" install
}
