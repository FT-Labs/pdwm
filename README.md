### pdwm & phyOS-dwm
#### pdwm & phyOS-dwm are both suckless dwm forks. Both of them are in this repo, please read below to understand why. Also note that dwmblocks is embedded in this project, you don't need to download it from somewhere else.
#### For full animation support, please install ``` phyOS-picom ``` fork. Any other picom fork won't work.
#### phyOS-dwm is the oldest master branch of this project. This has been changed to pdwm branch. phyOS-dwm is just modified suckless dwm, with dwmblocks in it. Please note that this dwmblocks uses libconf library, which you can add/remove statusbar blocks with .cfg files. Also more than 10 statusbar blocks available for phyOS users. However, all of these can be installed on base arch linux too. Statusbar configuration can be made easily with ``` pOS-make-bar ``` script.
#### Settings button on top left (blue one) will open up a terminal application, which you can choose powermenu themes - colorschemes, and animation options from there. It also has some extra scripts in it to make your statusbar, choose sddm & grub themes.
### Gifs

![alt text](https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/g1.gif)
![alt text](https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/g2.gif)

### Screenshots
(Alt + Tab to switch windows)
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/s1.png">
(Hide and restore windows from bottom dock (Check keys for how to do this))
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/s2.png">
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/s3.png">
(catppuccin & tokyo night theme)
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/s4.png">
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/s5.png">
(Power menu 'right one is to renew dwm without restart')
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/s6.png">
#### Why the change? What is pdwm?
Originally, dwm configuration options are compiled into a single binary. Therefore, to be able to change any attribute you need to edit configuration file and recompile it into dwm again. With pdwm, this changes. Since dwm is supposed to be minimalistic and performant, simply putting a configuration library to handle these options doesn't make sense. It will increase SLOC a lot. To overcome this issue, another simple method has been made:
All of the configuration variables in dwm (nearly everything, except some edge cases) have been marked as extern variables. In ``` pdwm ``` folder, you can see all the configuration options. These options are made a shared library (.so). With this flexibility, and minor lines of code, at the execution stage of dwm, all of the variables are being loaded into dwm from an outside source, which is the shared library. This way, by just compiling the variables and renewing dwm, all options have been renewed. Without even using ``` sudo ```.
Note that from now on, mostly pdwm will get updated. However, the older fork, phyOS-dwm is still available on master branch. This is much of a choice now, choose whatever you prefer.
The tool to configure dwm is a python program, which is called ``` pdwmc ```. It is the main control center of pdwm.
Link: https://github.com/FT-Labs/pdwmc
#### pdwm Usage
**IMPORTANT**: Please extend your "LD_LIBRARY_PATH" to this location:
E.G: ``` export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$HOME/.config/phyos/pdwm" ```
You must do this, because fallback library will be in /usr/lib. Your local configurations (.so) file will be stored here, and you must extend it to this place for linux ld linker to be able to see it.
As said before pdwm is a simple python program. It both has support for changing/printing any configuration options from terminal, or via QT app. Now lets get on with options.
```
usage: pdwmc [-h] [-w] [-b] [-g] [-s {appearance,buttons,keys,rules}]
            [-c {appr,font,button,key,rule}] [-a {font,button,key,rule}]
            [-d {font,button,key,rule}] [-q]

options:
  -h, --help            show this help message and exit
  -w, --write           Write current dwm configuration to edit [Use this before to
                        edit dwm configuration] or reset your changes
  -b, --build           Save changes to dwm
  -g, --get             Get default dwm settings (overrides current) to use pdwm,
                        also runs -w flag
  -s {appearance,buttons,keys,rules}, --show {appearance,buttons,keys,rules}
  -c {appr,font,button,key,rule}, --change {appr,font,button,key,rule}
                        Choose and change attribute
  -a {font,button,key,rule}, --add {font,button,key,rule}
                        Add new attribute
  -d {font,button,key,rule}, --delete {font,button,key,rule}
                        Delete an attribute
  -q, --qt              Run pdwm as a QT application
  ```
  If you prefer a gui application, just run ``` pdwmc -q ```. It is a simple and editable QT app, also catches your keypresses, which will save you some time from learning keysym's from internet. If key is not detected, simply find it from google and edit the key.
