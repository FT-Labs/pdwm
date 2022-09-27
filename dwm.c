/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <X11/X.h>
#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>
#include <X11/Xlib-xcb.h>
#include <xcb/res.h>

#include "drw.h"
#include "util.h"
#include "config.h"
#include "pdwm/dwm.h"

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define GETINC(X)               ((X) - 2000)
#define INC(X)                  ((X) + 2000)
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISINC(X)                ((X) > 1000 && (X) < 3000)
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define HIDDEN(C)               ((getstate(C->win) == IconicState))
#define PREVSEL                 3000
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define MOD(N,M)                ((N)%(M) < 0 ? (N)%(M) + (M) : (N)%(M))
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define NUMTAGS         (LENGTH(tags) + LENGTH(scratchpads))
#define TAGMASK         ((1 << NUMTAGS) - 1)
#define SPTAGMASK       (((1 << LENGTH(scratchpads))-1) << LENGTH(tags))
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)
#define TEXTW_SB(X)                (drw_fontset_getwidth(drw, (X)))
#define TRUNC(X,A,B)            (MAX((A), MIN((X), (B))))
#define TAGKEYS(KEY,TAG) \
    { Mod4Mask,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { Mod4Mask|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { Mod4Mask|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { Mod4Mask|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} }, \
    { Mod4Mask|Mod1Mask,               KEY,      swaptags,       {.ui = 1 << TAG} },
#define STACKKEYS(MOD,ACTION) \
    { MOD,  XK_j,   ACTION##stack,  {.i = INC(+1) } }, \
    { MOD,  XK_k,   ACTION##stack,  {.i = 0 } }, \
    { MOD,  XK_v,   ACTION##stack,  {.i = 0 } }, \

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_FOCUS_IN             4
#define XEMBED_MODALITY_ON         10
#define XEMBED_MAPPED              (1 << 0)
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2
#define VERSION_MAJOR               0
#define VERSION_MINOR               0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

static Window allbarwin[2];
static struct Stray {
    Window win;
    Client *icons;
} Systray;
static struct Stray *systray = &Systray;

const Rule *rules;
extern const Rule *get_rules();

static struct Rule defrules[] = {
{ TERMCLASS,  NULL,           NULL,            0,            0,           1,         0,         0,         0,         -1 },
{ NULL,       NULL,           "Event Tester",  0,            0,           0,         0,         1,         0,         -1 },
{ NULL,       "spterm",      NULL,             SPTAG(0),     1,           1,         0,         0,         0,         -1 },
{ NULL,       "physettings", NULL,             0,            1,           1,         1,         0,         0,         -1 },
{ NULL,       "physet-run",  NULL,             0,            1,           1,         1,         0,         0,         -1 },
{ NULL,       NULL,          "Pdwm Gui",             0,            1,           1,         1,         0,         0,         -1 },
};


extern const Button *get_buttons();
const Button *buttons;
extern const Key* get_keys();
const Key *keys;

const char **fonts;
extern const char **get_fonts();

/* variables */
static const char broken[] = "broken";
static char* sb_arr[10]; /* Array that holds name of pngs */
static int sb_tw;
static char stext[256];
static char rawstext[256];
static int dwmblockssig;
pid_t dwmblockspid = 0;
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh, blw = 0;      /* bar geometry */
static int lrpad;            /* sum of left and right padding for text */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast];
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Window root, wmcheckwin;

static xcb_connection_t *xcon;

/* function declarations */
static void autostart(void);
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void copyvalidchars(char *text, char *rawtext);
static Monitor *createmon(void);
void cyclelayout(const Arg *arg);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void freeicon(Client *c);
void focus(Client *c);
static void focusin(XEvent *e);
void focusmon(const Arg *arg);
void focusstack(const Arg *arg);
void hide(const Arg *arg);
static void hidewin(Client *c);
static Picture geticonprop(Window w, unsigned int *icw, unsigned int *ich);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
void swaptags(const Arg *arg);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
void layoutmenu(const Arg * arg);
void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
void monocle(Monitor *m);
static void motionnotify(XEvent *e);
void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *);
static int processrawtext(char* text);
static void propertynotify(XEvent *e);
void pushstack(const Arg *arg);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void scan(void);
static int sendevent(Window win, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setclienttagprop(Client *c);
static void setmonposcenter(Monitor *m);
void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
void setlayout(const Arg *arg);
void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void showwin(Client *c);
static void sigchld(int unused);
#ifndef __OpenBSD__
int getdwmblockspid();
void sigdwmblocks(const Arg *arg);
#endif
static void sighup(int unused);
static void sigterm(int unused);
void spawn(const Arg *arg);
static int stackpos(const Arg *arg);
void tag(const Arg *arg);
void tagmon(const Arg *arg);
void togglebar(const Arg *arg);
void togglefloating(const Arg *arg);
void togglescratch(const Arg *arg);
void togglesticky(const Arg *arg);
void togglefullscr(const Arg *arg);
void toggletag(const Arg *arg);
void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updatecurrentdesktop(Monitor *m);
static void updateclientlist(void);
static int updategeom(void);
static void updateicon(Client *c);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
void zoom(const Arg *arg);

static pid_t getparentprocess(pid_t p);
static int isdescprocess(pid_t p, pid_t c);
static Client *swallowingclient(Window w);
static Client *termforwin(const Client *c);
static pid_t winpid(Window w);
void defaultgaps(const Arg *arg);
void incrgaps(const Arg *arg);
void togglegaps(const Arg *arg);
void bstack(Monitor *m);
void centeredmaster(Monitor *m);
void centeredfloatingmaster(Monitor *m);
void deck(Monitor *m);
void dwindle(Monitor *m);
void fibonacci(Monitor *m, int s);
void spiral(Monitor *m);
void tile(Monitor *);
void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc);
void setgaps(int oh, int ov, int ih, int iv);
void shifttag(const Arg *arg);
void shiftview(const Arg *arg);

/* Systray function definitions */
static unsigned int getsystraywidth();
static void removesystrayicon(Client *i);
static void resizerequest(XEvent *e);
static void updatesystray(void);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);
static Client *wintosystrayicon(Window w);

static Monitor *mons, *selmon;
#include "vanitygaps.c" /* Needs selmon variable */

static const Key defkeys[] = {
    STACKKEYS(Mod4Mask,                          focus)
    STACKKEYS(Mod4Mask|ShiftMask,                push)
    TAGKEYS(            XK_1,       0)
    TAGKEYS(            XK_2,       1)
    TAGKEYS(            XK_3,       2)
    TAGKEYS(            XK_4,       3)
    TAGKEYS(            XK_5,       4)
    TAGKEYS(            XK_6,       5)
    TAGKEYS(            XK_7,       6)
    TAGKEYS(            XK_8,       7)
    TAGKEYS(            XK_9,       8)
};

struct Pertag {
    unsigned int curtag, prevtag; /* current and previous tag */
    int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
    float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
    unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
    const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
    int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

static void (*handler[]) (XEvent *) = {
    [ButtonPress] = buttonpress,
    [ClientMessage] = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify] = configurenotify,
    [DestroyNotify] = destroynotify,
    [EnterNotify] = enternotify,
    [Expose] = expose,
    [FocusIn] = focusin,
    [KeyPress] = keypress,
    [MappingNotify] = mappingnotify,
    [MapRequest] = maprequest,
    [MotionNotify] = motionnotify,
    [PropertyNotify] = propertynotify,
    [ResizeRequest] = resizerequest,
    [UnmapNotify] = unmapnotify
};


/* function implementations */
void
autostart(void)
{
    system("killall -q dwmblocks; dwmblocks &");
}


void
applyrules(Client *c)
{
    const char *class, *instance;
    unsigned int i;
    const Rule *r;
    Monitor *m;
    XClassHint ch = { NULL, NULL };

    /* rule matching */
    c->iscentered = c->iscentered ? 1 : 0;
    c->isfloating = c->isfloating ? 1 : 0;
    c->tags = 0;
    XGetClassHint(dpy, c->win, &ch);
    class    = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name  ? ch.res_name  : broken;

    if (strstr(class, "Steam") || strstr(class, "steam_app_"))
        c->issteam = 1;

    for (i = 0; i < LENGTH(defrules); i++) {
        r = &defrules[i];
        if ((!r->title || strstr(c->name, r->title))
        && (!r->class || strstr(class, r->class))
        && (!r->instance || strstr(instance, r->instance)))
        {
            c->iscentered = r->iscentered;
            c->isterminal = r->isterminal;
            c->isfloating = r->isfloating;
            c->noswallow  = r->noswallow;
            c->managedsize = r->managedsize;
            c->tags |= r->tags;
            if ((r->tags & SPTAGMASK) && r->isfloating) {
                c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
                c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
            }

            if (c->managedsize) {
                c->w = c->mon->ww / 2;
                c->h = c->mon->wh / 2;
            }

            for (m = mons; m && m->num != r->monitor; m = m->next);
            if (m)
                c->mon = m;
        }
    }

    for (i = 0; i < lenrules; i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title))
        && (!r->class || strstr(class, r->class))
        && (!r->instance || strstr(instance, r->instance)))
        {
            c->iscentered = r->iscentered;
            c->isterminal = r->isterminal;
            c->isfloating = r->isfloating;
            c->noswallow  = r->noswallow;
            c->managedsize = r->managedsize;
            c->tags |= r->tags;
            if ((r->tags & SPTAGMASK) && r->isfloating) {
                c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
                c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
            }

            if (c->managedsize) {
                c->w = c->mon->ww / 2;
                c->h = c->mon->wh / 2;
            }

            for (m = mons; m && m->num != r->monitor; m = m->next);
            if (m)
                c->mon = m;
        }
    }
    if (ch.res_class)
        XFree(ch.res_class);
    if (ch.res_name)
        XFree(ch.res_name);
    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : (c->mon->tagset[c->mon->seltags] & ~SPTAGMASK);
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
    int baseismin;
    Monitor *m = c->mon;

    /* set minimum possible */
    *w = MAX(1, *w);
    *h = MAX(1, *h);
    if (interact) {
        if (*x > sw)
            *x = sw - WIDTH(c);
        if (*y > sh)
            *y = sh - HEIGHT(c);
        if (*x + *w + 2 * c->bw < 0)
            *x = 0;
        if (*y + *h + 2 * c->bw < 0)
            *y = 0;
    } else {
        if (*x >= m->wx + m->ww)
            *x = m->wx + m->ww - WIDTH(c);
        if (*y >= m->wy + m->wh)
            *y = m->wy + m->wh - HEIGHT(c);
        if (*x + *w + 2 * c->bw <= m->wx)
            *x = m->wx;
        if (*y + *h + 2 * c->bw <= m->wy)
            *y = m->wy;
    }
    if (*h < bh)
        *h = bh;
    if (*w < bh)
        *w = bh;
    if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
        /* see last two sentences in ICCCM 4.1.2.3 */
        baseismin = c->basew == c->minw && c->baseh == c->minh;
        if (!baseismin) { /* temporarily remove base dimensions */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for aspect limits */
        if (c->mina > 0 && c->maxa > 0) {
            if (c->maxa < (float)*w / *h)
                *w = *h * c->maxa + 0.5;
            else if (c->mina < (float)*h / *w)
                *h = *w * c->mina + 0.5;
        }
        if (baseismin) { /* increment calculation requires this */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for increment value */
        if (c->incw)
            *w -= *w % c->incw;
        if (c->inch)
            *h -= *h % c->inch;
        /* restore base dimensions */
        *w = MAX(*w + c->basew, c->minw);
        *h = MAX(*h + c->baseh, c->minh);
        if (c->maxw)
            *w = MIN(*w, c->maxw);
        if (c->maxh)
            *h = MIN(*h, c->maxh);
    }
    return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Monitor *m)
{
    if (m) {
        updatecurrentdesktop(m);
        showhide(m->stack);
    }
    else for (m = mons; m; m = m->next)
        showhide(m->stack);
    if (m) {
        arrangemon(m);
        restack(m);
    } else for (m = mons; m; m = m->next)
        arrangemon(m);
}

