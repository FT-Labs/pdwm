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

 - light
 - picom
 - unclutter
 - rofi
 - dmenu
 - lf
 - xmenu
 - s-tui (stress test)
 - cointop (terminal crypto prices)
 - nvtop (nvidia gpu monitor)
 - pamixer
 - pulsemixer
 - imlib2
 - maim
 - joypixels emoji font

#### Non-arch users need to install all of them seperately from links below:
Programs below are all **source code modified** programs therefore all of them are needed to download from:
 - Simple installation would work for all repos other than fonts-phyOS, clone the repo and:
     `make && sudo make install`

Main account : **https://github.com/PhyTech-R0**

 - https://github.com/PhyTech-R0/dunst-phyOS
 - https://github.com/PhyTech-R0/fonts-phyOS
 - https://github.com/PhyTech-R0/st-phyOS
 - https://github.com/PhyTech-R0/dwmblocks-phyOS

## IMPORTANT!! READ BELOW
Make sure to use my dotfiles, all of the **scripts** and path variables need to be set correctly for full functionality.
 - https://github.com/PhyTech-R0/dotfiles


### I am currently maintaning this for arch linux only, therefore arch users can follow the below instructions to install everything easily:

Append package repo end of your `/etc/pacman.conf` :

    [phyOS-repo]
    SigLevel = Optional TrustedOnly
    Server = https://PhyTech-R0.github.io/$repo/$arch
Then install necessary programs with **pacman** easily:

    pacman -Sy phyOS-dwm phyOS-dunst phyOS-dwmblocks phyOS-st phyOS-fonts phyOS-dmenu phyOS-xmenu rofi picom unclutter lf ttf-joypixels light

#### Installation for different distros then arch linux:
    git clone https://github.com/PhyTech-R0/dwm-phyOS
    cd dwm-phyOS && make && sudo make install
You need to install fonts to your system first (Nerd fonts, including all glyphs etc.):
    git clone github.com/phytech-r0/fonts-phyOS
	cd fonts-phyOS && sudo mv *.otf *.ttf /usr/fonts/ttf
	sudo fc-cache

### ! IMPORTANT
Settings button is unfunctional currently. Later on, i will make a simple Qt program to create and load configurations via **settings** button. All of the keybinds will be configurable through a simple gui, which will save you from configuring via _config.h_ file manually.
Note that keybinds & default programs are not in **config.h**. To change default configurations, please change them in **keys.h**. Arch users can change keybinds differently via a script called **phyup** in dotfiles repo. It will be explained later in this file.

#### For arch users (if you are using dotfiles)
You can copy "keys.h" file to '~/.config/phyos/dwm/' to change default bindings. Change keys.h with your taste, after that run:
    phyup dwm
To update dwm easily. (wget required, you probably have it)

## Default keys (same in keys.md)


### NOTE: <kbd>Caps Lock</kbd> == <kbd>Win</kbd>
### NOTE: <kbd>Caps Lock</kbd> is also equal to <kbd>ESC</kbd> in terminal (vim etc..)