### pdwm Gui screenshots
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/pdwm1.png">
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/pdwm2.png">
<img src="https://github.com/FT-Labs/phyOS-dwm/blob/screenshots/screenshots/pdwm3.png">

### To be able to install these, please follow the steps below. Also if interested, you can download iso image to a vm, build it then update to system to be able to try.
#### These explanations are only for arch linux based distributions (any pacman using distro is okay). There are important dependencies, which I could not find all packages to fit all distributions. Later on a debian package version may come too.

Append package repo end of your `/etc/pacman.conf` :

    [phyOS-repo]
    SigLevel = Required DatabaseOptional
    Server = https://FT-Labs.github.io/$repo/$arch

After adding the repo, install keyring first:

    sudo pacman-key --recv-key 964FD85861C858D7
    sudo pacman-key --lsign-key 964FD85861C858D7
    sudo pacman -Syy phyOS-keyring
    sudo pacman-key --init
    sudo pacman-key --populate phyOS
Now install necessary packages (This will install most of the phyOS packages, if you which packages to choose simply install them, this is a general guide):
``` sudo pacman -Syy ```
``` curl [https://raw.githubusercontent.com/FT-Labs/phyOS-Aug-22/master/packages.x86_64](https://raw.githubusercontent.com/FT-Labs/phyOS-Aug-22/master/packages.x86_64) | sed -e '/\#/d' | tr -s "\n" | xargs sudo pacman -S --noconfirm ```
After the installation, please reboot your computer. Now you will need dotfiles, please move your current dots to somewhere else if they are important.
Run: ``` phyup dots --force ``` to get latest dotfiles. This is required to set your .xinitrc, .xprofile and picom options etc. correctly. Anyway, if you want you can just check dotfiles repo and choose whatever is required from there.
pdwm Packages: pdwm pdwmc
phyOS-dwm (classic compiled dwm) packages: phyOS-dwm

#### Running:
Simply run startx from tty, or use a display manager. Desktop file is automatically made and installed.

### NOTE: <kbd>Caps Lock</kbd> == <kbd>Win</kbd>
### NOTE: <kbd>Caps Lock</kbd> is also equal to <kbd>ESC</kbd> in terminal (vim etc..)

## Program Keys (man dwm will work too, or try keys sheet):

<div align="center">

Key Combination | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>q</kbd>          | Quit focused
 <kbd>Win</kbd> + <kbd>Return</kbd>          | Terminal(st)
 <kbd>Win</kbd> + <kbd>ESC</kbd>   | Power menu
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>Return</kbd>  | Scratchpad (Terminal) [Press again to toggle]
 <kbd>Win</kbd> + <kbd>w</kbd>   | $BROWSER(default ungoogled chromium)
 <kbd>Win</kbd> + <kbd>r</kbd>   | File Browser (lf)
 <kbd>Win</kbd> + <kbd>a</kbd>   | Execute Applications (rofi)
 <kbd>Win</kbd> + <kbd>b</kbd>   | Toggle status bar (hide)
 <kbd>Win</kbd> + <kbd>d</kbd>   | Jump to directory with terminal (check 'bm' script to add bookmarks)
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>a</kbd>   | Execute Any Runnable (dmenu)
 <kbd>Win</kbd> + <kbd>p</kbd>   | passmenu (pass password manager)
 <kbd>Alt</kbd> + <kbd>p</kbd>   | stress test and monitor (s-tui)
 <kbd>Win</kbd> + <kbd>F4</kbd>   | pulsemixer (audio input)
 <kbd>Win</kbd> + <kbd>F5</kbd>   | set multiple monitors and choose refresh rate
 <kbd>Win</kbd> + <kbd>F9</kbd>   | mount drive
 <kbd>Win</kbd> + <kbd>F10</kbd>   | unmount drive
 <kbd>Win</kbd> + <kbd>F11</kbd>   | Camera
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>s</kbd>  | Screenshot (choose area or screen)
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
 <kbd>Win</kbd> + <kbd>Tab</kbd>          | Choose window from current desktop
 <kbd>Alt</kbd> + <kbd>Tab</kbd>          | Choose any window and focus
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
### You can Right Click <kbd>RMB</kbd> on 1-9 in status bar to select multiple tags

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