void
arrangemon(Monitor *m)
{
    strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
    if (m->lt[m->sellt]->arrange)
        m->lt[m->sellt]->arrange(m);
}

void
attach(Client *c)
{
    c->next = c->mon->clients;
    c->mon->clients = c;
}

void
attachstack(Client *c)
{
    c->snext = c->mon->stack;
    c->mon->stack = c;
}

void
swallow(Client *p, Client *c)
{
    if (c->noswallow || c->isterminal)
        return;
    if (!swallowfloating && c->isfloating)
        return;

    detach(c);
    detachstack(c);

    setclientstate(c, WithdrawnState);
    XUnmapWindow(dpy, p->win);

    p->swallowing = c;
    c->mon = p->mon;

    Window w = p->win;
    p->win = c->win;
    c->win = w;
    updateicon(p);
    updatetitle(p);
    XMoveResizeWindow(dpy, p->win, p->x, p->y, p->w, p->h);
    arrange(p->mon);
    configure(p);
    updateclientlist();
}

void
unswallow(Client *c)
{
    c->win = c->swallowing->win;

    free(c->swallowing);
    c->swallowing = NULL;

    /* unfullscreen the client */
    setfullscreen(c, 0);
    updatetitle(c);
    arrange(c->mon);
    XMapWindow(dpy, c->win);
    XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
    setclientstate(c, NormalState);
    focus(NULL);
    arrange(c->mon);
}

int
processrawtext(char* text) {
    int x = 0, i;
    int s = strlen(text);

    for (i = 0; i<s && text[i] != '\0'; i++) {
        if (text[i] == '|') {
            /* If seperator and next char is not icon (between |0-9|, just 1 character) */
            if (!BETWEEN(text[i+1], '0', '9') || i+2 < s || text[i+2] != '|') {
                x += (sb_delimiter_w + sb_icon_margin_x * 2) / 2;
            }
            x -= TEXTW_SB("|");
        }
        else if (BETWEEN(text[i], '0', '9') && text[i+1] == '|' && (i == 0 || text[i-1] == '|' )) {
            /* If icon, delete seperator margin and add icon size + icon_x_margin */
            x += sb_icon_wh + sb_icon_margin_x;
            x -= (sb_delimiter_w + 2 * sb_icon_margin_x)/2;
            char tmp[] = {text[i], '\0'};
            x -= TEXTW_SB(tmp);
        }
        else if (text[i+1] == '|') {
            /* If next element is seperator add half width */
            x += (2 * sb_icon_margin_x + sb_delimiter_w) / 2;
        }
    }

    x += TEXTW_SB(text);
    return x;
}

void
buttonpress(XEvent *e)
{
    unsigned int i, x, click, occ = 0;
    Arg arg = {0};
    Client *c;
    Monitor *m;
    XButtonPressedEvent *ev = &e->xbutton;


    click = ClkRootWin;
    /* focus monitor if necessary */
    if ((m = wintomon(ev->window)) && m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        setmonposcenter(selmon);
        focus(NULL);
    }

    if (ev->window == selmon->barwin) {
        i = 0;
        x = sb_icon_wh + 2 * sb_delimiter_w;
        if (ev->x <= x) {
            if (ev->button == Button1) {
                Arg a = SHCMD("rofi -show drun");
                spawn(&a);
            } else if (ev->button == Button3) {
                Arg a = SHCMD("pOS-powermenu");
                spawn(&a);
            }
            return;
        }

        for (c = m->clients; c; c = c->next)
            occ |= c->tags == 255 ? 0 : c->tags;
        do {
            /* do not reserve space for vacant tags */
            if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
                continue;
            x += TEXTW(tags[i]);
        } while (ev->x >= x && ++i < LENGTH(tags));
        if (i < LENGTH(tags)) {
            click = ClkTagBar;
            arg.ui = 1 << i;
            goto execute_handler;
        } else if (ev->x < x + blw) {
            click = ClkLtSymbol;
            goto execute_handler;
        }

        x += blw;

        for (i=0; i<lenconfig; i++) {
            x += TEXTW(config[i].name) + sb_icon_wh + sb_icon_margin_x;

            if (ev->x < x) {
                Arg a;
                a.v = config[i].cmd;
                spawn(&a);
                return;
            }
        }
    } else if ((c = wintoclient(ev->window))) {
            focus(c);
            restack(selmon);
            XAllowEvents(dpy, ReplayPointer, CurrentTime);
            click = ClkClientWin;
    }  else if (ev->window == allbarwin[1]) {
        if (ev->x > (x = 0)) {
            click = ClkStatusText;

            char *text = rawstext;
            int i = -1;
            char ch;
            dwmblockssig = 0;
            while (text[++i]) {
                if ((unsigned char)text[i] < ' ') {

                    ch = text[i];
                    text[i] = '\0';
                    x += processrawtext(text);
                    text[i] = ch;
                    text += i+1;
                    i = -1;
                    if (x >= ev->x) break;
                    dwmblockssig = ch;
                }
            }
        } else
            click = ClkWinTitle;
    }

execute_handler:
    for (i = 0; i < lenbuttons; i++)
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void)
{
    xerrorxlib = XSetErrorHandler(xerrorstart);
    /* this causes an error if some other window manager is running */
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);
}

void
cleanup(void)
{
    Arg a = {.ui = ~0};
    Layout foo = { "", NULL };
    Monitor *m;
    size_t i;

    view(&a);
    selmon->lt[selmon->sellt] = &foo;
    for (m = mons; m; m = m->next)
        while (m->stack)
            unmanage(m->stack, 0);
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    while (mons)
        cleanupmon(mons);
    XUnmapWindow(dpy, allbarwin[0]);
    XUnmapWindow(dpy, allbarwin[1]);
    XDestroyWindow(dpy, allbarwin[0]);
    XDestroyWindow(dpy, allbarwin[1]);
    for (i = 0; i < CurLast; i++)
        drw_cur_free(drw, cursor[i]);
    for (i = 0; i < LENGTH(colors); i++)
        free(scheme[i]);
    XDestroyWindow(dpy, wmcheckwin);
    drw_free(drw);
    XSync(dpy, False);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void
cleanupmon(Monitor *mon)
{
    Monitor *m;

    if (mon == mons)
        mons = mons->next;
    else {
        for (m = mons; m && m->next != mon; m = m->next);
        m->next = mon->next;
    }

    XUnmapWindow(dpy, mon->barwin);
    XDestroyWindow(dpy, mon->barwin);
    free(mon);
}

void
clientmessage(XEvent *e)
{
    XWindowAttributes wa;
    XSetWindowAttributes swa;
    XClientMessageEvent *cme = &e->xclient;
    Client *c = wintoclient(cme->window);
    unsigned int i;
   	if (showsystray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
		/* add systray icons */
		if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
			if (!(c = (Client *)calloc(1, sizeof(Client))))
				die("fatal: could not malloc() %u bytes\n", sizeof(Client));
			if (!(c->win = cme->data.l[2])) {
				free(c);
				return;
			}
			c->mon = selmon;
			c->next = systray->icons;
			systray->icons = c;
			if (!XGetWindowAttributes(dpy, c->win, &wa)) {
				/* use sane defaults */
				wa.width = sb_icon_wh;
				wa.height = sb_icon_wh;
				wa.border_width = 0;
			}
            wa.width = 24, wa.height = 24;
			c->x = c->oldx = c->y = c->oldy = 0;
			c->w = c->oldw = wa.width;
			c->h = c->oldh = wa.height;
			c->oldbw = wa.border_width;
			c->bw = 0;
			c->isfloating = True;
			/* reuse tags field as mapped status */
			c->tags = 0;
			updatesizehints(c);
			updatesystrayicongeom(c, wa.width, wa.height);
			XAddToSaveSet(dpy, c->win);
			XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
			XReparentWindow(dpy, c->win, systray->win, 0, 0);
			/* use parents background color */
			swa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
			XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			/* FIXME not sure if I have to send these events, too */
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			XSync(dpy, False);
            drawbars();
			updatesystray();
			setclientstate(c, NormalState);
		}
		return;
	}

    if (!c)
        return;
    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen]
        || cme->data.l[2] == netatom[NetWMFullscreen])
            setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
    } else if (cme->message_type == netatom[NetActiveWindow]) {
        i = 0;
        do {
            if ((1 << i) & c->tags) {
                const Arg a = {.ui = (1 << i) };
                if (selmon->sel != c || HIDDEN(c)) {
                    selmon = c->mon;
                    setmonposcenter(selmon);
                    view(&a);
                    showwin(c);
                    focus(c);
                    if (!c->isfloating)
                        XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
                    restack(selmon);
                }
            }
        } while (i < LENGTH(tags) && !((1 << i++) & c->tags));
    }
}

