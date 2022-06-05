static const Rule rules[] = {
    /* xprop(1):
     *  WM_CLASS(STRING) = instance, class
     *  WM_NAME(STRING) = title
    */
    /* class    instance      title          tags mask    isfloating   isterminal  iscentered, noswallow  monitor */
    { TERMCLASS,   NULL,       NULL,            0,            0,           1,         0,         0,        -1 },
    { NULL,       NULL,       "Event Tester",   0,            0,           0,         0,         1,        -1 },
    { NULL,      "spterm",     NULL,            SPTAG(0),     1,           1,         0,         0,        -1 },
    { NULL,      "key_pdf",    NULL,            0,            1,           1,         1,         0,        -1 },
};

static Key keys[] = {
    /* modifier                     key        function        argument */
    STACKKEYS(MODKEY,                          focus)
    STACKKEYS(MODKEY|ShiftMask,                push)
    /*{ MODKEY,         XK_F1,      spawn,      SHCMD("") },
    { MODKEY,           XK_F2,      spawn,      SHCMD("")},
    { MODKEY,           XK_F3,      spawn,      SHCMD(TERMINAL "")  },*/
    { MODKEY,           XK_F4,      spawn,      SHCMD(TERMINAL " -e pulsemixer; kill -44 $(pidof dwmblocks)") },
    { MODKEY,           XK_F5,      spawn,      SHCMD("displayselect") },
    /*{ MODKEY,         XK_F6,      spawn,      SHCMD("") },
    { MODKEY,           XK_F7,      spawn,      SHCMD("") },*/
    { MODKEY,           XK_F9,      spawn,      SHCMD("dmenumount") },
    { MODKEY,           XK_F10,     spawn,      SHCMD("dmenuumount") },
    { MODKEY,           XK_F11,     spawn,      SHCMD("mpv --no-cache --no-osc --no-input-default-bindings --profile=low-latency --input-conf=/dev/null --title=webcam $(ls /dev/video[0,2,4,6,8] | tail -n 1)") },
    TAGKEYS(            XK_1,       0)
    TAGKEYS(            XK_2,       1)
    TAGKEYS(            XK_3,       2)
    TAGKEYS(            XK_4,       3)
    TAGKEYS(            XK_5,       4)
    TAGKEYS(            XK_6,       5)
    TAGKEYS(            XK_7,       6)
    TAGKEYS(            XK_8,       7)
    TAGKEYS(            XK_9,       8)
    { MODKEY,           XK_0,       view,       {.ui = ~0 } },
    { MODKEY,           XK_grave,   spawn,  SHCMD("dmenuunicode") },
    { MOD2KEY,          XK_1,       setlayout,  {.v = &layouts[0]} }, /* tile */
    { MOD2KEY,          XK_2,       setlayout,  {.v = &layouts[1]} }, /* bstack */
    { MOD2KEY,          XK_3,       setlayout,  {.v = &layouts[2]} }, /* monocle */
    { MOD2KEY,          XK_4,       setlayout,  {.v = &layouts[3]} }, /* deck */
    { MOD2KEY,          XK_5,       setlayout,  {.v = &layouts[4]} }, /* spiral */
    { MOD2KEY,          XK_6,       setlayout,  {.v = &layouts[5]} }, /* dwindle */
    { MOD2KEY,          XK_7,       setlayout,  {.v = &layouts[6]} }, /* centeredmaster */
    { MOD2KEY,          XK_8,       setlayout,  {.v = &layouts[7]} }, /* centeredfloatingmaster */
    { MOD2KEY,          XK_9,       setlayout,  {.v = &layouts[8]} },      /* Floating layout */
    { MODKEY,           XK_Escape,  spawn,      SHCMD("sysact") },
    { MODKEY,           XK_BackSpace,  spawn,   SHCMD("sysact") },
    { MOD2KEY,          XK_Tab,     spawn,      SHCMD("rofi -show window") },
    { MODKEY,           XK_q,       killclient, {0} },
    { MODKEY,           XK_w,       spawn,      SHCMD("$BROWSER") },
    { MODKEY,           XK_t,       spawn,      SHCMD("pidof -s $COMPOSITOR && killall -9 $COMPOSITOR || $COMPOSITOR --experimental-backends --backend glx &") },
    { MODKEY,           XK_r,       spawn,      SHCMD(TERMINAL " -e lf") },
    { MODKEY,           XK_p,       spawn,      SHCMD("passmenu") },
    { MODKEY|MOD2KEY,   XK_p,       spawn,      SHCMD(TERMINAL " -e htop") },
    { MOD2KEY,          XK_p,       spawn,      SHCMD(TERMINAL " -e s-tui") },
    { MODKEY,           XK_s,       togglesticky,   {0} },
    { MODKEY,           XK_d,       spawn,      SHCMD("rofi -show drun") },
    { MODKEY|ShiftMask, XK_d,       spawn,          {.v = dmenucmd } },
    { MODKEY,           XK_f,       togglefullscr,  {0} },
    { MODKEY,           XK_h,       setmfact,   {.f = -0.05} },
    { MOD2KEY,          XK_h,       tagmon,     {.i = -1 } },
    { MODKEY,           XK_g,       togglegaps, {0} },
    { MODKEY|ShiftMask, XK_g,       defaultgaps,    {0} },
    { MOD2KEY,          XK_j,       focusmon,   {.i = -1 } },
    { MOD2KEY,          XK_k,       focusmon,   {.i = +1 } },
    { MOD2KEY,          XK_l,       tagmon,     {.i = +1 } },
    { MODKEY,           XK_l,       setmfact,       {.f = +0.05} },
    { MODKEY,           XK_Return,  spawn,      {.v = termcmd } },
    { MODKEY|ShiftMask, XK_Return,  togglescratch,  {.ui = 0} },
    { MODKEY,           XK_x,       incrgaps,   {.i = +3 } },
    { MODKEY|ShiftMask, XK_x,       incrgaps,   {.i = -3 } },
    { MODKEY,           XK_b,       togglebar,  {0} },
    { MODKEY,           XK_m,       incnmaster,     {.i = +1 } },
    { MODKEY|ShiftMask, XK_m,       incnmaster,     {.i = -1 } },
    { MODKEY,           XK_space,   zoom,       {0} },
    { MODKEY|ShiftMask, XK_space,   togglefloating, {0} },
    { 0,                XK_Print,   spawn,      SHCMD("maim ~/Pictures/pic-full-$(date '+%y%m%d-%H%M-%S').png") },
    { ShiftMask,        XK_Print,   spawn,      SHCMD("maimpick ~/Pictures/") },
    { MODKEY,           XK_Print,   spawn,      SHCMD("dmenurecord") },
    { MODKEY|ShiftMask, XK_Print,   spawn,      SHCMD("dmenurecord kill") },
    { MODKEY,           XK_Left,                cyclelayout, {.i = -1} },
    { MODKEY,           XK_Right,               cyclelayout, {.i = +1} },
    { 0, XF86XK_AudioMute,          spawn,      SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
    { 0, XF86XK_AudioRaiseVolume,   spawn,      SHCMD("pamixer --allow-boost -i 3; kill -44 $(pidof dwmblocks)") },
    { 0, XF86XK_AudioLowerVolume,   spawn,      SHCMD("pamixer --allow-boost -d 3; kill -44 $(pidof dwmblocks)") },
    { 0, XF86XK_AudioPrev,          spawn,      SHCMD("mpc prev") },
    { 0, XF86XK_AudioNext,          spawn,      SHCMD("mpc next") },
    { 0, XF86XK_AudioPause,         spawn,      SHCMD("mpc pause") },
    { 0, XF86XK_AudioPlay,          spawn,      SHCMD("mpc play") },
    { 0, XF86XK_AudioStop,          spawn,      SHCMD("mpc stop") },
    { 0, XF86XK_AudioRewind,        spawn,      SHCMD("mpc seek -10") },
    { 0, XF86XK_AudioForward,       spawn,      SHCMD("mpc seek +10") },
    { 0, XF86XK_AudioMedia,         spawn,      SHCMD(TERMINAL " -e ncmpcpp") },
    { 0, XF86XK_AudioMicMute,       spawn,      SHCMD("pactl set-source-mute @DEFAULT_SOURCE@ toggle") },
    { 0, XF86XK_PowerOff,           spawn,      SHCMD("sysact") },
    { 0, XF86XK_Calculator,         spawn,      SHCMD(TERMINAL " -e bc -l") },
    { 0, XF86XK_WWW,                spawn,      SHCMD("$BROWSER") },
    { 0, XF86XK_DOS,                spawn,      SHCMD(TERMINAL) },
    { 0, XF86XK_TouchpadToggle,     spawn,      SHCMD("touchpadtap") },
    { 0, XF86XK_MonBrightnessUp,    spawn,      SHCMD("light -A 2 && brightness") },
    { 0, XF86XK_MonBrightnessDown,  spawn,      SHCMD("light -U 2 && brightness") },
    { 0, XF86XK_KbdBrightnessDown,  spawn,      SHCMD("var=$( ls /sys/class/leds/ | grep kbd_backlight ); light -s sysfs/leds/$var -r -U 1")},
    { 0, XF86XK_KbdBrightnessUp,    spawn,      SHCMD("var=$( ls /sys/class/leds/ | grep kbd_backlight ); light -s sysfs/leds/$var -r -A 1")},
};