## Program Keys:

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>q</kbd>          | Quit focused
 <kbd>Win</kbd> + <kbd>Return</kbd>          | Terminal(st)
 <kbd>Win</kbd> + <kbd>BackSpace</kbd>   | Power menu
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>Return</kbd>  | Scratchpad (Terminal) [Press again to toggle]
 <kbd>Win</kbd> + <kbd>w</kbd>   | $BROWSER(default brave)
 <kbd>Win</kbd> + <kbd>r</kbd>   | File Browser (lf)
 <kbd>Win</kbd> + <kbd>d</kbd>   | Execute Applications (rofi)
 <kbd>Win</kbd> + <kbd>b</kbd>   | Toggle status bar (hide)
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>d</kbd>   | Execute Any Runnable (dmenu)
 <kbd>Win</kbd> + <kbd>p</kbd>   | gotop (System monitor)
 <kbd>Win</kbd> + <kbd>Alt</kbd> + <kbd>p</kbd>   | gpu monitor (nvidia)
 <kbd>Alt</kbd> + <kbd>p</kbd>   | stress test and monitor (s-tui)
 <kbd>Win</kbd> + <kbd>F4</kbd>   | pulsemixer (audio input)
 <kbd>Win</kbd> + <kbd>F5</kbd>   | set multiple monitors and choose refresh rate
 <kbd>Win</kbd> + <kbd>F9</kbd>   | mount drive
 <kbd>Win</kbd> + <kbd>F10</kbd>   | unmount drive
 <kbd>Win</kbd> + <kbd>F11</kbd>   | Camera
 <kbd>Printscreen</kbd>   | Screenshot fullscreen
 <kbd>Shift</kbd> + <kbd>Printscreen</kbd>   | Screenshot (choose area or screen)
 <kbd>Win</kbd> + <kbd>\`</kbd>   | Choose emoji and copy

</div>
<div>

## Terminal keybinds

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Alt</kbd> + <kbd>Shift</kbd> + <kbd>j</kbd>     | Decrease font size (zoom -)
 <kbd>Alt</kbd> + <kbd>Shift</kbd> + <kbd>k</kbd>     | Increase font size (zoom +)
 <kbd>Alt</kbd> + <kbd>o</kbd>      | Copy output of command
 <kbd>Alt</kbd> + <kbd>;</kbd>      | Cycle fonts

</div>
<div>


## Window Movement

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Alt</kbd> + <kbd>Tab</kbd>          | Choose window and focus
 <kbd>Win</kbd> + <kbd>j</kbd>          | Focus next
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>j</kbd>          | Move focused to next
 <kbd>Win</kbd> + <kbd>k</kbd>          | Focus previous
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>k</kbd>          | Move focused to previous
 <kbd>Win</kbd> + <kbd>l</kbd>          | Resize master +
 <kbd>Win</kbd> + <kbd>h</kbd>          | Resize master -
 <kbd>Win</kbd> + <kbd>s</kbd>          | Sticky window toggle (show window in all tags)
 <kbd>Win</kbd> + <kbd>f</kbd>          | Fullscreen (toggle)
 <kbd>Win</kbd> + <kbd>g</kbd>          | Toggle gaps
 <kbd>Win</kbd> + <kbd>x</kbd>          | Decrease gaps
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>x</kbd>          | Increase gaps
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>g</kbd>          | Set default gaps
 <kbd>Win</kbd> + <kbd>m</kbd>          | Increase master (just try it to understand)
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>m</kbd>          | Decrease master (just try it to understand)
 <kbd>Win</kbd> + <kbd>Space</kbd>          | Set focused as master
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>Space</kbd>          | Toggle floating window

</div>
<div>


## Layouts

### You can Right Click <kbd>RMB</kbd> on ' []= ' in status bar to choose layout with mouse

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
 <kbd>Win</kbd> + <kbd>Right</kbd>          | Cycle layout next
 <kbd>Win</kbd> + <kbd>Left</kbd>          | Cycle layout previous

</div>
<div>


## Tag operations

### You can Left Click <kbd>LMB</kbd> on 1-9 in status bar to focus tag

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>1</kbd> .. <kbd>9</kbd>          | Go to tag **number**
 <kbd>Win</kbd> + <kbd>WheelUp</kbd>           | Go to next tag
 <kbd>Win</kbd> + <kbd>WheelDown</kbd>           | Go to previous tag
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>1</kbd> .. <kbd>9</kbd>          | Send focused window to tag **number**

</div>
<div>


## Utility keys

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Sound Up</kbd>        | Increase sound
 <kbd>Sound Down</kbd>        | Decrease sound
 <kbd>Brightness Down</kbd>        | Decrease brightness
 <kbd>Brightness Up</kbd>        | Increase brightness
 <kbd>fn</kbd> + <kbd>TouchpadToggle</kbd> | Toggle touchpad
 <kbd>fn</kbd> + <kbd>Kbd Backlight Up</kbd> | Increase keyboard backlight
 <kbd>fn</kbd> + <kbd>Kbd Backlight Down</kbd> | Decrease keyboard backlight

</div>
<div>

## Mouse actions

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>LMB</kbd>     | Move window with mouse (sets window to floating mode)
 <kbd>Win</kbd> + <kbd>RMB</kbd>     | Resize window with mouse (sets window to floating mode)
 <kbd>Middle Click</kbd>			 | At edge of screen middle click to hide status bar

</div>
<div>