void
configure(Client *c)
{
    XConfigureEvent ce;

    ce.type = ConfigureNotify;
    ce.display = dpy;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above = None;
    ce.override_redirect = False;
    XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e)
{
    Monitor *m;
    Client *c;
    XConfigureEvent *ev = &e->xconfigure;
    int dirty;

    /* TODO: updategeom handling sucks, needs to be simplified */
    if (ev->window == root) {
        dirty = (sw != ev->width || sh != ev->height);
        sw = ev->width;
        sh = ev->height;
        if (updategeom() || dirty) {
            drw_resize(drw, sw, bh);
            updatebars();
            for (m = mons; m; m = m->next) {
                for (c = m->clients; c; c = c->next)
                    if (c->isfullscreen)
                        resizeclient(c, m->mx, m->my, m->mw, m->mh);
                XMoveResizeWindow(dpy, m->barwin, m->wx + sb_padding_x, m->by, m->ww - 2 * sb_padding_x, bh);
            }
            focus(NULL);
            arrange(NULL);
        }
    }
}

void
configurerequest(XEvent *e)
{
    Client *c;
    Monitor *m;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;

    if ((c = wintoclient(ev->window))) {
        if (ev->value_mask & CWBorderWidth)
            c->bw = ev->border_width;
        else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
            m = c->mon;
            if (!c->issteam) {
                if (ev->value_mask & CWX) {
                    c->oldx = c->x;
                    c->x = m->mx + ev->x;
                }
                if (ev->value_mask & CWY) {
                    c->oldy = c->y;
                    c->y = m->my + ev->y;
                }
            }
            if (ev->value_mask & CWWidth) {
                c->oldw = c->w;
                c->w = ev->width;
            }
            if (ev->value_mask & CWHeight) {
                c->oldh = c->h;
                c->h = ev->height;
            }
            if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
                c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
            if ((c->y + c->h) > m->my + m->mh && c->isfloating)
                c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
            if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
                configure(c);
            if (ISVISIBLE(c))
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        } else
            configure(c);
    } else {
        wc.x = ev->x;
        wc.y = ev->y;
        wc.width = ev->width;
        wc.height = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling = ev->above;
        wc.stack_mode = ev->detail;
        XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    }
    XSync(dpy, False);
}

void
copyvalidchars(char *text, char *rawtext)
{
    int i = -1, j = 0;

    while(rawtext[++i]) {
        if ((unsigned char)rawtext[i] >= ' ') {
            text[j++] = rawtext[i];
        }
    }
    text[j] = '\0';
}

Monitor *
createmon(void)
{
    Monitor *m;
    unsigned int i;

    m = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact = mfact;
    m->nmaster = nmaster;
    m->showbar = showbar;
    m->topbar = topbar;
    m->borderpx = borderpx;
    m->gappih = gappih;
    m->gappiv = gappiv;
    m->gappoh = gappoh;
    m->gappov = gappov;
    m->lt[0] = &layouts[0];
    m->lt[1] = &layouts[1 % LENGTH(layouts)];
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
    m->pertag = ecalloc(1, sizeof(Pertag));
    m->pertag->curtag = m->pertag->prevtag = 1;

    for (i = 0; i <= LENGTH(tags); i++) {
        m->pertag->nmasters[i] = m->nmaster;
        m->pertag->mfacts[i] = m->mfact;

        m->pertag->ltidxs[i][0] = m->lt[0];
        m->pertag->ltidxs[i][1] = m->lt[1];
        m->pertag->sellts[i] = m->sellt;

        m->pertag->showbars[i] = m->showbar;
    }

    return m;
}

void
cyclelayout(const Arg *arg) {
    Layout *l;
    for(l = (Layout *)layouts; l != selmon->lt[selmon->sellt]; l++);
    if(arg->i > 0) {
        if(l->symbol && (l + 1)->symbol)
            setlayout(&((Arg) { .v = (l + 1) }));
        else
            setlayout(&((Arg) { .v = layouts }));
    } else {
        if(l != layouts && (l - 1)->symbol)
            setlayout(&((Arg) { .v = (l - 1) }));
        else
            setlayout(&((Arg) { .v = &layouts[LENGTH(layouts) - 2] }));
    }
}


void
destroynotify(XEvent *e)
{
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if ((c = wintoclient(ev->window)))
        unmanage(c, 1);
    else if ((c = wintosystrayicon(ev->window))) {
		removesystrayicon(c);
		updatesystray();
        drawbars();
	} else if ((c = swallowingclient(ev->window)))
        unmanage(c->swallowing, 1);
}

void
detach(Client *c)
{
    Client **tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
}

void
detachstack(Client *c)
{
    Client **tc, *t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;

    if (c == c->mon->sel) {
        for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
        c->mon->sel = t;
    }
}

Monitor *
dirtomon(int dir)
{
    Monitor *m = NULL;

    if (dir > 0) {
        if (!(m = selmon->next))
            m = mons;
    } else if (selmon == mons)
        for (m = mons; m->next; m = m->next);
    else
        for (m = mons; m->next != selmon; m = m->next);
    return m;
}

void
drawbar(Monitor *m)
{
    if (!m->barwin || !allbarwin[0] || !allbarwin[1])
        return;
    int x, w, y = 0, tw = 0, twtmp = 0, stw = 0;
    int boxs = drw->fonts->h / 9;
    int boxw = drw->fonts->h / 6 + 2;
    unsigned int i = 0, occ = 0, urg = 0;
    Client *c;

    if (showsystray)
        stw = getsystraywidth();

    /* Status bar is only drawn on selected monitor */
    if (m == selmon) {
        load_png_icons(drw, sb_icon_wh, sb_icon_wh);

        char stextcpy[256];
        strcpy(stextcpy, stext);
        sb_arr[i] = strtok(stextcpy, "|");

        while (sb_arr[i] != NULL)
        {
            if (strlen(sb_arr[i]) != 1) {
                tw += TEXTW_SB(sb_arr[i]);
                tw += sb_delimiter_w + 2 * sb_icon_margin_x;
            }
            else
                tw += sb_icon_wh + sb_icon_margin_x;
            sb_arr[++i] = strtok(NULL, "|");
        }


        i = 0;
        tw += 2 * sb_padding_x;
        sb_tw = twtmp = tw;

        /* Fill all bar with colorscheme first, some png might have empty locations */
        drw_setscheme(drw, scheme[SchemeInfoSel]);
        drw_rect(drw, m->ww - twtmp, y, twtmp + 3 * sb_padding_x + stw, bh, 1, 1);
        m->brightstart = m->ww - twtmp;
        XMoveResizeWindow(dpy, allbarwin[1], m->wx + m->brightstart - stw - sb_padding_x, m->by, stw + twtmp, bh);
        XMoveResizeWindow(dpy, systray->win, twtmp - sb_padding_x, 0, stw + 1, bh);
        twtmp -= sb_icon_margin_x;

        while (sb_arr[i] != NULL) {
            drw_setscheme(drw, scheme[SchemeInfoSel]);

            if (strlen(sb_arr[i]) == 1) {
                int idx = sb_arr[i++][0] - '0';
                drw_pic(drw, m->ww - twtmp, (bh - sb_icon_wh) / 2, sb_icon_wh, sb_icon_wh, None, idx);
                twtmp -= sb_icon_wh + sb_icon_margin_x;
                drw_text(drw, m->ww - twtmp, y, TEXTW_SB(sb_arr[i]), bh, 0, sb_arr[i], 0);
                twtmp -= TEXTW_SB(sb_arr[i]);
            } else {
                if (sb_arr[i][strlen(sb_arr[i]) - 1] == '%') {
                    char *start = sb_arr[i];

                    while (*start < 0)
                        start++;
                    int x = atoi(start);

                    if (x <= 30)
                        drw_setscheme(drw, scheme[SchemeCritical]);
                    else
                        drw_setscheme(drw, scheme[SchemeOptimal]);
                }
                drw_text(drw, m->ww - twtmp, y, TEXTW_SB(sb_arr[i]), bh, 0, sb_arr[i], 0);
                twtmp -= TEXTW_SB(sb_arr[i]);
            }

            /* Below draws seperators */
            if (sb_arr[i+1] != NULL || showsystray) {
                twtmp -= sb_icon_margin_x;
                drw_setscheme(drw, scheme[SchemeSel]);
                drw_rect(drw, m->ww - twtmp + sb_delimiter_w / 4, y, sb_delimiter_w / 2, bh , 1, 0);
                drw_rect(drw, m->ww - twtmp, y + bh/10, sb_delimiter_w, bh - 2 * bh/10, 1, 0);
                twtmp -= sb_delimiter_w + sb_icon_margin_x;
            }
            i++;
        }
    }

    for (c = m->clients; c; c = c->next) {
        occ |= c->tags == 255 ? 0 : c->tags;
        if (c->isurgent)
            urg |= c->tags;
    }

    x = 0;
    /* Draw logo offset with margin */
    drw_setscheme(drw, scheme[SchemeTagsSel]);
    drw_rect(drw, 0, y, 2 * sb_delimiter_w + sb_icon_wh, bh, 1, 0);
    drw_pic(drw, sb_delimiter_w, y + (bh - sb_icon_wh) / 2, sb_icon_wh, sb_icon_wh, None, 0);
    x += sb_icon_wh + 2 * sb_delimiter_w;


    for (i = 0; i < LENGTH(tags); i++) {
        /* do not draw vacant tags */
        if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
            continue;

        w = TEXTW(tags[0]);
        drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeTagsSel : SchemeTagsNorm]);
        drw_text(drw, x, y, w, bh, lrpad / 2, tags[i], urg & 1 << i);
        x += w;
    }
    w = blw = TEXTW(m->ltsymbol);
    drw_setscheme(drw, scheme[SchemeOptimal]);
    x = drw_text(drw, x, y, w, bh, lrpad / 2, m->ltsymbol, 0);

    for (i=0; i<lenconfig; i++) {
        w = strcmp(config[i].name, "") ? TEXTW(config[i].name) : 0;
        drw_setscheme(drw, scheme[SchemeTagsNorm]);
        drw_rect(drw, x, y, w + sb_icon_wh + sb_icon_margin_x, bh, 1, 1);
        drw_text(drw, x, y, w, bh, lrpad / 2, config[i].name, 0);
        drw_pic(drw, x + w, (bh - sb_icon_wh) / 2, sb_icon_wh, sb_icon_wh, None, i+1);
        w += sb_icon_wh + sb_icon_margin_x;
        x += w;
    }

    XMoveResizeWindow(dpy, m->barwin, m->wx + sb_padding_x, m->by, x, bh);
    m->bleftend = x;

    if ((w = m->ww - tw - x - stw - 2 * sb_padding_x) > bh) {
        int s;
        if (m->sel && m == selmon) {
            c = m->sel;
            drw_setscheme(drw, scheme[SchemeStatus]);
            s = drw_text(drw, x, 0, MIN(w, TEXTW(c->name) + (c->icon ? c->icw + sb_icon_margin_x : 0) + lrpad), bh, lrpad / 2 + (c->icon ? c->icw + sb_icon_margin_x : 0), c->name, HIDDEN(c) ? 1 : 0) - x;
            if (c->icon) drw_pic(drw, x + lrpad / 2, (bh - c->ich) / 2, c->icw, c->ich, c->icon, -1);
            if (c->isfloating)
                drw_rect(drw, x + boxs, boxs, boxw, boxw, c->isfixed, 0);
            XMoveResizeWindow(dpy, allbarwin[0], m->wx + (m->bleftend + m->brightstart - s - stw - sb_padding_x) / 2 + 2 * sb_padding_x, m->by, s - 2 * sb_padding_x, bh);
            drw_map(drw, allbarwin[0], m->bleftend, 0, m->ww, bh);
        } else if (!m->sel && m == selmon) {
            XMoveWindow(dpy, allbarwin[0], m->wx + m->ww / 2, -2 * (bh + sb_padding_y));
        }
    }
    drw_map(drw, m->barwin, 0, 0, m->ww, bh);
    if (m == selmon)
        drw_map(drw, allbarwin[1], m->brightstart, 0, m->ww, bh);
}

