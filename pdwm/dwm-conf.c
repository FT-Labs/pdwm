/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include "colors.h"
#include <X11/XF86keysym.h>
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define TERMINAL "st"
#define TERMCLASS "St"
#define MAKETERM(TERMINAL, cmd) TERMINAL cmd

extern void cyclelayout(const Arg *arg);
extern void focus(Client *c);
extern void focusmon(const Arg *arg);
extern void focusstack(const Arg *arg);
extern void hide(const Arg *arg);
extern void swaptags(const Arg *arg);
extern void layoutmenu(const Arg * arg);
extern void incnmaster(const Arg *arg);
extern void killclient(const Arg *arg);
extern void monocle(Monitor *m);
extern void movemouse(const Arg *arg);
extern void pushstack(const Arg *arg);
extern void resizemouse(const Arg *arg);
extern void setfocus(Client *c);
extern void setlayout(const Arg *arg);
extern void setmfact(const Arg *arg);
extern void spawn(const Arg *arg);
extern void tag(const Arg *arg);
extern void tagmon(const Arg *arg);
extern void togglebar(const Arg *arg);
extern void togglefloating(const Arg *arg);
extern void togglescratch(const Arg *arg);
extern void togglesticky(const Arg *arg);
extern void togglefullscr(const Arg *arg);
extern void toggletag(const Arg *arg);
extern void toggleview(const Arg *arg);
extern void view(const Arg *arg);
extern void zoom(const Arg *arg);
extern void defaultgaps(const Arg *arg);
extern void incrgaps(const Arg *arg);
extern void togglegaps(const Arg *arg);
extern void bstack(Monitor *m);
extern void centeredmaster(Monitor *m);
extern void centeredfloatingmaster(Monitor *m);
extern void deck(Monitor *m);
extern void dwindle(Monitor *m);
extern void fibonacci(Monitor *m, int s);
extern void spiral(Monitor *m);
extern void tile(Monitor *);
extern void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc);
extern void setgaps(int oh, int ov, int ih, int iv);
extern void shifttag(const Arg *arg);
extern void shiftview(const Arg *arg);
#ifndef __OpenBSD__
extern int getdwmblockspid();
extern void sigdwmblocks(const Arg *arg);
#endif

/* Default icon width, height, margin and delimiter width */
const unsigned int sb_icon_wh = 32;
const unsigned int sb_icon_margin_x = 12;
const unsigned int sb_delimiter_w = 4;
/* Status bar x  y margin */
const unsigned int sb_padding_x = 12;
const unsigned int sb_padding_y = 12;
char dmenufont[]             = "JetBrains Mono:style=Regular:size=16";
char dmenuh[] = "40";

#include "appearance"
const char **get_fonts() {
    return fonts;
}
const int lenfonts           = LENGTH(fonts);

char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { black, black, gray2 },
       [SchemeSel]  = { blue2,  green,  blue  },
       [SchemeTagsSel] = { black, blue, "#000000" },
       [SchemeTagsNorm] = { blue, black, "#000000" },
       [SchemeInfoSel] = { blue, black, "#000000" },
       [SchemeInfoNorm] = { black, black, "#000000" },
       [SchemeStatus] = { white, black, "#000000" },
       [SchemeOptimal] = { green, black, "#000000" },
       [SchemeCritical] = { red, black, "#000000" } ,
};

const char *physettings[] = {TERMINAL, "-n", "physettings", "-g", "120x35", "-e", "physettings", NULL};
const Config config[] = {
    {physettings, ""},
};
const int lenconfig = LENGTH(config);

typedef struct {
    const char *name;
    const void *cmd;
} Sp;
const char *spcmd1[] = {TERMINAL, "-n", "spterm", "-g", "120x34", NULL };
const Sp scratchpads[] = {
    /* name          cmd  */
    {"spterm",      spcmd1},
};

/* tagging */
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/* layout(s) */
const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
const int nmaster     = 1;    /* number of clients in master area */
const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
const Layout layouts[] = {
    /* symbol     arrange function */
    { "[]=",    tile },         /* Default: Master on left, slaves on right */
    { "TTT",    bstack },       /* Master on top, slaves on bottom */
    { "[M]",    monocle },      /* All windows on top of eachother */
    { "H[]",    deck },         /* Master on left, slaves in monocle-like mode on right */
    { "[@]",    spiral },       /* Fibonacci spiral */
    { "[\\]",   dwindle },      /* Decreasing in size right and leftward */
    { "|M|",    centeredmaster },       /* Master in middle, slaves on sides */
    { ">M>",    centeredfloatingmaster },   /* Same but master floats */
    { "><>",    NULL },         /* no layout function means floating behavior */
    { NULL,     NULL }
};

/* key definitions */
#ifndef KEY_DEFS
#define KEY_DEFS
#define Win Mod4Mask
#define Alt Mod1Mask
#define Shift ShiftMask
#define Control ControlMask
#define LeftClick Button1
#define MiddleClick Button2
#define RightClick Button3
#define WheelUp Button4
#define WheelDown Button5
#endif

#include "buttons"
const int lenbuttons = LENGTH(buttons);

const Button *get_buttons() {
    return &buttons[0];
}

/* commands */
char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", "#000000", "-nf", blue, "-sb", blue, "-sf", black, "-h", dmenuh,  NULL };
const char *layoutmenu_cmd = "pOS-layoutmenu";
const char *termcmd[]  = { TERMINAL, NULL };

#include "keys"
const int lenkeys = LENGTH(keys);

const Key *get_keys() {
    return &keys[0];
}

#include "rules"
const int lenrules = LENGTH(rules);
const Rule *get_rules() {
    return &rules[0];
}
