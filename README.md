## dwm-phyOS (dwm and dwmblocks in one)
## Clickable and modifiable dwmblocks without compiling, all editing can be done from configuration files

### DO NOT USE GITLAB ISO FOR NOW, IT IS OLD. LINK WILL BE HERE WHEN IT IS AVAILABLE.

### Gifs

![alt text](https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/g1.gif)
![alt text](https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/g2.gif)

### Screenshots
(Alt + Tab to switch windows)
<img src="https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/s1.png">
(Hide and restore windows from bottom dock (Check keys for how to do this))
<img src="https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/s2.png">
<img src="https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/s3.png">
(catppuccin & tokyo night theme)
<img src="https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/s4.png">
<img src="https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/s5.png">
(Power menu 'right one is to renew dwm without restart')
<img src="https://github.com/PhyTech-R0/phyOS-dwm/blob/screenshots/screenshots/s6.png">

 This modified suckless **dwm** window manager is built for arch based _phyOS_ distro but can be installed with following for any linux OS.
 - Beta version iso has been released on gitlab. It will automatically install everything and clean up bloat packages with calamares.
 - LINK IS OLD, NEW ISO WILL BE AVAILABLE SOON

### Click on 'Keys' button on top left in status bar to check out keybindings.
#### Non-arch users need to install all of them seperately from links below:
Programs below are all **source code modified** programs therefore all of them are needed to download from:
 - Simple installation would work for all repos other than phyOS-fonts, clone the repo and:

     `make && sudo make install`

Main account : **https://github.com/PhyTech-R0**

 - https://github.com/PhyTech-R0/phyOS-dunst
 - https://github.com/PhyTech-R0/phyOS-fonts
 - https://github.com/PhyTech-R0/phyOS-st

### IMPORTANT!! READ BELOW (If you just want statusbar scripts, you can omit this just read below)
Make sure to use my dotfiles, all of the **scripts** and path variables need to be set correctly for full functionality.
 - https://github.com/PhyTech-R0/dotfiles

### ABOUT DWMBLOCKS AND SCRIPTS
All dwmblocks and scripts are in **dwmblocksconfig** directory. All configuration files can be read from single path only,
if it is not found it will revert to default. Note that if __statusbar__ scripts are not in path, it won't work correctly. Please be sure that all scripts are something in your **$PATH** variable.
#### Path for config file must be: "$HOME/.config/phyos/dwmblocks"
- I thought about creating this folder and configuration files automatically, however they will erase on if you changed your config file and reinstalled dwm. Therefore please create this folder, move everything in **dwmblocksconfig** to path above and follow the steps below.
- All blocks in bar is clickable. It will open programs, or make some action if clicked. Just check scripts for how to implement your own.
- In **dwmblocksconfig** folder, there is a main configuration file called __dwmblocks.cfg__ . This is the main config file, you can add or delete any configuration files from there by __@include__ directive, just use the same syntax in that file.
- All the bash scripts can be found in **dwmblocksconfig/scripts**. Choose any scripts you want, then add them on your path, like "/usr/local/bin" etc. or anywhere you want.
- Don't delete .cfg files contained in **dwmblocksconfig/statusbar**, you can add or remove them from main config file. Not included files won't be loaded in dwmblocks.
- Note that most scripts use glyphs, please install a compatible nerd font for it. You can checkout fonts-phyOS repo for default fonts that i currenty use.

### Requirement for total functionality:

 - light
 - picom-animations-git
 - unclutter
 - rofi
 - dmenu
 - conky
 - jgmenu
 - lf
 - s-tui (stress test)
 - cointop (terminal crypto prices)
 - pamixer
 - pulsemixer
 - imlib2
 - maim
 - joypixels emoji font
 - zathura (pdf reader, used for keys)

### I am using this for phyOS arch iso only, therefore arch users can follow the below instructions to install everything easily:

Append package repo end of your `/etc/pacman.conf` :

    [phyOS-repo]
    SigLevel = Required DatabaseOptional
    Server = https://PhyTech-R0.github.io/$repo/$arch

After adding the repo, install keyring first:

    sudo pacman-key --recv-key 964FD85861C858D7 && sudo pacman-key --lsign-key $_
    sudo pacman -Syy phyOS-keyring
    sudo pacman-key --init
    sudo pacman-key --populate phyOS


Then install necessary programs with **pacman** easily:

    pacman -S phyOS-dwm phyOS-dunst phyOS-st phyOS-fonts phyOS-dmenu phyOS-xmenu rofi unclutter lf-png ttf-joypixels light picom-animations-git

#### Installation for different distros then arch linux:

    git clone https://github.com/PhyTech-R0/phyOS-dwm
    cd dwm-phyOS && make && sudo make install
You need to install fonts to your system first (Nerd fonts, including all glyphs etc.):

    git clone github.com/phytech-r0/phyOS-fonts
    cd fonts-phyOS && sudo mv *.otf *.ttf /usr/fonts/
    sudo fc-cache


#### For non-arch and arch users (updating dotfiles and configs)


#### To pull newest dotfiles, just write `phyup dots`

Note that this just pulls newest config from github repo. If any conflict happens, this will give error and won't replace current dotfiles. Just move them elsewhere if you prefer not to change them.

#### If you want to force install dotfiles, write the command below:

	phyup dots --force

Note that this will replace all dots with new ones, if you have your current vim configuration or etc. please move them or rename them before using this command or just make them immutable with something like

	sudo chattr +i .xinitrc

## Default keys (same in keys.md)

### NOTE: <kbd>Caps Lock</kbd> == <kbd>Win</kbd>
### NOTE: <kbd>Caps Lock</kbd> is also equal to <kbd>ESC</kbd> in terminal (vim etc..)

## Program Keys:

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>q</kbd>          | Quit focused
 <kbd>Win</kbd> + <kbd>Return</kbd>          | Terminal(st)
 <kbd>Win</kbd> + <kbd>ESC</kbd>   | Power menu
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>Return</kbd>  | Scratchpad (Terminal) [Press again to toggle]
 <kbd>Win</kbd> + <kbd>w</kbd>   | $BROWSER(default ungoogled chromium)
 <kbd>Win</kbd> + <kbd>r</kbd>   | File Browser (lf)
 <kbd>Win</kbd> + <kbd>d</kbd>   | Execute Applications (rofi)
 <kbd>Win</kbd> + <kbd>b</kbd>   | Toggle status bar (hide)
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>d</kbd>   | Execute Any Runnable (dmenu)
 <kbd>Win</kbd> + <kbd>p</kbd>   | passmenu (pass password manager)
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
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>h</kbd>          | Next Tag
 <kbd>Win</kbd> + <kbd>h</kbd>          | Resize master -
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>h</kbd>          | Previous Tag
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
 <kbd>RMB</kbd>                      | Open Dropdown Menu (On Root Window)
 <kbd>Win</kbd> + <kbd>RMB</kbd>     | Open Dropdown Menu (On Any Client)
 <kbd>Win</kbd> + <kbd>LMB</kbd>     | Move window with mouse (sets window to floating mode)
 <kbd>Win</kbd> + <kbd>MMB</kbd>     | Resize window with mouse (sets window to floating mode)
 <kbd>Alt</kbd> + <kbd>LMB</kbd> | Hide focused window (iconic state, restore it from dock)
 <kbd>Alt</kbd> + <kbd>RMB</kbd> | Toggle dock

</div>
<div>