void
drawbars(void)
{
    Monitor *m;

    for (m = mons; m; m = m->next)
        drawbar(m);
}

void
enternotify(XEvent *e)
{
    Client *c;
    Monitor *m;
    XCrossingEvent *ev = &e->xcrossing;

    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
        return;
    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);
    if (m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        setmonposcenter(selmon);
    } else if (!c || c == selmon->sel)
        return;
    focus(c);
}

void
expose(XEvent *e)
{
    Monitor *m;
    XExposeEvent *ev = &e->xexpose;

    if (ev->count == 0 && (m = wintomon(ev->window))) {
        drawbar(m);
        updatesystray();
    }
}

void
focus(Client *c)
{
    if (!c || !ISVISIBLE(c)) {
        for (c = selmon->stack; c && (!ISVISIBLE(c) || (c->issticky && !selmon->sel->issticky)); c = c->snext);
        /* No windows found; check for available stickies */
        if (!c)
            for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
    }
    if (selmon->sel && selmon->sel != c)
        unfocus(selmon->sel, 0);
    if (c) {
        if (c->mon != selmon) {
            selmon = c->mon;
        }
        if (c->isurgent)
            seturgent(c, 0);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, 1);
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
    selmon->sel = c;
    setmonposcenter(selmon);
    drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
    XFocusChangeEvent *ev = &e->xfocus;

    if (selmon->sel && ev->window != selmon->sel->win) {
        setfocus(selmon->sel);
    }
}

void
focusmon(const Arg *arg)
{
    Monitor *m;

    if (!mons->next)
        return;
    if ((m = dirtomon(arg->i)) == selmon)
        return;
    unfocus(selmon->sel, 0);
    selmon = m;
    setmonposcenter(selmon);
    focus(NULL);

    if(selmon->sel)
        XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, selmon->sel->w/2, selmon->sel->h/2);
}

void
focusstack(const Arg *arg)
{
    if (!selmon->sel || !selmon->clients)
        return;
    Client *c = NULL, *i;

    if (arg->i > 0) {
        if (selmon->sel)
            for (c = selmon->sel->next; (c && (!ISVISIBLE(c) || HIDDEN(c))); c = c->next);
        if (!c)
            for (c = selmon->clients; c && (!ISVISIBLE(c) || HIDDEN(c)); c = c->next);
    } else {
        if (selmon->sel) {
            for (i = selmon->clients; i != selmon->sel; i = i->next)
                if (ISVISIBLE(i) && !HIDDEN(i))
                    c = i;
        }
        if (!c)
            for (; i; i = i->next )
                if (ISVISIBLE(i) && !HIDDEN(i))
                    c = i;
    }

    if (c) {
        focus(c);
        restack(selmon);
        XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
    }
}

Atom
getatomprop(Client *c, Atom prop)
{
    int di;
    unsigned long dl;
    unsigned char *p = NULL;
    Atom da, atom = None;

    Atom req = XA_ATOM;
    if (prop == xatom[XembedInfo])
        req = xatom[XembedInfo];

    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req,
        &da, &di, &dl, &dl, &p) == Success && p) {
        atom = *(Atom *)p;
        if (da == xatom[XembedInfo] && dl == 2)
            atom = ((Atom *)p)[1];
        XFree(p);
    }
    return atom;
}

unsigned int
getsystraywidth()
{
	unsigned int w = 0;
	Client *i;
	if(showsystray)
		for(i = systray->icons; i; w += i->w + sb_delimiter_w, i = i->next) ;
	return w;
}


#ifndef __OpenBSD__
int
getdwmblockspid()
{
    char buf[16];
    FILE *fp = popen("pidof -s dwmblocks", "r");
    fgets(buf, sizeof(buf), fp);
    pid_t pid = strtoul(buf, NULL, 10);
    pclose(fp);
    dwmblockspid = pid;
    return pid != 0 ? 0 : -1;
}
#endif

int
getrootptr(int *x, int *y)
{
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w)
{
    int format;
    long result = -1;
    unsigned char *p = NULL;
    unsigned long n, extra;
    Atom real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
        &real, &format, &n, &extra, (unsigned char **)&p) != Success)
        return -1;
    if (n != 0)
        result = *p;
    XFree(p);
    return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
    char **list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return 0;
    text[0] = '\0';
    if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
        return 0;
    if (name.encoding == XA_STRING)
        strncpy(text, (char *)name.value, size - 1);
    else {
        if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
            strncpy(text, *list, size - 1);
            XFreeStringList(list);
        }
    }
    text[size - 1] = '\0';
    XFree(name.value);
    return 1;
}

void
grabbuttons(Client *c, int focused)
{
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (!focused)
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
                BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
        for (i = 0; i < lenbuttons; i++)
            if (buttons[i].click == ClkClientWin)
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabButton(dpy, buttons[i].button,
                        buttons[i].mask | modifiers[j],
                        c->win, False, BUTTONMASK,
                        GrabModeAsync, GrabModeSync, None, None);
    }
}

void
grabkeys(void)
{
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        KeyCode code;

        XUngrabKey(dpy, AnyKey, AnyModifier, root);

        for (i = 0; i < MAX(lenkeys, LENGTH(defkeys)); i++) {
            if (i < lenkeys)
                if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
                    for (j = 0; j < LENGTH(modifiers); j++)
                        XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
                            True, GrabModeAsync, GrabModeAsync);
            if (i < LENGTH(defkeys))
            if ((code = XKeysymToKeycode(dpy, defkeys[i].keysym)))
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabKey(dpy, code, defkeys[i].mod | modifiers[j], root,
                        True, GrabModeAsync, GrabModeAsync);
        }
    }
}

void
hide(const Arg *arg)
{
    hidewin(selmon->sel);
    arrange(selmon);
    focusstack(arg);
}

void
hidewin(Client *c)
{
    if (!c || HIDDEN(c))
        return;
    Window w = c->win;
    char name[264] = "[HIDDEN]";
    strcpy((name+8), c->name);
    XChangeProperty(dpy, w, netatom[NetWMName], XInternAtom(dpy, "UTF8_STRING", False), 8,
        PropModeReplace, (unsigned char *) name, strlen(name));
    static XWindowAttributes ra, ca;
    XGrabServer(dpy);
    XGetWindowAttributes(dpy, root, &ra);
    XGetWindowAttributes(dpy, w, &ca);
    /* Prevent UnmapNotify events */
    XSelectInput(dpy, root, ra.your_event_mask & ~SubstructureNotifyMask);
    XSelectInput(dpy, w, ca.your_event_mask & ~StructureNotifyMask);
    XUnmapWindow(dpy, w);
    setclientstate(c, IconicState);
    XSelectInput(dpy, root, ra.your_event_mask);
    XSelectInput(dpy, w, ca.your_event_mask);
    XUngrabServer(dpy);
}


void
incnmaster(const Arg *arg)
{
    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
    arrange(selmon);
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
        && unique[n].width == info->width && unique[n].height == info->height)
            return 0;
    return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e)
{
    unsigned int i;
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    for (i = 0; i < MAX(lenkeys, LENGTH(defkeys)); i++) {
        if (i < lenkeys)
            if (keysym == keys[i].keysym
            && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
            && keys[i].func)
                keys[i].func(&(keys[i].arg));
        if (i < LENGTH(defkeys))
            if (keysym == defkeys[i].keysym
            && CLEANMASK(defkeys[i].mod) == CLEANMASK(ev->state)
            && defkeys[i].func)
                defkeys[i].func(&(defkeys[i].arg));
    }
}

