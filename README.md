## dwm-phyOS

### Screenshots

(Alt + Tab to switch windows)
<img src="https://github.com/PhyTech-R0/dwm-phyOS/blob/master/screenshots/s1.png">

<img src="https://github.com/PhyTech-R0/dwm-phyOS/blob/master/screenshots/s2.png">
<img src="https://github.com/PhyTech-R0/dwm-phyOS/blob/master/screenshots/s3.png">
(catppuccin & tokyo night theme)
<img src="https://github.com/PhyTech-R0/dwm-phyOS/blob/master/screenshots/s4.png">
<img src="https://github.com/PhyTech-R0/dwm-phyOS/blob/master/screenshots/s5.png">


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

## Default keys (same in keys.md)


### NOTE: <kbd>Caps Lock</kbd> == <kbd>Super</kbd>
### NOTE: <kbd>Caps Lock</kbd> is also equal to <kbd>ESC</kbd> in terminal (vim etc..)

## Program Keys:

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Super</kbd> + <kbd>q</kbd>          | Quit focused
 <kbd>Super</kbd> + <kbd>Return</kbd>          | Terminal(st)
 <kbd>Super</kbd> + <kbd>BackSpace</kbd>   | Power menu
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>Return</kbd>  | Scratchpad (Terminal) [Press again to toggle]
 <kbd>Super</kbd> + <kbd>w</kbd>   | $BROWSER(default brave)
 <kbd>Super</kbd> + <kbd>r</kbd>   | File Browser (lf)
 <kbd>Super</kbd> + <kbd>d</kbd>   | Execute Applications (rofi)
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>d</kbd>   | Execute Any Runnable (dmenu)
 <kbd>Super</kbd> + <kbd>p</kbd>   | gotop (System monitor)
 <kbd>Super</kbd> + <kbd>Alt</kbd> + <kbd>p</kbd>   | gpu monitor (nvidia)
 <kbd>Alt</kbd> + <kbd>p</kbd>   | stress test and monitor (s-tui)
 <kbd>Super</kbd> + <kbd>F4</kbd>   | pulsemixer (audio input)
 <kbd>Super</kbd> + <kbd>F5</kbd>   | set multiple monitors and choose refresh rate
 <kbd>Super</kbd> + <kbd>F9</kbd>   | mount drive
 <kbd>Super</kbd> + <kbd>F10</kbd>   | unmount drive
 <kbd>Super</kbd> + <kbd>F11</kbd>   | Camera
 <kbd>Shift</kbd> + <kbd>Printscreen</kbd>   | Screenshot (choose area or screen)

</div>
<div>

## Window Movement

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Super</kbd> + <kbd>j</kbd>          | Focus next
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>j</kbd>          | Move focused to next
 <kbd>Super</kbd> + <kbd>k</kbd>          | Focus previous
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>k</kbd>          | Move focused to previous
 <kbd>Super</kbd> + <kbd>l</kbd>          | Resize master +
 <kbd>Super</kbd> + <kbd>h</kbd>          | Resize master -
 <kbd>Super</kbd> + <kbd>s</kbd>          | Sticky window toggle (show window in all tags)
 <kbd>Super</kbd> + <kbd>f</kbd>          | Fullscreen (toggle)
 <kbd>Super</kbd> + <kbd>g</kbd>          | Toggle gaps
 <kbd>Super</kbd> + <kbd>x</kbd>          | Decrease gaps
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>x</kbd>          | Increase gaps
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>g</kbd>          | Set default gaps
 <kbd>Super</kbd> + <kbd>m</kbd>          | Increase master (just try it to understand)
 <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>m</kbd>          | Decrease master (just try it to understand)
 <kbd>Super</kbd> + <kbd>Space</kbd>          | Set focused as master

</div>
<div>


## Layouts

### You can Right Click <kbd>RMB</kbd> on []= in status bar to choose layout with mouse

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Alt</kbd> + <kbd>1</kbd>          | Tiling (Default)
 <kbd>Alt</kbd> + <kbd>2</kbd>          | Backstack
 <kbd>Alt</kbd> + <kbd>3</kbd>          | Monocle (windows top of each)
 <kbd>Alt</kbd> + <kbd>4</kbd>          | Deck layout
 <kbd>Alt</kbd> + <kbd>5</kbd>          | Spiral
 <kbd>Alt</kbd> + <kbd>6</kbd>          | Dwindle
 <kbd>Alt</kbd> + <kbd>7</kbd>          | Centered master
 <kbd>Alt</kbd> + <kbd>8</kbd>          | Centered floating master
 <kbd>Alt</kbd> + <kbd>9</kbd>          | Floating (windows 10 style)

</div>
<div>


## Choose tag

### You can Left Click <kbd>LMB</kbd> on 1-9 in status bar to focus tag

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Super</kbd> + <kbd>1</kbd> .. <kbd>9</kbd>          | Go to tag **number**

</div>
<div>
