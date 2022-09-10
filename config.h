/* See LICENSE file for copyright and license details. */
#pragma once
#include "pdwm/dwm.h"
#include <X11/XF86keysym.h>
/* Constants */
#define INC(X)                  ((X) + 2000)
#define TERMINAL "st"
#define TERMCLASS "St"

/* appearance */
const extern unsigned int borderpx;
const extern unsigned int snap;
const extern unsigned int gappih;
const extern unsigned int gappiv;
const extern unsigned int gappoh;
const extern unsigned int gappov;
const extern int swallowfloating;
const extern int smartgaps;
const extern int showbar;
const extern int topbar;
const extern int user_dh;
const extern int docklrmargin;
const extern int user_bh;
char extern dmenufont[];
char extern dmenuh[];

const extern char *fonts[];
const extern int lenfonts;

/* Default icon width, height, margin and delimiter width */
const extern unsigned int sb_icon_wh;
const extern unsigned int sb_icon_margin_x;
const extern unsigned int sb_delimiter_w;

/* Status bar x  y margin */
const extern unsigned int sb_padding_x;
const extern unsigned int sb_padding_y;

char extern *colors[9][3];

const extern char *physettings[];

const extern Config config[];
const extern int lenconfig;

typedef struct {
    const char *name;
    const void *cmd;
} Sp;

const extern char *spcmd1[];

const extern Sp scratchpads[1];

/* tagging */
const extern char *tags[10];

/* layout(s) */
const extern float mfact;
const extern int nmaster;
const extern int resizehints;
#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
const extern Layout layouts[10];

/* key definitions */
#define Win Mod4Mask
#define Alt Mod1Mask

/* commands */
char extern dmenumon[2];
const extern char *dmenucmd[];
const extern char *layoutmenu_cmd;

const extern char *termcmd[];

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
const extern Button buttons[];
const extern int lenbuttons;

const extern Key keys[];
const extern int lenkeys;