void
killclient(const Arg *arg)
{
    if (!selmon->sel)
        return;
    if (!sendevent(selmon->sel->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        freeicon(selmon->sel);
        XKillClient(dpy, selmon->sel->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

void layoutmenu(const Arg *arg)
{
    FILE *p;
    char c[3], *s;
    int i;

    if(!(p = popen(layoutmenu_cmd,"r")))
        return;
    s = fgets(c,sizeof(c),p);
    pclose(p);

    if(!s || *s == '\0')
        return;

    i = atoi(c);
    setlayout(&((Arg) {.v = &layouts[i]}));
}

void
manage(Window w, XWindowAttributes *wa)
{
    Client *c, *t = NULL, *term = NULL;
    Window trans = None;
    XWindowChanges wc;

    c = ecalloc(1, sizeof(Client));
    c->win = w;
    c->pid = winpid(w);
    /* geometry */
    c->x = c->oldx = wa->x;
    c->y = c->oldy = wa->y;
    c->w = c->oldw = wa->width;
    c->h = c->oldh = wa->height;
    c->oldbw = wa->border_width;

    updateicon(c);
    updatetitle(c);
    updatesizehints(c);
    updatewmhints(c);
    if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
        c->mon = t->mon;
        c->tags = t->tags;
    } else {
        c->mon = selmon;
        applyrules(c);
        term = termforwin(c);
    }
    updatewindowtype(c);

    {
        int format;
        unsigned long *data, n, extra;
        Monitor *m;
        Atom atom;

        if (XGetWindowProperty(dpy, c->win, netatom[NetWMDesktop], 0L, 2L, False, XA_CARDINAL,
                &atom, &format, &n, &extra, (unsigned char **)&data) == Success && n == 2)
        {

            c->tags = *data;

            for (m = mons; m; m = m->next)
            {
                if (m->num == *(data+1))
                {
                    c->mon = m;
                    break;
                }
            }
        }
        if (n > 0)
            XFree(data);
    }
    setclienttagprop(c);

    if (!c->iscentered) {
        if (c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
            c->x = c->mon->mx + c->mon->mw - WIDTH(c);
        if (c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
            c->y = c->mon->my + c->mon->mh - HEIGHT(c);
        c->x = MAX(c->x, c->mon->mx);
        /* only fix client y-offset, if the client center might cover the bar */
        c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
            && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);
    } else {
            c->x = c->mon->wx + (c->mon->ww - WIDTH(c)) / 2;
            c->y = c->mon->wy + (c->mon->wh - HEIGHT(c)) / 2;
    }

    if (!c->isfullscreen) {
        c->bw = c->mon->borderpx;
        wc.border_width = c->bw;
    } else {
        c->bw = 0;
        wc.border_width = 0;
    }
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
    configure(c); /* propagates border_width, if size doesn't change */
    XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    grabbuttons(c, 0);
    if (!c->isfloating)
        c->isfloating = c->oldstate = trans != None || c->isfixed;
    if (c->isfloating)
        XRaiseWindow(dpy, c->win);
    attach(c);
    attachstack(c);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
        (unsigned char *) &(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
    if (!HIDDEN(c))
        setclientstate(c, NormalState);
    if (c->mon->sel && c->mon->sel->isfullscreen) {
        XMapWindow(dpy, c->win);
        return;
    }
    if (c->mon == selmon)
        unfocus(selmon->sel, 0);
    c->mon->sel = c;
    if (!HIDDEN(c)) {
        arrange(c->mon);
        XMapWindow(dpy, c->win);
        if (term)
            swallow(term, c);
    }
    focus(NULL);
}

void
mappingnotify(XEvent *e)
{
    XMappingEvent *ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard)
        grabkeys();
}

void
maprequest(XEvent *e)
{
    static XWindowAttributes wa;
    XMapRequestEvent *ev = &e->xmaprequest;
    Client *i;
	if ((i = wintosystrayicon(ev->window))) {
		sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
		updatesystray();
	}

    if (!XGetWindowAttributes(dpy, ev->window, &wa))
        return;
    if (wa.override_redirect)
        return;
    if (!wintoclient(ev->window))
        manage(ev->window, &wa);
}

void
monocle(Monitor *m)
{
    unsigned int n;
    int oh, ov, ih, iv;
    Client *c;

    getgaps(m, &oh, &ov, &ih, &iv, &n);

    if (n > 0) /* override layout symbol */
        snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
    for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
        resize(c, m->wx + ov, m->wy + oh, m->ww - 2 * c->bw - 2 * ov, m->wh - 2 * c->bw - 2 * oh, 0);
}

void
motionnotify(XEvent *e)
{
    static Monitor *mon = NULL;
    Monitor *m;
    XMotionEvent *ev = &e->xmotion;

    if (ev->window != root)
        return;
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    mon = m;
}

void
movemouse(const Arg *arg)
{
    int x, y, ocx, ocy, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;
    Time lasttime = 0;

    if (!(c = selmon->sel))
        return;
    if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
        return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
        None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
        return;
    if (!getrootptr(&x, &y))
        return;
    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            if ((ev.xmotion.time - lasttime) <= (1000 / 280))
                continue;
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 280))
                continue;
            lasttime = ev.xmotion.time;

            nx = ocx + (ev.xmotion.x - x);
            ny = ocy + (ev.xmotion.y - y);
            if (abs(selmon->wx - nx) < snap)
                nx = selmon->wx;
            else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
                nx = selmon->wx + selmon->ww - WIDTH(c);
            if (abs(selmon->wy - ny) < snap)
                ny = selmon->wy;
            else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
                ny = selmon->wy + selmon->wh - HEIGHT(c);
            if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
            && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
                togglefloating(NULL);
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, nx, ny, c->w, c->h, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    XUngrabPointer(dpy, CurrentTime);
    lasttime = ev.xmotion.time;
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

Client *
nexttiled(Client *c)
{
    for (; c && (c->isfloating || !ISVISIBLE(c) || HIDDEN(c)); c = c->next);
    return c;
}

void
pop(Client *c)
{
    detach(c);
    attach(c);
    focus(c);
    arrange(c->mon);
}

void
pushstack(const Arg *arg) {
    int i = stackpos(arg);
    Client *sel = selmon->sel, *c, *p;

    if(i < 0 || !sel)
        return;
    else if(i == 0) {
        detach(sel);
        attach(sel);
    }
    else {
        for(p = NULL, c = selmon->clients; c; p = c, c = c->next)
            if(!(i -= (ISVISIBLE(c) && c != sel)))
                break;
        c = c ? c : p;
        detach(sel);
        sel->next = c->next;
        c->next = sel;
    }
    arrange(selmon);
}

static uint32_t prealpha(uint32_t p) {
    uint8_t a = p >> 24u;
    uint32_t rb = (a * (p & 0xFF00FFu)) >> 8u;
    uint32_t g = (a * (p & 0x00FF00u)) >> 8u;
    return (rb & 0xFF00FFu) | (g & 0x00FF00u) | (a << 24u);
}

Picture
geticonprop(Window win, unsigned int *picw, unsigned int *pich)
{
    int format;
    unsigned long n, extra, *p = NULL;
    Atom real;

    if (XGetWindowProperty(dpy, win, netatom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType,
                           &real, &format, &n, &extra, (unsigned char **)&p) != Success)
        return None;
    if (n == 0 || format != 32) { XFree(p); return None; }

    unsigned long *bstp = NULL;
    uint32_t w, h, sz;
    {
        unsigned long *i; const unsigned long *end = p + n;
        uint32_t bstd = UINT32_MAX, d, m;
        for (i = p; i < end - 1; i += sz) {
            if ((w = *i++) >= 16384 || (h = *i++) >= 16384) { XFree(p); return None; }
            if ((sz = w * h) > end - i) break;
            if ((m = w > h ? w : h) >= sb_icon_wh && (d = m - sb_icon_wh) < bstd) { bstd = d; bstp = i; }
        }
        if (!bstp) {
            for (i = p; i < end - 1; i += sz) {
                if ((w = *i++) >= 16384 || (h = *i++) >= 16384) { XFree(p); return None; }
                if ((sz = w * h) > end - i) break;
                if ((d = sb_icon_wh - (w > h ? w : h)) < bstd) { bstd = d; bstp = i; }
            }
        }
        if (!bstp) { XFree(p); return None; }
    }

    if ((w = *(bstp - 2)) == 0 || (h = *(bstp - 1)) == 0) { XFree(p); return None; }

    uint32_t icw, ich;
    if (w <= h) {
        ich = sb_icon_wh; icw = w * sb_icon_wh / h;
        if (icw == 0) icw = 1;
    }
    else {
        icw = sb_icon_wh; ich = h * sb_icon_wh / w;
        if (ich == 0) ich = 1;
    }
    *picw = icw; *pich = ich;

    uint32_t i, *bstp32 = (uint32_t *)bstp;
    for (sz = w * h, i = 0; i < sz; ++i) bstp32[i] = prealpha(bstp[i]);

    Picture ret = drw_picture_create_resized(drw, (char *)bstp, w, h, icw, ich);
    XFree(p);

    return ret;
}


void
propertynotify(XEvent *e)
{
    Client *c;
    Window trans;
    XPropertyEvent *ev = &e->xproperty;

   	if ((c = wintosystrayicon(ev->window))) {
		if (ev->atom == XA_WM_NORMAL_HINTS) {
			updatesizehints(c);
			updatesystrayicongeom(c, c->w, c->h);
		}
		else
			updatesystrayiconstate(c, ev);
		updatesystray();
	}

    if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
        updatestatus();
    } else if (ev->state == PropertyDelete) {
        return; /* ignore */
    } else if ((c = wintoclient(ev->window))) {
        switch(ev->atom) {
        default: break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
                (c->isfloating = (wintoclient(trans)) != NULL))
                arrange(c->mon);
            break;
        case XA_WM_NORMAL_HINTS:
            updatesizehints(c);
            break;
        case XA_WM_HINTS:
            updatewmhints(c);
            drawbars();
            break;
        }
        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            updateicon(c);
            updatetitle(c);
            if (c == c->mon->sel)
                drawbar(c->mon);
        }
        if (ev->atom == netatom[NetWMWindowType])
            updatewindowtype(c);
    }
}

void
removesystrayicon(Client *i)
{
	Client **ii;

	if (!showsystray || !i)
		return;
	for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next);
	if (ii)
		*ii = i->next;
	free(i);
}

void
resizerequest(XEvent *e)
{
	XResizeRequestEvent *ev = &e->xresizerequest;
	Client *i;

	if ((i = wintosystrayicon(ev->window))) {
		updatesystrayicongeom(i, i->w, i->h);
		updatesystray();
	}
}

void
quit(const Arg *arg)
{
    if(arg->i) restart = 1;
    running = 0;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
    Monitor *m, *r = selmon;
    int a, area = 0;

    for (m = mons; m; m = m->next)
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r = m;
        }
    return r;
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
    if (applysizehints(c, &x, &y, &w, &h, interact))
        resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
    XWindowChanges wc;

    c->oldx = c->x; c->x = wc.x = x;
    c->oldy = c->y; c->y = wc.y = y;
    c->oldw = c->w; c->w = wc.width = w;
    c->oldh = c->h; c->h = wc.height = h;
    wc.border_width = c->bw;
    XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
    configure(c);
    XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
    int ocx, ocy, nw, nh;
    Client *c;
    Monitor *m;
    XEvent ev;
    Time lasttime = 0;

    if (!(c = selmon->sel))
        return;
    if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
        return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
        None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
        return;
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 280))
                continue;
            lasttime = ev.xmotion.time;

            nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
            nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
            if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
            && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
            {
                if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
                && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                    togglefloating(NULL);
            }
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, c->x, c->y, nw, nh, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    XUngrabPointer(dpy, CurrentTime);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

