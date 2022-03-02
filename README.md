## dwm-phyOS



 This modified suckless **dwm** window manager is built for arch based _phyOS_ distro but can be installed with following for any linux OS.
 ### Requirement for total functionality:


 - picom
 - unclutter
 - rofi
 - dmenu
 - lf
 - xmenu

Programs below are all **source code modified** programs therefore all of them are needed to download from:
 **https://github.com/PhyTech-R0**

 - https://github.com/PhyTech-R0/dunst-phyOS
 - https://github.com/PhyTech-R0/fonts-phyOS
 - https://github.com/PhyTech-R0/st-phyOS
 - https://github.com/PhyTech-R0/dwmblocks-phyOS

#### MAKE SURE TO USE DOTFILES REPO, ALL PROGRAM CONFIGURATION AND SCRIPTS ARE IN IT
 - https://github.com/PhyTech-R0/dotfiles

### I am currently maintaning this for arch linux only, therefore arch users can follow the below instructions to install everything easily:

Append package repo end of your `/etc/pacman.conf` :

    [phyOS-repo]
    SigLevel = Optional TrustedOnly
    Server = https://PhyTech-R0.github.io/$repo/$arch
Then install necessary programs with **pacman** easily:

    pacman -Sy phyOS-dwm phyOS-dunst phyOS-dwmblocks phyOS-st phyOS-fonts phyOS-dmenu phyOS-xmenu rofi picom unclutter lf

#### Installation for different distros then arch linux:

    git clone https://github.com/PhyTech-R0/dwm-phyOS
    cd dwm-phyOS && make && sudo make install
### ! IMPORTANT
Settings button is unfunctional currently. Later on, i will make a simple Qt program to create and load configurations via **settings** button. All of the keybinds will be configurable through a simple gui, which will save you from configuring via _config.h_ file manually.
Note that keybinds & default programs are not in **config.h**. To change default configurations, please change them in **keys.h**. Arch users can change keybinds differently via a script called **phyup** in dotfiles repo. It will be explained later in this file.
