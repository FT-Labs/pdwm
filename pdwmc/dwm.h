#pragma once

#include <X11/extensions/render.h>
#include <signal.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd)                                 \
	{                                          \
		.v = (const char *[])              \
		{                                  \
			"/bin/sh", "-c", cmd, NULL \
		}                                  \
	}
#define SPTAG(i) ((1 << LENGTH(tags)) << (i))

typedef struct Monitor Monitor;
typedef struct Client Client;
typedef struct Pertag Pertag;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	int issteam;
	unsigned int icw, ich;
	Picture icon;
	unsigned int tags;
	int isfixed, iscentered, isfloating, isurgent, neverfocus, oldstate, isfullscreen,
		isterminal, noswallow, managedsize, issticky;
	pid_t pid;
	Client *next;
	Client *snext;
	Client *curtagnext;
	Client *swallowing;
	Monitor *mon;
	Window win;
};

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by; /* bar geometry */
	int bleftend, brightstart; /* left bar end, right bar start pos */
	int mx, my, mw, mh; /* screen size */
	int wx, wy, ww, wh; /* window area  */
	unsigned int borderpx; /* Set border pixel */
	int gappih; /* horizontal gap between windows */
	int gappiv; /* vertical gap between windows */
	int gappoh; /* horizontal outer gaps */
	int gappov; /* vertical outer gaps */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
	Pertag *pertag;
};

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct Button {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Key {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char **cmd;
	const char *name;
} Config;

typedef struct Rule {
	char *class;
	char *instance;
	char *title;
	int tags;
	int isfloating;
	int isterminal;
	int iscentered;
	int noswallow;
	int managedsize;
	int monitor;
} Rule;

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum {
	SchemeNorm,
	SchemeSel,
	SchemeStatus,
	SchemeTagsSel,
	SchemeTagsNorm,
	SchemeInfoSel,
	SchemeInfoNorm,
	SchemeOptimal,
	SchemeCritical
}; /* color schemes */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum {
	NetSupported,
	NetWMName,
	NetWMIcon,
	NetWMState,
	NetWMCheck,
	NetSystemTray,
	NetSystemTrayOP,
	NetSystemTrayOrientation,
	NetSystemTrayOrientationHorz, /* System tray */
	NetWMFullscreen,
	NetActiveWindow,
	NetWMWindowType,
	NetWMWindowTypeDialog,
	NetWMSticky,
	NetClientList,
	NetWMDesktop,
	NetCurrentDesktop,
	NetCurrentMonCenter,
	NetLast
}; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum {
	ClkTagBar,
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkLast
}; /* clicks */