void
restack(Monitor *m)
{
    Client *c;
    XEvent ev;
    XWindowChanges wc;

    drawbar(m);
    if (!m->sel)
        return;
    if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
        XRaiseWindow(dpy, m->sel->win);
    if (m->lt[m->sellt]->arrange) {
        wc.stack_mode = Below;
        wc.sibling = m->barwin;
        for (c = m->stack; c; c = c->snext)
            if (!c->isfloating && ISVISIBLE(c) && !HIDDEN(c)) {
                XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
                wc.sibling = c->win;
            }
    }
    XSync(dpy, False);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void)
{
    XEvent ev;
    /* main event loop */
    XSync(dpy, False);
    while (running && !XNextEvent(dpy, &ev))
        if (handler[ev.type])
            handler[ev.type](&ev); /* call handler */
}

void
scan(void)
{
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa)
            || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
                manage(wins[i], &wa);
        }
        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa))
                continue;
            if (XGetTransientForHint(dpy, wins[i], &d1)
            && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }
        if (wins)
            XFree(wins);
    }
}

void
sendmon(Client *c, Monitor *m)
{
    if (c->mon == m)
        return;
    unfocus(c, 1);
    detach(c);
    detachstack(c);
    c->mon = m;
    c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
    attach(c);
    attachstack(c);
    setclienttagprop(c);
    focus(NULL);
    arrange(NULL);
}


void
setclientstate(Client *c, long state)
{
    long data[] = { state, None };

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
        PropModeReplace, (unsigned char *)data, 2);
}

void
setclienttagprop(Client *c)
{
    long data[] = { (long) c->tags, (long) c->mon->num };
    XChangeProperty(dpy, c->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
            PropModeReplace, (unsigned char *) data, 2);
}

void
setmonposcenter(Monitor *m)
{
    long data[] = { m->mx + m->mw / 2, m->my + m->mh / 2 };
    XChangeProperty(dpy, root, netatom[NetCurrentMonCenter], XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *) data, 2);
}

int
sendevent(Window win, Atom proto, int m, long d0, long d1, long d2, long d3, long d4)
{
    int n;
    Atom *protocols, mt;
    int exists = 0;
    XEvent ev;

   if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
		mt = wmatom[WMProtocols];
		if (XGetWMProtocols(dpy, win, &protocols, &n)) {
			while (!exists && n--)
				exists = protocols[n] == proto;
			XFree(protocols);
		}
	} else {
		exists = True;
		mt = proto;
    }
    if (exists) {
        ev.type = ClientMessage;
        ev.xclient.window = win;
        ev.xclient.message_type = mt;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = d0;
		ev.xclient.data.l[1] = d1;
		ev.xclient.data.l[2] = d2;
		ev.xclient.data.l[3] = d3;
		ev.xclient.data.l[4] = d4;
		XSendEvent(dpy, win, False, m, &ev);
    }
    return exists;
}

void
setfocus(Client *c)
{
    if (!c->neverfocus) {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow],
            XA_WINDOW, 32, PropModeReplace,
            (unsigned char *) &(c->win), 1);
    }
    if (c->issteam)
        setclientstate(c, NormalState);
    sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void
setfullscreen(Client *c, int fullscreen)
{
    if (fullscreen && !c->isfullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
            PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
        c->isfullscreen = 1;
        c->oldstate = c->isfloating;
        c->oldbw = c->bw;
        c->bw = 0;
        c->isfloating = 1;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    } else if (!fullscreen && c->isfullscreen){
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
            PropModeReplace, (unsigned char*)0, 0);
        c->isfullscreen = 0;
        c->isfloating = c->oldstate;
        c->bw = c->oldbw;
        c->x = c->oldx;
        c->y = c->oldy;
        c->w = c->oldw;
        c->h = c->oldh;
        resizeclient(c, c->x, c->y, c->w, c->h);
        arrange(c->mon);
    }
}

int
stackpos(const Arg *arg) {
    int n, i;
    Client *c, *l;

    if(!selmon->clients)
        return -1;

    if(arg->i == PREVSEL) {
        for(l = selmon->stack; l && (!ISVISIBLE(l) || l == selmon->sel); l = l->snext);
        if(!l)
            return -1;
        for(i = 0, c = selmon->clients; c != l; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
        return i;
    }
    else if(ISINC(arg->i)) {
        if(!selmon->sel)
            return -1;
        for(i = 0, c = selmon->clients; c != selmon->sel; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
        for(n = i; c; n += ISVISIBLE(c) ? 1 : 0, c = c->next);
        return MOD(i + GETINC(arg->i), n);
    }
    else if(arg->i < 0) {
        for(i = 0, c = selmon->clients; c; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
        return MAX(i + arg->i, 0);
    }
    else
        return arg->i;
}

void
setlayout(const Arg *arg)
{
    if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
    if (arg && arg->v)
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
    strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
    if (selmon->sel)
        arrange(selmon);
    else
        drawbar(selmon);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
    float f;

    if (!arg || !selmon->lt[selmon->sellt]->arrange)
        return;
    f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
    if (f < 0.05 || f > 0.95)
        return;
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
    arrange(selmon);
}

void
setup(void)
{
    int i;
    XSetWindowAttributes wa;
    Atom utf8string;

    /* clean up any zombies immediately */
    sigchld(0);

    signal(SIGHUP, sighup);
    signal(SIGTERM, sigterm);

    /* init screen */
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = RootWindow(dpy, screen);
    drw = drw_create(dpy, screen, root, sw, sh);
    if (!drw_fontset_create(drw, fonts, lenfonts))
        die("no fonts could be loaded.");
    lrpad = drw->fonts->h;
    bh = user_bh ? user_bh : drw->fonts->h+2;
    updategeom();
    /* init atoms */
    utf8string = XInternAtom(dpy, "UTF8_STRING", False);
    wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
    wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
    netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    netatom[NetWMIcon] = XInternAtom(dpy, "_NET_WM_ICON", False);
    netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
    netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
    netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
    netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    netatom[NetWMDesktop] = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
    netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    netatom[NetCurrentDesktop] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
    netatom[NetCurrentMonCenter] = XInternAtom(dpy, "_NET_CURRENT_MON_CENTER", False);
    netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetSystemTray] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
	netatom[NetSystemTrayOP] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
	netatom[NetSystemTrayOrientation] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
	netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
    xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);


    /* init cursors */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove] = drw_cur_create(drw, XC_fleur);
    /* init appearance */
    scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
    for (i = 0; i < LENGTH(colors); i++)
        scheme[i] = drw_scm_create(drw, colors[i], 3);
    /* init bars */
    updatebars();
    updatesystray();
    updatestatus();
    /* supporting window for NetWMCheck */
    wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char *) &wmcheckwin, 1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
        PropModeReplace, (unsigned char *) "dwm", 3);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char *) &wmcheckwin, 1);
    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
        PropModeReplace, (unsigned char *) netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetWMDesktop]);
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    /* select events */
    wa.cursor = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
        |ButtonPressMask|PointerMotionMask|EnterWindowMask
        |LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);
    grabkeys();
    focus(NULL);
}


void
seturgent(Client *c, int urg)
{
    XWMHints *wmh;

    c->isurgent = urg;
    if (!(wmh = XGetWMHints(dpy, c->win)))
        return;
    wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);
}

void
showhide(Client *c)
{
    if (!c)
        return;
    if (ISVISIBLE(c) && !HIDDEN(c)) {
        if ((c->tags & SPTAGMASK) && c->isfloating) {
            c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
            c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
        }
        /* show clients top down */
        XMoveWindow(dpy, c->win, c->x, c->y);
        if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
            resize(c, c->x, c->y, c->w, c->h, 0);
        showhide(c->snext);
    } else {
        /* hide clients bottom up */
        showhide(c->snext);
        XMoveWindow(dpy, c->win, c->mon->wx + c->mon->ww / 2 - WIDTH(c) / 2, - (HEIGHT(c) * 3) / 2);
    }
}

void
showwin(Client *c)
{
    if (!c || !HIDDEN(c))
        return;
    XMapWindow(dpy, c->win);
    setclientstate(c, NormalState);
    XChangeProperty(dpy, c->win, netatom[NetWMName], XInternAtom(dpy, "UTF8_STRING", False), 8,
        PropModeReplace, (unsigned char *) (c->name+8), strlen(c->name+8));
    arrange(c->mon);
}

void
sigchld(int unused)
{
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
        die("can't install SIGCHLD handler:");
    while (0 < waitpid(-1, NULL, WNOHANG));
}

void
sighup(int unused)
{
    Arg a = {.i = 1};
    quit(&a);
}

void
sigterm(int unused)
{
    Arg a = {.i = 0};
    quit(&a);
}

#ifndef __OpenBSD__
void
sigdwmblocks(const Arg *arg)
{
    union sigval sv;
    sv.sival_int = 0 | (dwmblockssig << 8) | arg->i;
    if (!dwmblockspid)
        if (getdwmblockspid() == -1)
            return;

    if (sigqueue(dwmblockspid, SIGUSR1, sv) == -1) {
        if (errno == ESRCH) {
            if (!getdwmblockspid())
                sigqueue(dwmblockspid, SIGUSR1, sv);
        }
    }
}
#endif

void
spawn(const Arg *arg)
{
    if (arg->v == dmenucmd)
        dmenumon[0] = '0' + selmon->num;
    if (fork() == 0) {
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
        perror(" failed");
        exit(EXIT_SUCCESS);
    }
}

void
tag(const Arg *arg)
{
    Client *c;
    if (selmon->sel && arg->ui & TAGMASK) {
        c = selmon->sel;
        selmon->sel->tags = arg->ui & TAGMASK;
        setclienttagprop(c);
        focus(NULL);
        arrange(selmon);
    }
}

void
tagmon(const Arg *arg)
{
    if (!selmon->sel || !mons->next)
        return;
    sendmon(selmon->sel, dirtomon(arg->i));
}

void
togglebar(const Arg *arg)
{
    selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] = !selmon->showbar;
    updatebarpos(selmon);
    drawbar(selmon);
    arrange(selmon);
}

void
togglefloating(const Arg *arg)
{
    if (!selmon->sel)
        return;
    if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
        return;
    selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
    if (selmon->sel->isfloating)
        resize(selmon->sel, selmon->sel->x, selmon->sel->y,
            selmon->sel->w, selmon->sel->h, 0);
    arrange(selmon);
}

void
togglefullscr(const Arg *arg)
{
  if(selmon->sel)
    setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}

void
togglesticky(const Arg *arg)
{
    if (!selmon->sel)
        return;
    selmon->sel->issticky = !selmon->sel->issticky;
    arrange(selmon);
}

void
togglescratch(const Arg *arg)
{
    Client *c;
    unsigned int found = 0;
    unsigned int scratchtag = SPTAG(arg->ui);
    Arg sparg = {.v = scratchpads[arg->ui].cmd};

    for (c = selmon->clients; c && !(found = c->tags & scratchtag); c = c->next);
    if (found) {
        unsigned int newtagset = selmon->tagset[selmon->seltags] ^ scratchtag;
        if (newtagset) {
            selmon->tagset[selmon->seltags] = newtagset;
            focus(NULL);
            arrange(selmon);
        }
        if (ISVISIBLE(c)) {
            focus(c);
            restack(selmon);
        }
    } else {
        selmon->tagset[selmon->seltags] |= scratchtag;
        spawn(&sparg);
    }
}

void
toggletag(const Arg *arg)
{
    unsigned int newtags;

    if (!selmon->sel)
        return;
    newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
    if (newtags) {
        selmon->sel->tags = newtags;
        setclienttagprop(selmon->sel);
        focus(NULL);
        arrange(selmon);
    }
}

void
toggleview(const Arg *arg)
{
    unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
    int i;

    if (newtagset) {
        selmon->tagset[selmon->seltags] = newtagset;

        if (newtagset == ~0) {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            selmon->pertag->curtag = 0;
        }

        /* test if the user did not select the same tag */
        if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            for (i = 0; !(newtagset & 1 << i); i++) ;
            selmon->pertag->curtag = i + 1;
        }

        /* apply settings for this view */
        selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
        selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
        selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

        if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
            togglebar(NULL);

        focus(NULL);
        arrange(selmon);
    }
}

void
freeicon(Client *c)
{
    if (c->icon) {
        XRenderFreePicture(dpy, c->icon);
        c->icon = None;
    }
}


void
unfocus(Client *c, int setfocus)
{
    if (!c)
        return;
    grabbuttons(c, 0);
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void
unmanage(Client *c, int destroyed)
{
    Monitor *m = c->mon;
    XWindowChanges wc;

    if (c->swallowing) {
        unswallow(c);
        freeicon(c);
        return;
    }

    Client *s = swallowingclient(c->win);
    if (s) {
        free(s->swallowing);
        s->swallowing = NULL;
        arrange(m);
        focus(NULL);
        return;
    }

    detach(c);
    detachstack(c);
    if (!destroyed) {
        wc.border_width = c->oldbw;
        XGrabServer(dpy); /* avoid race conditions */
        XSetErrorHandler(xerrordummy);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        setclientstate(c, WithdrawnState);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
    free(c);

    if (!s) {
        arrange(m);
        focus(NULL);
        updateclientlist();
    }
}

void
unmapnotify(XEvent *e)
{
    Client *c;
    XUnmapEvent *ev = &e->xunmap;

    if ((c = wintoclient(ev->window))) {
        if (ev->send_event)
            setclientstate(c, WithdrawnState);
        else
            unmanage(c, 0);
    } else if ((c = wintosystrayicon(ev->window))) {
		/* KLUDGE! sometimes icons occasionally unmap their windows, but do
		 * _not_ destroy them. We map those windows back */
		XMapRaised(dpy, c->win);
		updatesystray();
	}
}

void
updatebars(void)
{
    Monitor *m;
    XSetWindowAttributes wa = {
        .override_redirect = True,
        .background_pixmap = ParentRelative,
        .event_mask = ButtonPressMask|ExposureMask,
    };
    XClassHint ch = {"dwm", "dwm"};
    for (int i = 0; i < 2; i++) {
        if (!allbarwin[i]) {
            allbarwin[i] = XCreateWindow(dpy, root, 0, 0, bh, bh, 0, DefaultDepth(dpy, screen),
                    CopyFromParent, DefaultVisual(dpy, screen),
                    CWColormap|CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
            XDefineCursor(dpy, allbarwin[i], cursor[CurNormal]->cursor);
            XMapRaised(dpy, allbarwin[i]);
            XSetClassHint(dpy, allbarwin[i], &ch);
        }
    }
    for (m = mons; m; m = m->next) {
        if (!m->barwin) {
            m->barwin = XCreateWindow(dpy, root, m->wx + sb_padding_x, m->by, m->ww - 2 * sb_padding_x, bh, 0, DefaultDepth(dpy, screen),
                    CopyFromParent, DefaultVisual(dpy, screen),
                    CWColormap|CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
            XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
            XMapRaised(dpy, m->barwin);
            XSetClassHint(dpy, m->barwin, &ch);
        }
    }
    /* Init systray */
    systray->win = XCreateSimpleWindow(dpy, allbarwin[1], 0, 0, bh, bh, 0, 0, scheme[SchemeSel][ColBg].pixel);
    wa.event_mask        = ButtonPressMask | ExposureMask;
    wa.override_redirect = True;
    wa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
    XSelectInput(dpy, systray->win, SubstructureNotifyMask);
    XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32,
            PropModeReplace, (unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
    XChangeWindowAttributes(dpy, systray->win, CWEventMask|CWOverrideRedirect|CWBackPixel, &wa);
    XMapRaised(dpy, systray->win);
    XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);
    if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) == systray->win) {
        sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0, 0);
        XSync(dpy, False);
    }
    XMapSubwindows(dpy, allbarwin[1]);
}

void
updatebarpos(Monitor *m)
{
    m->wy = m->my + sb_padding_y;
    m->wh = m->mh;
    if (m->showbar) {
        m->wh -= bh;
        m->by = m->topbar ? m->wy : m->wy + m->wh;
        m->wy = m->topbar ? m->wy + bh : m->wy;
    } else {
        m->by = -bh;
        m->wy -= sb_padding_y;
        m->wh += sb_padding_y;
    }
}

void
updatecurrentdesktop(Monitor *m)
{
    long data[] = { (long) m->tagset[m->seltags] };
    XChangeProperty(dpy, root, netatom[NetCurrentDesktop],
        XA_CARDINAL, 32, PropModeReplace,
        (unsigned char *) data, 1);
}

void
updateclientlist()
{
    Client *c;
    Monitor *m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next) {
            XChangeProperty(dpy, root, netatom[NetClientList],
                XA_WINDOW, 32, PropModeAppend,
                (unsigned char *) &(c->win), 1);
        }
}

int
updategeom(void)
{
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int i, j, n, nn;
        Client *c;
        Monitor *m;
        XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo *unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++);
        /* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(XineramaScreenInfo));
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
        XFree(info);
        nn = j;
        if (n <= nn) { /* new monitors available */
            for (i = 0; i < (nn - n); i++) {
                for (m = mons; m && m->next; m = m->next);
                if (m)
                    m->next = createmon();
                else
                    mons = createmon();
            }
            for (i = 0, m = mons; i < nn && m; m = m->next, i++)
                if (i >= n
                || unique[i].x_org != m->mx || unique[i].y_org != m->my
                || unique[i].width != m->mw || unique[i].height != m->mh)
                {
                    dirty = 1;
                    m->num = i;
                    m->mx = m->wx = unique[i].x_org;
                    m->my = m->wy = unique[i].y_org ;
                    m->mw = m->ww = unique[i].width;
                    m->mh = unique[i].height;
                    m->wh = unique[i].height;
                    updatebarpos(m);
                }
        } else { /* less monitors available nn < n */
            for (i = nn; i < n; i++) {
                for (m = mons; m && m->next; m = m->next);
                while ((c = m->clients)) {
                    dirty = 1;
                    m->clients = c->next;
                    detachstack(c);
                    c->mon = mons;
                    attach(c);
                    attachstack(c);
                }
                if (m == selmon)
                    selmon = mons;
                cleanupmon(m);
            }
        }
        free(unique);
    } else
#endif /* XINERAMA */
    { /* default monitor setup */
        if (!mons)
            mons = createmon();
        if (mons->mw != sw || mons->mh != sh) {
            dirty = 1;
            mons->mw = mons->ww = sw;
            mons->mh = mons->wh = sh;
            updatebarpos(mons);
        }
    }
    if (dirty) {
        selmon = mons;
        selmon = wintomon(root);
    }
    return dirty;
}

void
updatenumlockmask(void)
{
    unsigned int i, j;
    XModifierKeymap *modmap;

    numlockmask = 0;
    modmap = XGetModifierMapping(dpy);
    for (i = 0; i < 8; i++)
        for (j = 0; j < modmap->max_keypermod; j++)
            if (modmap->modifiermap[i * modmap->max_keypermod + j]
                == XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = (1 << i);
    XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
{
    long msize;
    XSizeHints size;

    if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
        /* size is uninitialized, ensure that size.flags aren't used */
        size.flags = PSize;
    if (size.flags & PBaseSize) {
        c->basew = size.base_width;
        c->baseh = size.base_height;
    } else if (size.flags & PMinSize) {
        c->basew = size.min_width;
        c->baseh = size.min_height;
    } else
        c->basew = c->baseh = 0;
    if (size.flags & PResizeInc) {
        c->incw = size.width_inc;
        c->inch = size.height_inc;
    } else
        c->incw = c->inch = 0;
    if (size.flags & PMaxSize) {
        c->maxw = size.max_width;
        c->maxh = size.max_height;
    } else
        c->maxw = c->maxh = 0;
    if (size.flags & PMinSize) {
        c->minw = size.min_width;
        c->minh = size.min_height;
    } else if (size.flags & PBaseSize) {
        c->minw = size.base_width;
        c->minh = size.base_height;
    } else
        c->minw = c->minh = 0;
    if (size.flags & PAspect) {
        c->mina = (float)size.min_aspect.y / size.min_aspect.x;
        c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
    } else
        c->maxa = c->mina = 0.0;
    c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatestatus(void)
{
    if (!gettextprop(root, XA_WM_NAME, rawstext, sizeof(rawstext)))
        strcpy(stext, "phyOS-dwm-"VERSION);
    else
        copyvalidchars(stext, rawstext);
    drawbar(selmon);
}

void
updatetitle(Client *c)
{
    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    if (c->name[0] == '\0') /* hack to mark broken clients */
        strcpy(c->name, broken);
}

void
updateicon(Client *c)
{
    freeicon(c);
    c->icon = geticonprop(c->win, &c->icw, &c->ich);
}


void
updatewindowtype(Client *c)
{
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen])
        setfullscreen(c, 1);
    if (wtype == netatom[NetWMWindowTypeDialog]) {
        c->iscentered = 1;
        c->isfloating = 1;
    }
}

void
updatewmhints(Client *c)
{
    XWMHints *wmh;

    if ((wmh = XGetWMHints(dpy, c->win))) {
        if (c == selmon->sel && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        } else
            c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
        if (wmh->flags & InputHint)
            c->neverfocus = !wmh->input;
        else
            c->neverfocus = 0;
        XFree(wmh);
    }
}

void
view(const Arg *arg)
{
    int i;
    unsigned int tmptag;

    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
        return;
    selmon->seltags ^= 1; /* toggle sel tagset */
    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
        selmon->pertag->prevtag = selmon->pertag->curtag;

        if (arg->ui == ~0)
            selmon->pertag->curtag = 0;
        else {
            for (i = 0; !(arg->ui & 1 << i); i++) ;
            selmon->pertag->curtag = i + 1;
        }
    } else {
        tmptag = selmon->pertag->prevtag;
        selmon->pertag->prevtag = selmon->pertag->curtag;
        selmon->pertag->curtag = tmptag;
    }

    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
    selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
    selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
    selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

    if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
        togglebar(NULL);

    focus(NULL);
    arrange(selmon);
}

pid_t
winpid(Window w)
{
    pid_t result = 0;

    xcb_res_client_id_spec_t spec = {0};
    spec.client = w;
    spec.mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID;

    xcb_generic_error_t *e = NULL;
    xcb_res_query_client_ids_cookie_t c = xcb_res_query_client_ids(xcon, 1, &spec);
    xcb_res_query_client_ids_reply_t *r = xcb_res_query_client_ids_reply(xcon, c, &e);

    if (!r)
        return (pid_t)0;

    xcb_res_client_id_value_iterator_t i = xcb_res_query_client_ids_ids_iterator(r);
    for (; i.rem; xcb_res_client_id_value_next(&i)) {
        spec = i.data->spec;
        if (spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID) {
            uint32_t *t = xcb_res_client_id_value_value(i.data);
            result = *t;
            break;
        }
    }

    free(r);

    if (result == (pid_t)-1)
        result = 0;
    return result;
}

pid_t
getparentprocess(pid_t p)
{
    unsigned int v = 0;

#if defined(__linux__)
    FILE *f;
    char buf[256];
    snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

    if (!(f = fopen(buf, "r")))
        return (pid_t)0;

    if (fscanf(f, "%*u %*s %*c %u", (unsigned *)&v) != 1)
        v = (pid_t)0;
    fclose(f);
#elif defined(__FreeBSD__)
    struct kinfo_proc *proc = kinfo_getproc(p);
    if (!proc)
        return (pid_t)0;

    v = proc->ki_ppid;
    free(proc);
#endif
    return (pid_t)v;
}

int
isdescprocess(pid_t p, pid_t c)
{
    while (p != c && c != 0)
        c = getparentprocess(c);

    return (int)c;
}

Client *
termforwin(const Client *w)
{
    Client *c;
    Monitor *m;

    if (!w->pid || w->isterminal)
        return NULL;

    for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
            if (c->isterminal && !c->swallowing && c->pid && isdescprocess(c->pid, w->pid))
                return c;
        }
    }

    return NULL;
}

Client *
swallowingclient(Window w)
{
    Client *c;
    Monitor *m;

    for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
            if (c->swallowing && c->swallowing->win == w)
                return c;
        }
    }

    return NULL;
}

void
shiftview(const Arg *arg)
{
	Arg a;
	Client *c;
	unsigned visible = 0;
	int i = arg->i;
	int count = 0;
	int nextseltags, curseltags = selmon->tagset[selmon->seltags];

	do {
		if(i > 0) // left circular shift
			nextseltags = (curseltags << i) | (curseltags >> (LENGTH(tags) - i));

		else // right circular shift
			nextseltags = curseltags >> (- i) | (curseltags << (LENGTH(tags) + i));

                // Check if tag is visible
		for (c = selmon->clients; c && !visible; c = c->next)
			if (nextseltags & c->tags) {
				visible = 1;
				break;
			}
		i += arg->i;
	} while (!visible && ++count < 10);

	if (count < 10) {
		a.i = nextseltags;
		view(&a);
	}
}

void
shifttag(const Arg *arg)
{
	Arg a;
	Client *c;
	unsigned visible = 0;
	int i = arg->i;
	int count = 0;
	int nextseltags, curseltags = selmon->tagset[selmon->seltags];

	do {
		if(i > 0) // left circular shift
			nextseltags = (curseltags << i) | (curseltags >> (LENGTH(tags) - i));

		else // right circular shift
			nextseltags = curseltags >> (- i) | (curseltags << (LENGTH(tags) + i));

                // Check if tag is visible
		for (c = selmon->clients; c && !visible; c = c->next)
			if (nextseltags & c->tags) {
				visible = 1;
				break;
			}
		i += arg->i;
	} while (!visible && ++count < 10);

	if (count < 10) {
		a.i = nextseltags;
		tag(&a);
	}
}

void
updatesystrayicongeom(Client *i, int w, int h)
{
    if (!i)
        return;
    applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
    i->w = w;
    i->h = h;
}

void
updatesystrayiconstate(Client *i, XPropertyEvent *ev)
{
	long flags;
	int code = 0;

	if (!showsystray || !i || ev->atom != xatom[XembedInfo] ||
			!(flags = getatomprop(i, xatom[XembedInfo])))
		return;

	if (flags & XEMBED_MAPPED && !i->tags) {
		i->tags = 1;
		code = XEMBED_WINDOW_ACTIVATE;
		XMapRaised(dpy, i->win);
		setclientstate(i, NormalState);
	}
	else if (!(flags & XEMBED_MAPPED) && i->tags) {
		i->tags = 0;
		code = XEMBED_WINDOW_DEACTIVATE;
		XUnmapWindow(dpy, i->win);
		setclientstate(i, WithdrawnState);
	}
	else
		return;
	sendevent(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0,
			systray->win, XEMBED_EMBEDDED_VERSION);
}

void
updatesystray(void)
{
	XSetWindowAttributes wa;
	Client *i;
	unsigned int w = 1;

	if (!showsystray)
		return;
	for (w = 1, i = systray->icons; i; i = i->next) {
		/* make sure the background color stays the same */
		wa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
		XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
		XMapRaised(dpy, i->win);
		i->x = w;
		XMoveResizeWindow(dpy, i->win, i->x, (bh - i->w) / 2, i->w, i->h);
        w += sb_delimiter_w;
		w += i->w;
	}
    w = w ? w - sb_delimiter_w : 1;
    XWindowChanges wc;
	wc.x = 0; wc.y = 0; wc.width = w; wc.height = bh;
	wc.stack_mode = Above; wc.sibling = allbarwin[1];
	XConfigureWindow(dpy, systray->win, CWX|CWY|CWWidth|CWHeight|CWSibling|CWStackMode, &wc);

	XSetForeground(dpy, drw->gc, scheme[SchemeNorm][ColBg].pixel);
	XFillRectangle(dpy, systray->win, drw->gc, 0, 0, w, bh);
	XSync(dpy, False);
}



Client *
wintoclient(Window w)
{
    Client *c;
    Monitor *m;

    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w)
                return c;
    return NULL;
}

Client *
wintosystrayicon(Window w) {
	Client *i = NULL;

	if (!showsystray || !w)
		return i;
	for (i = systray->icons; i && i->win != w; i = i->next) ;
	return i;
}


Monitor *
wintomon(Window w)
{
    int x, y;
    Client *c;
    Monitor *m;

    if (w == root && getrootptr(&x, &y))
        return recttomon(x, y, 1, 1);
    for (m = mons; m; m = m->next)
        if (w == m->barwin)
            return m;
    if ((c = wintoclient(w)))
        return c->mon;
    return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
    if (ee->error_code == BadWindow
    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
        ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
    return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
    die("dwm: another window manager is already running");
    return -1;
}


void
zoom(const Arg *arg)
{
    Client *c = selmon->sel;

    if (!selmon->lt[selmon->sellt]->arrange
    || (selmon->sel && selmon->sel->isfloating))
        return;
    if (c == nexttiled(selmon->clients))
        if (!c || !(c = nexttiled(c->next)))
            return;
    pop(c);
}


void
swaptags(const Arg *arg)
{
    unsigned int newtag = arg->ui & TAGMASK;
    unsigned int curtag = selmon->tagset[selmon->seltags];

    if (newtag == curtag || !curtag || (curtag & (curtag-1)))
        return;

    for (Client *c = selmon->clients; c != NULL; c = c->next)
    {
        if ((c->tags & newtag) || (c->tags & curtag))
            c->tags ^= curtag ^ newtag;

        if(!c->tags) c->tags = newtag;
        setclienttagprop(c);
    }

    selmon->tagset[selmon->seltags] = newtag;

    focus(NULL);
    arrange(selmon);
}

int
main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("pdwm-"VERSION);
    else if (argc != 1)
        die("usage: pdwm [-v]");
    if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
        fputs("warning: no locale support\n", stderr);
    if (!(dpy = XOpenDisplay(NULL)))
        die("pdwm: cannot open display");
    if (!(xcon = XGetXCBConnection(dpy)))
        die("pdwm: cannot get xcb connection\n");
    checkotherwm();
    buttons = get_buttons();
    keys = get_keys();
    fonts = get_fonts();
    rules = get_rules();
    setup();
#ifdef __OpenBSD__
    if (pledge("stdio rpath proc exec", NULL) == -1)
        die("pledge");
#endif /* __OpenBSD__ */
    scan();
    autostart();
    run();
    if(restart) execvp(argv[0], argv);
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
