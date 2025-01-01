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
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>
#include <X11/Xlib-xcb.h>
#include <xcb/res.h>

#include "drw.h"
#include "util.h"

#include <assert.h>
#include <libgen.h>
#include <sys/stat.h>

/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)                                                     \
    (mask & ~(numlockmask | LockMask) &                                     \
     (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | \
      Mod5Mask))
#define GETINC(X) ((X)-2000)
#define INC(X) ((X) + 2000)
#define INTERSECT(x, y, w, h, m)                                     \
    (MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * \
     MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))
#define ISINC(X) ((X) > 1000 && (X) < 3000)
#define ISVISIBLE(C) \
    ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define PREVSEL 3000
#define LENGTH(X) (sizeof X / sizeof X[0])
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)
#define MOD(N, M) ((N) % (M) < 0 ? (N) % (M) + (M) : (N) % (M))
#define WIDTH(X) ((X)->w + 2 * (X)->bw)
#define HEIGHT(X) ((X)->h + 2 * (X)->bw)
#define NUMTAGS (LENGTH(tags) + LENGTH(scratchpads))
#define TAGMASK ((1 << NUMTAGS) - 1)
#define SPTAG(i) ((1 << LENGTH(tags)) << (i))
#define SPTAGMASK (((1 << LENGTH(scratchpads)) - 1) << LENGTH(tags))
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)
#define TRUNC(X, A, B) (MAX((A), MIN((X), (B))))
#define ColFloat 3
#define OPAQUE 0xffU

/* enums */
enum
{
    CurNormal,
    CurResize,
    CurMove,
    CurLast
}; /* cursor */
enum
{
    SchemeNorm,
    SchemeSel
}; /* color schemes */
enum
{
    ModeCommand,
    ModeInsert
};
enum
{
    NetSupported,
    NetWMName,
    NetWMState,
    NetWMCheck,
    NetWMFullscreen,
    NetActiveWindow,
    NetWMWindowType,
    NetWMWindowTypeDialog,
    NetClientList,
    NetClientInfo,
    NetLast
}; /* EWMH atoms */
enum
{
    WMProtocols,
    WMDelete,
    WMState,
    WMTakeFocus,
    WMLast
}; /* default atoms */
enum
{
    ClkTagBar,
    ClkLtSymbol,
    ClkStatusText,
    ClkWinTitle,
    ClkClientWin,
    ClkRootWin,
    ClkLast
}; /* clicks */

typedef union
{
    int i;
    unsigned int ui;
    float f;
    const void *v;
} Arg;

typedef struct
{
    char *gname;
 	void (*func)(const Arg *arg);
 	const Arg arg;
} Gesture;

typedef struct
{
    unsigned int click;
    unsigned int mask;
    unsigned int button;
    void (*func)(const Arg *arg);
    const Arg arg;
} Button;

typedef struct
{
    unsigned int mod[4];
    KeySym keysym[4];
    void (*func)(const Arg *);
    const Arg arg;
} Command;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client
{
    char name[256];
    float mina, maxa;
    float cfact;
    int x, y, w, h;
    int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
    int bw, oldbw;
    unsigned int tags;
    int allowkill, wasfloating, isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, isterminal, noswallow, issticky;
    unsigned char expandmask;
    int expandx1, expandy1, expandx2, expandy2;
    pid_t pid;
    Client *next;
    Client *snext;
    Client *swallowing;
    Monitor *mon;
    Window win;
};

typedef struct
{
    unsigned int mod;
    KeySym keysym;
    void (*func)(const Arg *);
    const Arg arg;
} Key;

typedef struct
{
    const char *symbol;
    void (*arrange)(Monitor *);
} Layout;

typedef struct Pertag Pertag;

struct Monitor
{
    char ltsymbol[16];
    float mfact;
    int nmaster;
    int num;
    int by;             /* bar geometry */
    int mx, my, mw, mh; /* screen size */
    int wx, wy, ww, wh; /* window area  */
    int gappih;         /* horizontal gap between windows */
    int gappiv;         /* vertical gap between windows */
    int gappoh;         /* horizontal outer gaps */
    int gappov;         /* vertical outer gaps */
    unsigned int seltags;
    unsigned int sellt;
    unsigned int tagset[2];
    int showbar;
    unsigned int barmask;
    int topbar;
    Client *clients;
    Client *sel;
    Client *stack;
    Monitor *next;
    Window barwin;
    const Layout *lt[2];
    Pertag *pertag;
};

typedef struct
{
    const char *class;
    const char *instance;
    const char *title;
    unsigned int tags;
    int allowkill;
    int isfloating;
    int isterminal;
    int noswallow;
    int monitor;
} Rule;

/* Xresources preferences */
enum resource_type
{
    STRING = 0,
    INTEGER = 1,
    FLOAT = 2
};

typedef struct
{
    char *name;
    enum resource_type type;
    void *dst;
} ResourcePref;

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h,
                          int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clearcmd(const Arg *arg);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void copyvalidchars(char *text, char *rawtext);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void keypresscmd(XEvent *e);
static void killclient(const Arg *arg);
static void killthis(Client *c);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void gesture(const Arg *arg);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *c);
static void propertynotify(XEvent *e);
static void pushstack(const Arg *arg);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void runAutostart(void);
static void scan(void);
static int sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setclienttagprop(Client *c);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setinsertmode(void);
static void setkeymode(const Arg *arg);
static void setlayout(const Arg *arg);
static void setmark(Client *c);
static void setcfact(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sigchld(int unused);
#ifndef __OpenBSD__
static int getdwmblockspid();
static void sigdwmblocks(const Arg *arg);
#endif
static void sighup(int unused);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static void swapclient(const Arg *arg);
static void swapfocus(const Arg *arg);
static int stackpos(const Arg *arg);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void spawntag(const Arg *arg);
static void togglebar(const Arg *arg);
static void toggleborder(const Arg *arg);
static void togglefloating(const Arg *arg);
static void toggleallowkill(const Arg *arg);
static void togglemark(const Arg *arg);
static void togglescratch(const Arg *arg);
static void togglesticky(const Arg *arg);
static void togglefullscr(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void xinitvisual();
static void zoom(const Arg *arg);
static void xrdb(const Arg *arg);
static void load_xresources(void);
static void resource_load(XrmDatabase db, char *name, enum resource_type rtype, void *dst);

static pid_t getparentprocess(pid_t p);
static int isdescprocess(pid_t p, pid_t c);
static Client *swallowingclient(Window w);
static Client *termforwin(const Client *c);
static pid_t winpid(Window w);

/* variables */
static const char broken[] = "broken";
static char stext[256];
static char rawstext[256];
static int dwmblockssig;
pid_t dwmblockspid = 0;
static int screen;
static int sw, sh; /* X display screen geometry width, height */
static int bh;     /* bar height */
static int lrpad;  /* sum of left and right padding for text */
static int vp;     /* vertical padding for bar */
static int sp;     /* side padding for bar */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int cmdmod[4];
static unsigned int keymode = ModeCommand;
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent])(XEvent *) = {
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
    [UnmapNotify] = unmapnotify};
static Atom wmatom[WMLast], netatom[NetLast];
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static KeySym cmdkeysym[4];
static Monitor *mons, *selmon;
static Window root, wmcheckwin;
static Client *mark;

static int useargb = 0;
static Visual *visual;
static int depth;
static Colormap cmap;

static xcb_connection_t *xcon;

/* configuration, allows nested code to access above variables */
#include "config.h"

struct Pertag
{
    unsigned int curtag, prevtag;              /* current and previous tag */
    int nmasters[LENGTH(tags) + 1];            /* number of windows in master area */
    float mfacts[LENGTH(tags) + 1];            /* mfacts per tag */
    unsigned int sellts[LENGTH(tags) + 1];     /* selected layouts */
    const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags
{
    char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

/* function implementations */
void
applyrules(Client *c)
{
    const char *class, *instance;
    unsigned int i;
    const Rule *r;
    Monitor *m;
    XClassHint ch = {NULL, NULL};

    /* rule matching */
    c->isfloating = 0;
    c->tags = 0;
    c->allowkill = allowkill;
    XGetClassHint(dpy, c->win, &ch);
    class = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name ? ch.res_name : broken;

    for (i = 0; i < LENGTH(rules); i++)
    {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title)) &&
            (!r->class || strstr(class, r->class)) &&
            (!r->instance || strstr(instance, r->instance)))
        {
            c->isterminal = r->isterminal;
            c->isfloating = r->isfloating;
            c->noswallow = r->noswallow;
            c->tags |= r->tags;
            c->allowkill = r->allowkill;
            if ((r->tags & SPTAGMASK) && r->isfloating)
            {
                c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
                c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
            }

            for (m = mons; m && m->num != r->monitor; m = m->next)
                ;
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
    if (interact)
    {
        if (*x > sw)
            *x = sw - WIDTH(c);
        if (*y > sh)
            *y = sh - HEIGHT(c);
        if (*x + *w + 2 * c->bw < 0)
            *x = 0;
        if (*y + *h + 2 * c->bw < 0)
            *y = 0;
    }
    else
    {
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
    if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange)
    {
        if (!c->hintsvalid)
            updatesizehints(c);
        /* see last two sentences in ICCCM 4.1.2.3 */
        baseismin = c->basew == c->minw && c->baseh == c->minh;
        if (!baseismin)
        { /* temporarily remove base dimensions */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for aspect limits */
        if (c->mina > 0 && c->maxa > 0)
        {
            if (c->maxa < (float)*w / *h)
                *w = *h * c->maxa + 0.5;
            else if (c->mina < (float)*h / *w)
                *h = *w * c->mina + 0.5;
        }
        if (baseismin)
        { /* increment calculation requires this */
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
    if (m)
        showhide(m->stack);
    else
        for (m = mons; m; m = m->next)
            showhide(m->stack);
    if (m)
    {
        arrangemon(m);
        restack(m);
    }
    else
        for (m = mons; m; m = m->next)
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
    updatetitle(p);

    XWindowChanges wc;
    wc.border_width = p->bw;
    XConfigureWindow(dpy, p->win, CWBorderWidth, &wc);
    XMoveResizeWindow(dpy, p->win, p->x, p->y, p->w, p->h);
    XSetWindowBorder(dpy, p->win, scheme[SchemeNorm][ColBorder].pixel);

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

    XWindowChanges wc;
    wc.border_width = c->bw;
    XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
    XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);

    setclientstate(c, NormalState);
    focus(NULL);
    arrange(c->mon);
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
    if ((m = wintomon(ev->window)) && m != selmon)
    {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    if (ev->window == selmon->barwin)
    {
        i = x = 0;
        for (c = m->clients; c; c = c->next)
            occ |= c->tags == 255 ? 0 : c->tags;
        do
        {
            /* do not reserve space for vacant tags */
            if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
                continue;
            x += TEXTW(tags[i]);
        } while (ev->x >= x && ++i < LENGTH(tags));
        if (i < LENGTH(tags))
        {
            click = ClkTagBar;
            arg.ui = 1 << i;
        }
        else if (ev->x < x + TEXTW(selmon->ltsymbol))
            click = ClkLtSymbol;
        else if (ev->x > (x = selmon->ww - (int)TEXTW(stext) + lrpad))
        {
            click = ClkStatusText;

            char *text = rawstext;
            int i = -1;
            char ch;
            dwmblockssig = 0;
            while (text[++i])
            {
                if ((unsigned char)text[i] < ' ')
                {
                    ch = text[i];
                    text[i] = '\0';
                    x += TEXTW(text) - lrpad;
                    text[i] = ch;
                    text += i + 1;
                    i = -1;
                    if (x >= ev->x)
                        break;
                    dwmblockssig = ch;
                }
            }
        }
        else
            click = ClkWinTitle;
    }
    else if ((c = wintoclient(ev->window)))
    {
        focus(c);
        restack(selmon);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        click = ClkClientWin;
    }
    for (i = 0; i < LENGTH(buttons); i++)
        if (click == buttons[i].click && buttons[i].func &&
            buttons[i].button == ev->button &&
            CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(
                click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
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
    Layout foo = {"", NULL};
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
    for (i = 0; i < CurLast; i++)
        drw_cur_free(drw, cursor[i]);
    for (i = 0; i < LENGTH(colors); i++)
        free(scheme[i]);
    free(scheme);
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
    else
    {
        for (m = mons; m && m->next != mon; m = m->next)
            ;
        m->next = mon->next;
    }
    XUnmapWindow(dpy, mon->barwin);
    XDestroyWindow(dpy, mon->barwin);
    free(mon);
}

void
clearcmd(const Arg *arg)
{
    unsigned int i;

    for (i = 0; i < LENGTH(cmdkeysym); i++)
    {
        cmdkeysym[i] = 0;
        cmdmod[i] = 0;
    }
}

void
clientmessage(XEvent *e)
{
    XClientMessageEvent *cme = &e->xclient;
    Client *c = wintoclient(cme->window);

    if (!c)
        return;
    if (cme->message_type == netatom[NetWMState])
    {
        if (cme->data.l[1] == netatom[NetWMFullscreen] ||
            cme->data.l[2] == netatom[NetWMFullscreen])
            setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                              || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
                                  !c->isfullscreen)));
    }
    else if (cme->message_type == netatom[NetActiveWindow])
    {
        if (c != selmon->sel && !c->isurgent)
            seturgent(c, 1);
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
    if (ev->window == root)
    {
        dirty = (sw != ev->width || sh != ev->height);
        sw = ev->width;
        sh = ev->height;
        if (updategeom() || dirty)
        {
            drw_resize(drw, sw, bh);
            updatebars();
            for (m = mons; m; m = m->next)
            {
                for (c = m->clients; c; c = c->next)
                    if (c->isfullscreen)
                        resizeclient(c, m->mx, m->my, m->mw, m->mh);
                XMoveResizeWindow(dpy, m->barwin, m->wx + sp, m->by + vp, m->ww - 2 * sp, bh);
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

    if ((c = wintoclient(ev->window)))
    {
        if (ev->value_mask & CWBorderWidth)
            c->bw = ev->border_width;
        else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange)
        {
            m = c->mon;
            if (ev->value_mask & CWX)
            {
                c->oldx = c->x;
                c->x = m->mx + ev->x;
            }
            if (ev->value_mask & CWY)
            {
                c->oldy = c->y;
                c->y = m->my + ev->y;
            }
            if (ev->value_mask & CWWidth)
            {
                c->oldw = c->w;
                c->w = ev->width;
            }
            if (ev->value_mask & CWHeight)
            {
                c->oldh = c->h;
                c->h = ev->height;
            }
            if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
                c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
            if ((c->y + c->h) > m->my + m->mh && c->isfloating)
                c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
            if ((ev->value_mask & (CWX | CWY)) &&
                !(ev->value_mask & (CWWidth | CWHeight)))
                configure(c);
            if (ISVISIBLE(c))
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        }
        else
            configure(c);
    }
    else
    {
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

    while (rawtext[++i])
    {
        if ((unsigned char)rawtext[i] >= ' ')
        {
            text[j++] = rawtext[i];
        }
    }
    text[j] = '\0';
}

Monitor *createmon(void)
{
    Monitor *m;
    int i;

    m = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact = mfact;
    m->nmaster = nmaster;
    m->showbar = showbar;
    m->barmask = showbar * TAGMASK;
    m->topbar = topbar;
    m->gappih = gappih;
    m->gappiv = gappiv;
    m->gappoh = gappoh;
    m->gappov = gappov;
    m->lt[0] = &layouts[0];
    m->lt[1] = &layouts[1 % LENGTH(layouts)];
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	if (!(m->pertag = (Pertag *)calloc(1, sizeof(Pertag))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Pertag));
	m->pertag->curtag = m->pertag->prevtag = 1;
	for (i=0; i <= LENGTH(tags); i++) {
		/* init nmaster */
		m->pertag->nmasters[i] = m->nmaster;

		/* init mfacts */
		m->pertag->mfacts[i] = m->mfact;

		/* init layouts */
		m->pertag->ltidxs[i][0] = m->lt[0];
		m->pertag->ltidxs[i][1] = m->lt[1];
		m->pertag->sellts[i] = m->sellt;
	}
    return m;
}

void
destroynotify(XEvent *e)
{
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if ((c = wintoclient(ev->window)))
        unmanage(c, 1);

    else if ((c = swallowingclient(ev->window)))
        unmanage(c->swallowing, 1);
}

void
detach(Client *c)
{
    Client **tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next)
        ;
    *tc = c->next;
}

void
detachstack(Client *c)
{
    Client **tc, *t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext)
        ;
    *tc = c->snext;

    if (c == c->mon->sel)
    {
        for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext)
            ;
        c->mon->sel = t;
    }
}

Monitor *dirtomon(int dir)
{
    Monitor *m = NULL;

    if (dir > 0)
    {
        if (!(m = selmon->next))
            m = mons;
    }
    else if (selmon == mons)
        for (m = mons; m->next; m = m->next)
            ;
    else
        for (m = mons; m->next != selmon; m = m->next)
            ;
    return m;
}

void
drawbar(Monitor *m)
{
    int x = 0, w, tw = 0, moveright = 0;
    int boxs = drw->fonts->h / 9;
    int boxw = drw->fonts->h / 6 + 2;
    unsigned int i, j, occ = 0, urg = 0;
    Client *c;

    if (!(m->tagset[m->seltags] & m->barmask))
      return;
	if (barlayout[0] == '\0')
      barlayout = "tln|s";

    drw_setscheme(drw, scheme[SchemeNorm]);
	drw_text(drw, 0, 0, m->ww, bh, 0, "", 0); /* draw background */

	for (i = 0; i < strlen(barlayout); i++) {
		switch (barlayout[i]) {
			case 'l':
				w = TEXTW(m->ltsymbol);
				drw_setscheme(drw, scheme[SchemeNorm]);
				if (moveright) {
					x -= w;
					drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);
				} else
					x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);
				break;
			case 'n':
				tw = TEXTW(m->sel->name);
				if (moveright)
					x -= tw;
				if (m->sel) {
                 drw_setscheme(drw, scheme[m == selmon ? SchemeSel : SchemeNorm]);
					drw_text(drw, x, 0, moveright ? tw : m->ww, bh, lrpad / 2, m->sel->name, 0);
					if (m->sel->isfloating)
						drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
				} else {
					drw_rect(drw, x, 0, tw, bh, 1, 1);
				}
				if (!moveright)
					x += tw;
				break;
			case 's':
				if (m == selmon) { /* status is only drawn on selected monitor */
					drw_setscheme(drw, scheme[SchemeNorm]);
					tw = TEXTW(stext) - lrpad + 2; /* 2px right padding */
					if (moveright) {
						x -= tw;
						drw_text(drw, x, 0, tw, bh, 0, stext, 0);
					} else
						x = drw_text(drw, x, 0, tw, bh, 0, stext, 0);
				}
				break;
			case 't':
				for (c = m->clients; c; c = c->next) {
                    occ |= c->tags == 255 ? 0 : c->tags;
					if (c->isurgent)
						urg |= c->tags;
				}
				/* tags */
				if (moveright) {
					tw = 0;
					for (j = 0; j < LENGTH(tags); j++) {
						tw += TEXTW(tags[j]);
					}
					x -= tw;
				}
				for (j = 0; j < LENGTH(tags); j++) {
                 /* do not draw vacant tags */
                 if (!(occ & 1 << j || m->tagset[m->seltags] & 1 << j))
                     continue;

					w = TEXTW(tags[j]);
					drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << j ? SchemeSel : SchemeNorm]);
					drw_text(drw, x, 0, w, bh, lrpad / 2, tags[j], urg & 1 << j);
					x += w;
				}
				if (moveright)
					x -= tw;
				break;
			case '|':
				moveright = 1;
				x = m->ww;
				break;
    		}
    	}

    drw_map(drw, m->barwin, 0, 0, m->ww, bh);
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

    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) &&
        ev->window != root)
        return;
    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);
    if (m != selmon)
    {
        unfocus(selmon->sel, 1);
        selmon = m;
    }
    else if (!c || c == selmon->sel)
        return;
    focus(c);
}

void
expose(XEvent *e)
{
    Monitor *m;
    XExposeEvent *ev = &e->xexpose;

    if (ev->count == 0 && (m = wintomon(ev->window)))
        drawbar(m);
}

void
focus(Client *c)
{
    if (!c || !ISVISIBLE(c))
    {
        for (c = selmon->stack;
             c && (!ISVISIBLE(c) || (c->issticky && !selmon->sel->issticky));
             c = c->snext)
            ;

        if (!c) /* No windows found; check for available stickies */
            for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext)
                ;
    }

    if (selmon->sel && selmon->sel != c)
        unfocus(selmon->sel, 0);
    if (c)
    {
        if (c->mon != selmon)
            selmon = c->mon;
        if (c->isurgent)
            seturgent(c, 0);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, 1);
        if (c == mark)
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColMark].pixel);
		else
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
        setfocus(c);
    }
    else
    {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
    selmon->sel = c;
    drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
    XFocusChangeEvent *ev = &e->xfocus;

    if (selmon->sel && ev->window != selmon->sel->win)
        setfocus(selmon->sel);
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
    focus(NULL);
}

void
focusstack(const Arg *arg)
{
    int i = stackpos(arg);
    Client *c, *p;

    if (i < 0 || !selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
        return;

    for (p = NULL, c = selmon->clients; c && (i || !ISVISIBLE(c));
         i -= ISVISIBLE(c) ? 1 : 0, p = c, c = c->next)
        ;
    focus(c ? c : p);
    restack(selmon);
}

Atom getatomprop(Client *c, Atom prop)
{
    int di;
    unsigned long dl;
    unsigned char *p = NULL;
    Atom da, atom = None;

    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
                           &da, &di, &dl, &dl, &p) == Success &&
        p)
    {
        atom = *(Atom *)p;
        XFree(p);
    }
    return atom;
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

long getstate(Window w)
{
    int format;
    long result = -1;
    unsigned char *p = NULL;
    unsigned long n, extra;
    Atom real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False,
                           wmatom[WMState], &real, &format, &n, &extra,
                           (unsigned char **)&p) != Success)
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
    {
        strncpy(text, (char *)name.value, size - 1);
    }
    else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success &&
             n > 0 && *list)
    {
        strncpy(text, *list, size - 1);
        XFreeStringList(list);
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
        unsigned int modifiers[] = {0, LockMask, numlockmask,
                                    numlockmask | LockMask};
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (!focused)
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK,
                        GrabModeSync, GrabModeSync, None, None);
        for (i = 0; i < LENGTH(buttons); i++)
            if (buttons[i].click == ClkClientWin)
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabButton(dpy, buttons[i].button, buttons[i].mask | modifiers[j],
                                c->win, False, BUTTONMASK, GrabModeAsync, GrabModeSync,
                                None, None);
    }
}

void
grabkeys(void)
{
    if (keymode == ModeCommand)
    {
        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        return;
    }

    XUngrabKeyboard(dpy, CurrentTime);
    updatenumlockmask();
    {
        unsigned int i, j, k;
        unsigned int modifiers[] = {0, LockMask, numlockmask,
                                    numlockmask | LockMask};
        int start, end, skip;
        KeySym *syms;

        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        XDisplayKeycodes(dpy, &start, &end);
        syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
        if (!syms)
            return;
        for (k = start; k <= end; k++)
            for (i = 0; i < LENGTH(keys); i++)
                /* skip modifier codes, we do that ourselves */
                if (keys[i].keysym == syms[(k - start) * skip])
                    for (j = 0; j < LENGTH(modifiers); j++)
                        XGrabKey(dpy, k, keys[i].mod | modifiers[j], root, True,
                                 GrabModeAsync, GrabModeAsync);
        XFree(syms);
    }
}
void
incnmaster(const Arg *arg)
{
    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
    arrange(selmon);
}

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n,
                        XineramaScreenInfo *info)
{
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
            unique[n].width == info->width && unique[n].height == info->height)
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

    if (keymode == ModeCommand)
    {
        keypresscmd(e);
        return;
    }

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    for (i = 0; i < LENGTH(keys); i++)
        if (keysym == keys[i].keysym &&
            CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func)
            keys[i].func(&(keys[i].arg));
}

void
keypresscmd(XEvent *e)
{
    unsigned int i, j;
    int matches = 0;
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    if (XK_Shift_L <= keysym && keysym <= XK_Hyper_R)
    {
        return;
    }

    for (i = 0; i < LENGTH(cmdkeys); i++)
    {
        if (keysym == cmdkeys[i].keysym && CLEANMASK(cmdkeys[i].mod) == CLEANMASK(ev->state) && cmdkeys[i].func)
        {
            cmdkeys[i].func(&(cmdkeys[i].arg));
            return;
        }
    }

    for (j = 0; j < LENGTH(cmdkeysym); j++)
    {
        if (cmdkeysym[j] == 0)
        {
            cmdkeysym[j] = keysym;
            cmdmod[j] = ev->state;
            break;
        }
    }

    for (i = 0; i < LENGTH(commands); i++)
    {
        matches = 0;
        for (j = 0; j < LENGTH(cmdkeysym); j++)
        {
            if (cmdkeysym[j] == commands[i].keysym[j] && CLEANMASK(cmdmod[j]) == CLEANMASK(commands[i].mod[j]))
                matches++;
        }
        if (matches == LENGTH(cmdkeysym))
        {
            if (commands[i].func)
                commands[i].func(&(commands[i].arg));
            clearcmd(NULL);
            return;
        }
    }
}

void
killclient(const Arg *arg)
{
    Client *c;

    if (!selmon->sel || !selmon->sel->allowkill)
        return;

    if (!arg->ui || arg->ui == 0)
    {
        killthis(selmon->sel);
        return;
    }

    for (c = selmon->clients; c; c = c->next)
    {
        if (!ISVISIBLE(c) || (arg->ui == 1 && c == selmon->sel))
            continue;
        killthis(c);
    }
}

void
killthis(Client *c)
{
    if (!sendevent(c, wmatom[WMDelete]))
    {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, c->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
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
    c->cfact = 1.0;

    updatetitle(c);
    if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans)))
    {
        c->mon = t->mon;
        c->tags = t->tags;
    }
    else
    {
        c->mon = selmon;
        applyrules(c);
        term = termforwin(c);
    }

    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
        c->x = c->mon->wx + c->mon->ww - WIDTH(c);
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
        c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
    c->x = MAX(c->x, c->mon->wx);
    c->y = MAX(c->y, c->mon->wy);
    c->bw = borderpx;

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    if (c == mark)
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColMark].pixel);
	else
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
    configure(c); /* propagates border_width, if size doesn't change */
    updatewindowtype(c);
    updatesizehints(c);
    updatewmhints(c);
    {
        int format;
        unsigned long *data, n, extra;
        Monitor *m;
        Atom atom;
        if (XGetWindowProperty(dpy, c->win, netatom[NetClientInfo], 0L, 2L, False,
                               XA_CARDINAL, &atom, &format, &n, &extra,
                               (unsigned char **)&data) == Success &&
            n == 2)
        {
            c->tags = *data;
            for (m = mons; m; m = m->next)
            {
                if (m->num == *(data + 1))
                {
                    c->mon = m;
                    break;
                }
            }
        }
        if (n > 0)
            XFree(data);
    }
    c->sfx = c->x;
    c->sfy = c->y;
    c->sfw = c->w;
    c->sfh = c->h;
    setclienttagprop(c);

    c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
    c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
    XSelectInput(dpy, w,
                 EnterWindowMask | FocusChangeMask | PropertyChangeMask |
                     StructureNotifyMask);
    grabbuttons(c, 0);
    c->wasfloating = 0;
    c->expandmask = 0;
    if (!c->isfloating)
        c->isfloating = c->oldstate = trans != None || c->isfixed;
    if (c->isfloating)
        XRaiseWindow(dpy, c->win);
    attach(c);
    attachstack(c);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                    PropModeAppend, (unsigned char *)&(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w,
                      c->h); /* some windows require this */
    setclientstate(c, NormalState);
    if (selmon->sel && selmon->sel->isfullscreen && !c->isfloating)
        setfullscreen(selmon->sel, 0);
    if (c->mon == selmon)
        unfocus(selmon->sel, 0);
    c->mon->sel = c;
    XMapWindow(dpy, c->win);
    if (term)
        swallow(term, c);
    arrange(c->mon);
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

    if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect)
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
        resize(c, m->wx + ov, m->wy + oh, m->ww - 2 * c->bw - 2 * ov,
               m->wh - 2 * c->bw - 2 * oh, 0);
}

void
motionnotify(XEvent *e)
{
    static Monitor *mon = NULL;
    Monitor *m;
    XMotionEvent *ev = &e->xmotion;

    if (ev->window != root)
        return;
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon)
    {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    mon = m;
}

void
gesture(const Arg *arg) {
	int x, y, dx, dy, q;
	int valid=0, listpos=0, gestpos=0, count=0;
	char move, currGest[10];
	XEvent ev;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if(!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
			case ConfigureRequest:
			case Expose:
			case MapRequest:
				handler[ev.type](&ev);
				break;
			case MotionNotify:
				if(count++ < 10)
					break;
				count = 0;
				dx = ev.xmotion.x - x;
				dy = ev.xmotion.y - y;
				x = ev.xmotion.x;
				y = ev.xmotion.y;

				if( abs(dx)/(abs(dy)+1) == 0 )
					move = dy<0?'u':'d';
				else
					move = dx<0?'l':'r';

				if(move!=currGest[gestpos-1])
				{
					if(gestpos>9)
					{	ev.type++;
						break;
					}

					currGest[gestpos] = move;
					currGest[++gestpos] = '\0';

					valid = 0;
					for(q = 0; q<LENGTH(gestures); q++)
					{	if(!strcmp(currGest, gestures[q].gname))
						{	valid++;
							listpos = q;
						}
					}
				}

		}
	} while(ev.type != ButtonRelease);

	if(valid)
		gestures[listpos].func(&(gestures[listpos].arg));

	XUngrabPointer(dpy, CurrentTime);
}

void
movemouse(const Arg *arg)
{
    int x, y, ocx, ocy, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;

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
    do
    {
        XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
        switch (ev.type)
        {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            nx = ocx + (ev.xmotion.x - x);
            ny = ocy + (ev.xmotion.y - y);
            if ((m = recttomon(nx, ny, c->w, c->h)))
            {
                if (m != selmon)
                    sendmon(c, m);
                if (abs(selmon->wx - nx) < snap)
                    nx = selmon->wx;
                else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
                    nx = selmon->wx + selmon->ww - WIDTH(c);
                if (abs(selmon->wy - ny) < snap)
                    ny = selmon->wy;
                else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
                    ny = selmon->wy + selmon->wh - HEIGHT(c);
                if (!c->isfloating && selmon->lt[selmon->sellt]->arrange &&
                    (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
                    togglefloating(NULL);
                if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                    resize(c, nx, ny, c->w, c->h, 1);
            }
            break;
        }
    } while (ev.type != ButtonRelease);
    XUngrabPointer(dpy, CurrentTime);
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon)
    {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

Client *
nexttiled(Client *c)
{
    for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next)
        ;
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
pushstack(const Arg *arg)
{
    int i = stackpos(arg);
    Client *sel = selmon->sel, *c, *p;

    if (i < 0 || !sel)
        return;
    else if (i == 0)
    {
        detach(sel);
        attach(sel);
    }
    else
    {
        for (p = NULL, c = selmon->clients; c; p = c, c = c->next)
            if (!(i -= (ISVISIBLE(c) && c != sel)))
                break;
        c = c ? c : p;
        detach(sel);
        sel->next = c->next;
        c->next = sel;
    }
    arrange(selmon);
}

void
propertynotify(XEvent *e)
{
    Client *c;
    Window trans;
    XPropertyEvent *ev = &e->xproperty;

    if ((ev->window == root) && (ev->atom == XA_WM_NAME))
    {
        updatestatus();
    }
    else if (ev->state == PropertyDelete)
    {
        return; /* ignore */
    }
    else if ((c = wintoclient(ev->window)))
    {
        switch (ev->atom)
        {
        default:
            break;
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
        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName])
        {
            updatetitle(c);
            if (c == c->mon->sel)
                drawbar(c->mon);
        }
        if (ev->atom == netatom[NetWMWindowType])
            updatewindowtype(c);
    }
}

void
quit(const Arg *arg)
{
    if (arg->i)
        restart = 1;
    running = 0;
}

Monitor *recttomon(int x, int y, int w, int h)
{
    Monitor *m, *r = selmon;
    int a, area = 0;

    for (m = mons; m; m = m->next)
        if ((a = INTERSECT(x, y, w, h, m)) > area)
        {
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

    c->oldx = c->x;
    c->x = wc.x = x;
    c->oldy = c->y;
    c->y = wc.y = y;
    c->oldw = c->w;
    c->w = wc.width = w;
    c->oldh = c->h;
    c->h = wc.height = h;
    c->expandmask = 0;
    wc.border_width = c->bw;
    XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth,
                     &wc);
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
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1,
                 c->h + c->bw - 1);
    do
    {
        XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
        switch (ev.type)
        {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
            nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
            if ((m = recttomon(c->x, c->y, nw, nh)))
            {
                if (m != selmon)
                    sendmon(c, m);
                if (!c->isfloating && selmon->lt[selmon->sellt]->arrange && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                    togglefloating(NULL);
            }
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, c->x, c->y, nw, nh, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1,
                 c->h + c->bw - 1);
    XUngrabPointer(dpy, CurrentTime);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev))
        ;
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon)
    {
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
    if (m->lt[m->sellt]->arrange)
    {
        wc.stack_mode = Below;
        wc.sibling = m->barwin;
        for (c = m->stack; c; c = c->snext)
            if (!c->isfloating && ISVISIBLE(c))
            {
                XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
                wc.sibling = c->win;
            }
    }
    XSync(dpy, False);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev))
        ;
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
runAutostart(void)
{
  system("killall -q dwmblocks; dwmblocks &");
}

void
scan(void)
{
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num))
    {
        for (i = 0; i < num; i++)
        {
            if (!XGetWindowAttributes(dpy, wins[i], &wa) || wa.override_redirect ||
                XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
                manage(wins[i], &wa);
        }
        for (i = 0; i < num; i++)
        { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa))
                continue;
            if (XGetTransientForHint(dpy, wins[i], &d1) &&
                (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }
        if (wins)
            XFree(wins);
    }
}

void
sendmon(Client *c, Monitor *m)
{
    Monitor *oldm = selmon;
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
    if (oldm != m)
        arrange(oldm);
    arrange(m);
    focus(c);
    restack(m);
}

void
setclientstate(Client *c, long state)
{
    long data[] = {state, None};

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
                    PropModeReplace, (unsigned char *)data, 2);
}

void
setinsertmode()
{
    keymode = ModeInsert;
    clearcmd(NULL);
    grabkeys();
}

void
setkeymode(const Arg *arg)
{
    if (!arg)
        return;
    keymode = arg->ui;
    clearcmd(NULL);
    grabkeys();
}

int
sendevent(Client *c, Atom proto)
{
    int n;
    Atom *protocols;
    int exists = 0;
    XEvent ev;

    if (XGetWMProtocols(dpy, c->win, &protocols, &n))
    {
        while (!exists && n--)
            exists = protocols[n] == proto;
        XFree(protocols);
    }
    if (exists)
    {
        ev.type = ClientMessage;
        ev.xclient.window = c->win;
        ev.xclient.message_type = wmatom[WMProtocols];
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = proto;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(dpy, c->win, False, NoEventMask, &ev);
    }
    return exists;
}

void
setfocus(Client *c)
{
    if (!c->neverfocus)
    {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow], XA_WINDOW, 32,
                        PropModeReplace, (unsigned char *)&(c->win), 1);
    }
    sendevent(c, wmatom[WMTakeFocus]);
}

void
setfullscreen(Client *c, int fullscreen)
{
    if (fullscreen && !c->isfullscreen)
    {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)&netatom[NetWMFullscreen],
                        1);
        c->isfullscreen = 1;
        c->oldstate = c->isfloating;
        c->oldbw = c->bw;
        c->bw = 0;
        c->isfloating = 1;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    }
    else if (!fullscreen && c->isfullscreen)
    {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)0, 0);
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
stackpos(const Arg *arg)
{
    int n, i;
    Client *c, *l;

    if (!selmon->clients)
        return -1;

    if (arg->i == PREVSEL)
    {
        for (l = selmon->stack; l && (!ISVISIBLE(l) || l == selmon->sel);
             l = l->snext)
            ;
        if (!l)
            return -1;
        for (i = 0, c = selmon->clients; c != l;
             i += ISVISIBLE(c) ? 1 : 0, c = c->next)
            ;
        return i;
    }
    else if (ISINC(arg->i))
    {
        if (!selmon->sel)
            return -1;
        for (i = 0, c = selmon->clients; c != selmon->sel;
             i += ISVISIBLE(c) ? 1 : 0, c = c->next)
            ;
        for (n = i; c; n += ISVISIBLE(c) ? 1 : 0, c = c->next)
            ;
        return MOD(i + GETINC(arg->i), n);
    }
    else if (arg->i < 0)
    {
        for (i = 0, c = selmon->clients; c; i += ISVISIBLE(c) ? 1 : 0, c = c->next)
            ;
        return MAX(i + arg->i, 0);
    }
    else
        return arg->i;
}

void
setlayout(const Arg *arg)
{
    if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
    {
        selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
        if (!selmon->lt[selmon->sellt]->arrange)
        {
            for (Client *c = selmon->clients; c; c = c->next)
            {
                if (!c->isfloating)
                {
                    /*restore last known float dimensions*/
                    resize(c, selmon->mx + c->sfx, selmon->my + c->sfy,
                           c->sfw, c->sfh, 0);
                }
            }
        }
    }
    if (arg && arg->v)
        selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
    selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
    strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
    if (selmon->sel)
        arrange(selmon);
    else
        drawbar(selmon);

    setinsertmode();
}

void
setcfact(const Arg *arg)
{
    float f;
    Client *c;

    c = selmon->sel;

    if (!arg || !c || !selmon->lt[selmon->sellt]->arrange)
        return;
    f = arg->f + c->cfact;
    if (arg->f == 0.0)
        f = 1.0;
    else if (f < 0.25 || f > 4.0)
        return;
    c->cfact = f;
    arrange(selmon);
}

void
setmark(Client *c)
{
	if (c == mark)
		return;
	if (mark) {
		XSetWindowBorder(dpy, mark->win, scheme[mark == selmon->sel
				? SchemeSel : SchemeNorm][ColBorder].pixel);
		mark = 0;
	}
	if (c) {
		XSetWindowBorder(dpy, c->win, scheme[c == selmon->sel
				? SchemeSel : SchemeNorm][ColMark].pixel);
		mark = c;
	}
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
    xinitvisual();
    drw = drw_create(dpy, screen, root, sw, sh, visual, depth, cmap);
    if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
        die("no fonts could be loaded.");
    lrpad = drw->fonts->h;
    bh = drw->fonts->h + 2;
    sp = sidepad;
    vp = (topbar == 1) ? vertpad : -vertpad;
    updategeom();
    /* init atoms */
    utf8string = XInternAtom(dpy, "UTF8_STRING", False);
    wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
    wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
    netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
    netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
    netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
    netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    netatom[NetWMFullscreen] =
        XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    netatom[NetWMWindowTypeDialog] =
        XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    netatom[NetClientInfo] = XInternAtom(dpy, "_NET_CLIENT_INFO", False);
    /* init cursors */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove] = drw_cur_create(drw, XC_fleur);
    /* init appearance */
    scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
    for (i = 0; i < LENGTH(colors); i++)
        scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 4);
    /* init bars */
    updatebars();
    updatestatus();
    /* supporting window for NetWMCheck */
    wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&wmcheckwin, 1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
                    PropModeReplace, (unsigned char *)"dwm", 3);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&wmcheckwin, 1);
    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    XDeleteProperty(dpy, root, netatom[NetClientInfo]);
    /* select events */
    wa.cursor = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                    ButtonPressMask | PointerMotionMask | EnterWindowMask |
                    LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
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
    if (ISVISIBLE(c))
    {
        if ((c->tags & SPTAGMASK) && c->isfloating)
        {
            c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
            c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
        }
        /* show clients top down */
        XMoveWindow(dpy, c->win, c->x, c->y);
        if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) &&
            !c->isfullscreen)
            resize(c, c->x, c->y, c->w, c->h, 0);
        showhide(c->snext);
    }
    else
    {
        /* hide clients bottom up */
        showhide(c->snext);
        XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
    }
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

    if (sigqueue(dwmblockspid, SIGUSR1, sv) == -1)
    {
        if (errno == ESRCH)
        {
            if (!getdwmblockspid())
                sigqueue(dwmblockspid, SIGUSR1, sv);
        }
    }
}
#endif

void
sigchld(int unused)
{
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
        die("can't install SIGCHLD handler:");
    while (0 < waitpid(-1, NULL, WNOHANG))
        ;
}

#define SPAWN_CWD_DELIM " []{}()<>\"':"

void
spawn(const Arg *arg)
{
    setinsertmode();
    if (fork() == 0)
    {
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
    }
}

void
setclienttagprop(Client *c)
{
    long data[] = {(long)c->tags, (long)c->mon->num};
    XChangeProperty(dpy, c->win, netatom[NetClientInfo], XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *)data, 2);
}

void
swapclient(const Arg *arg)
{
	Client *s, *m, t;

	if (!mark || !selmon->sel || mark == selmon->sel
	    || !selmon->lt[selmon->sellt]->arrange)
		return;
	s = selmon->sel;
	m = mark;
	t = *s;
	strcpy(s->name, m->name);
	s->win = m->win;
	s->x = m->x;
	s->y = m->y;
	s->w = m->w;
	s->h = m->h;

	m->win = t.win;
	strcpy(m->name, t.name);
	m->x = t.x;
	m->y = t.y;
	m->w = t.w;
	m->h = t.h;

	selmon->sel = m;
	mark = s;
	focus(s);
	setmark(m);

	arrange(s->mon);
	if (s->mon != m->mon) {
		arrange(m->mon);
	}
}

void
swapfocus(const Arg *arg)
{
	Client *t;

	if (!selmon->sel || !mark || selmon->sel == mark)
		return;
	t = selmon->sel;
	if (mark->mon != selmon) {
		unfocus(selmon->sel, 0);
		selmon = mark->mon;
	}
	if (ISVISIBLE(mark)) {
		focus(mark);
		restack(selmon);
	} else {
		selmon->seltags ^= 1;
		selmon->tagset[selmon->seltags] = mark->tags;
		focus(mark);
		arrange(selmon);
	}
	setmark(t);
}

void
togglemark(const Arg *arg)
{
	if (!selmon->sel)
		return;
	setmark(selmon->sel == mark ? 0 : selmon->sel);
}

void
tag(const Arg *arg)
{
    Client *c;
    if (selmon->sel && arg->ui & TAGMASK)
    {
        c = selmon->sel;
        selmon->sel->tags = arg->ui & TAGMASK;
        setclienttagprop(c);
        focus(NULL);
        arrange(selmon);
    }
}

void
spawntag(const Arg *arg)
{
	if (arg->ui & TAGMASK) {
		for (int i = LENGTH(tags); i >= 0; i--) {
			if (arg->ui & 1<<i) {
				spawn(&tagexec[i]);
				return;
			}
		}
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
    unsigned int ctag = selmon->tagset[selmon->seltags];

    if (arg->i == 1 || ctag == TAGMASK)
    {
        selmon->showbar = !selmon->showbar;
        selmon->barmask = selmon->showbar * TAGMASK;
    }
    else
    {
        selmon->barmask ^= ctag;
    }
    updatebarpos(selmon);
    XMoveResizeWindow(dpy, selmon->barwin, selmon->wx + sp, selmon->by + vp,
                      selmon->ww - 2 * sp, bh);
    arrange(selmon);
}

void
toggleborder(const Arg *arg)
{
    selmon->sel->bw = (selmon->sel->bw == borderpx ? 0 : borderpx);
    arrange(selmon);
}

void
toggleallowkill(const Arg *arg)
{
    if (!selmon->sel) return;
    selmon->sel->allowkill = !selmon->sel->allowkill;
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
    {
        /*restore last known float dimensions*/
        resize(selmon->sel, selmon->mx + selmon->sel->sfx, selmon->my + selmon->sel->sfy,
               selmon->sel->sfw, selmon->sel->sfh, 0);
    }
    else
    {
        if (selmon->sel->isfullscreen)
            setfullscreen(selmon->sel, 0);
        /*save last known float dimensions*/
        selmon->sel->sfx = selmon->sel->x - selmon->mx;
        selmon->sel->sfy = selmon->sel->y - selmon->my;
        selmon->sel->sfw = selmon->sel->w;
        selmon->sel->sfh = selmon->sel->h;
    }
    arrange(selmon);
}

void
togglefullscr(const Arg *arg)
{
    if (selmon->sel)
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

    for (c = selmon->clients; c && !(found = c->tags & scratchtag); c = c->next)
        ;
    if (found)
    {
        unsigned int newtagset = selmon->tagset[selmon->seltags] ^ scratchtag;
        if (newtagset)
        {
            selmon->tagset[selmon->seltags] = newtagset;
            focus(NULL);
            arrange(selmon);
        }
        if (ISVISIBLE(c))
        {
            focus(c);
            restack(selmon);
        }
    }
    else
    {
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
    if (newtags)
    {
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

    if (newtagset)
    {
        if (newtagset == ~0)
        {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            selmon->pertag->curtag = 0;
        }
        /* test if the user did not select the same tag */
        if (!(newtagset & 1 << (selmon->pertag->curtag - 1)))
        {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            for (i = 0; !(newtagset & 1 << i); i++)
                ;
            selmon->pertag->curtag = i + 1;
        }
        selmon->tagset[selmon->seltags] = newtagset;

        /* apply settings for this view */
        selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
        selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
        selmon->lt[selmon->sellt ^ 1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt ^ 1];
        focus(NULL);
        arrange(selmon);
    }
}

void
unfocus(Client *c, int setfocus)
{
    if (!c)
        return;
    grabbuttons(c, 0);
    if (c == mark)
		XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColMark].pixel);
	else
		XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
    if (setfocus)
    {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void
unmanage(Client *c, int destroyed)
{
    Monitor *m = c->mon;
    XWindowChanges wc;

    if (c->swallowing)
    {
        unswallow(c);
        return;
    }

    Client *s = swallowingclient(c->win);
    if (s)
    {
        free(s->swallowing);
        s->swallowing = NULL;
        arrange(m);
        focus(NULL);
        return;
    }

	if (c == mark)
		setmark(0);

    detach(c);
    detachstack(c);
    if (!destroyed)
    {
        wc.border_width = c->oldbw;
        XGrabServer(dpy); /* avoid race conditions */
        XSetErrorHandler(xerrordummy);
        XSelectInput(dpy, c->win, NoEventMask);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        setclientstate(c, WithdrawnState);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
    free(c);

    if (!s)
    {
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

    if ((c = wintoclient(ev->window)))
    {
        if (ev->send_event)
            setclientstate(c, WithdrawnState);
        else
            unmanage(c, 0);
    }
}

void
updatebars(void)
{
    Monitor *m;
    XSetWindowAttributes wa = {.override_redirect = True,
                               .background_pixel = 0,
                               .border_pixel = 0,
                               .colormap = cmap,
                               .event_mask = ButtonPressMask | ExposureMask};
    XClassHint ch = {"dwm", "dwm"};
    for (m = mons; m; m = m->next)
    {
        if (m->barwin)
            continue;
        m->barwin = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, m->ww - 2 * sp,
                                  bh, 0, depth, InputOutput, visual,
                                  CWOverrideRedirect | CWBackPixel | CWBorderPixel |
                                      CWColormap | CWEventMask,
                                  &wa);
        XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
        XMapRaised(dpy, m->barwin);
        XSetClassHint(dpy, m->barwin, &ch);
    }
}

void
updatebarpos(Monitor *m)
{
    m->wy = m->my;
    m->wh = m->mh;
    if ((m->tagset[m->seltags] & m->barmask))
    {
        m->wh = m->wh - vertpad - bh;
        m->by = m->topbar ? m->wy : m->wy + m->wh + vertpad;
        m->wy = m->topbar ? m->wy + bh + vp : m->wy;
    }
    else
        m->by = -bh - vp;
}

void
updateclientlist()
{
    Client *c;
    Monitor *m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                            PropModeAppend, (unsigned char *)&(c->win), 1);
}

int
updategeom(void)
{
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(dpy))
    {
        int i, j, n, nn;
        Client *c;
        Monitor *m;
        XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo *unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++)
            ;
        /* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(XineramaScreenInfo));
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
        XFree(info);
        nn = j;

        /* new monitors if nn > n */
        for (i = n; i < nn; i++)
        {
            for (m = mons; m && m->next; m = m->next)
                ;
            if (m)
                m->next = createmon();
            else
                mons = createmon();
        }
        for (i = 0, m = mons; i < nn && m; m = m->next, i++)
            if (i >= n || unique[i].x_org != m->mx || unique[i].y_org != m->my ||
                unique[i].width != m->mw || unique[i].height != m->mh)
            {
                dirty = 1;
                m->num = i;
                m->mx = m->wx = unique[i].x_org;
                m->my = m->wy = unique[i].y_org;
                m->mw = m->ww = unique[i].width;
                m->mh = m->wh = unique[i].height;
                updatebarpos(m);
            }
        /* removed monitors if n > nn */
        for (i = nn; i < n; i++)
        {
            for (m = mons; m && m->next; m = m->next)
                ;
            while ((c = m->clients))
            {
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
        free(unique);
    }
    else
#endif /* XINERAMA */
    {  /* default monitor setup */
        if (!mons)
            mons = createmon();
        if (mons->mw != sw || mons->mh != sh)
        {
            dirty = 1;
            mons->mw = mons->ww = sw;
            mons->mh = mons->wh = sh;
            updatebarpos(mons);
        }
    }
    if (dirty)
    {
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
            if (modmap->modifiermap[i * modmap->max_keypermod + j] ==
                XKeysymToKeycode(dpy, XK_Num_Lock))
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
    if (size.flags & PBaseSize)
    {
        c->basew = size.base_width;
        c->baseh = size.base_height;
    }
    else if (size.flags & PMinSize)
    {
        c->basew = size.min_width;
        c->baseh = size.min_height;
    }
    else
        c->basew = c->baseh = 0;
    if (size.flags & PResizeInc)
    {
        c->incw = size.width_inc;
        c->inch = size.height_inc;
    }
    else
        c->incw = c->inch = 0;
    if (size.flags & PMaxSize)
    {
        c->maxw = size.max_width;
        c->maxh = size.max_height;
    }
    else
        c->maxw = c->maxh = 0;
    if (size.flags & PMinSize)
    {
        c->minw = size.min_width;
        c->minh = size.min_height;
    }
    else if (size.flags & PBaseSize)
    {
        c->minw = size.base_width;
        c->minh = size.base_height;
    }
    else
        c->minw = c->minh = 0;
    if (size.flags & PAspect)
    {
        c->mina = (float)size.min_aspect.y / size.min_aspect.x;
        c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
    }
    else
        c->maxa = c->mina = 0.0;
    c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatestatus(void)
{
    if (!gettextprop(root, XA_WM_NAME, rawstext, sizeof(rawstext)))
        strcpy(stext, "dwm-" VERSION);
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
updatewindowtype(Client *c)
{
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen])
        setfullscreen(c, 1);
    if (wtype == netatom[NetWMWindowTypeDialog])
        c->isfloating = 1;
}

void
updatewmhints(Client *c)
{
    XWMHints *wmh;

    if ((wmh = XGetWMHints(dpy, c->win)))
    {
        if (c == selmon->sel && wmh->flags & XUrgencyHint)
        {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        }
        else
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
    if (arg->ui & TAGMASK)
    {
        selmon->pertag->prevtag = selmon->pertag->curtag;
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
        if (arg->ui == ~0)
            selmon->pertag->curtag = 0;
        else
        {
            for (i = 0; !(arg->ui & 1 << i); i++)
                ;
            selmon->pertag->curtag = i + 1;
        }
    }
    else
    {
        tmptag = selmon->pertag->prevtag;
        selmon->pertag->prevtag = selmon->pertag->curtag;
        selmon->pertag->curtag = tmptag;
    }
    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
    selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
    selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
    selmon->lt[selmon->sellt ^ 1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt ^ 1];
    updatebarpos(selmon);
    XMoveResizeWindow(dpy, selmon->barwin, selmon->wx + sp, selmon->by + vp,
                      selmon->ww - 2 * sp, bh);
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
    xcb_res_query_client_ids_cookie_t c =
        xcb_res_query_client_ids(xcon, 1, &spec);
    xcb_res_query_client_ids_reply_t *r =
        xcb_res_query_client_ids_reply(xcon, c, &e);

    if (!r)
        return (pid_t)0;

    xcb_res_client_id_value_iterator_t i = xcb_res_query_client_ids_ids_iterator(r);
    for (; i.rem; xcb_res_client_id_value_next(&i))
    {
        spec = i.data->spec;
        if (spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID)
        {
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

    for (m = mons; m; m = m->next)
    {
        for (c = m->clients; c; c = c->next)
        {
            if (c->isterminal && !c->swallowing && c->pid &&
                isdescprocess(c->pid, w->pid))
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

    for (m = mons; m; m = m->next)
    {
        for (c = m->clients; c; c = c->next)
        {
            if (c->swallowing && c->swallowing->win == w)
                return c;
        }
    }

    return NULL;
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

Monitor *wintomon(Window w)
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
    if (ee->error_code == BadWindow ||
        (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch) ||
        (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable) ||
        (ee->request_code == X_PolyFillRectangle &&
         ee->error_code == BadDrawable) ||
        (ee->request_code == X_PolySegment && ee->error_code == BadDrawable) ||
        (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch) ||
        (ee->request_code == X_GrabButton && ee->error_code == BadAccess) ||
        (ee->request_code == X_GrabKey && ee->error_code == BadAccess) ||
        (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
            ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee) { return 0; }

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
    die("dwm: another window manager is already running");
    return -1;
}

void
xinitvisual()
{
    XVisualInfo *infos;
    XRenderPictFormat *fmt;
    int nitems;
    int i;

    XVisualInfo tpl = {.screen = screen, .depth = 32, .class = TrueColor};
    long masks = VisualScreenMask | VisualDepthMask | VisualClassMask;

    infos = XGetVisualInfo(dpy, masks, &tpl, &nitems);
    visual = NULL;
    for (i = 0; i < nitems; i++)
    {
        fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
        if (fmt->type == PictTypeDirect && fmt->direct.alphaMask)
        {
            visual = infos[i].visual;
            depth = infos[i].depth;
            cmap = XCreateColormap(dpy, root, visual, AllocNone);
            useargb = 1;
            break;
        }
    }

    XFree(infos);

    if (!visual)
    {
        visual = DefaultVisual(dpy, screen);
        depth = DefaultDepth(dpy, screen);
        cmap = DefaultColormap(dpy, screen);
    }
}

void
zoom(const Arg *arg)
{
    Client *c = selmon->sel;

    if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
        return;
    if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
        return;
    pop(c);
}

void
xrdb(const Arg *arg)
{
    load_xresources();

    for (int i = 0; i < LENGTH(colors); i++)
        scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 3);

    focus(NULL);
    arrange(NULL);
}

void
resource_load(XrmDatabase db, char *name, enum resource_type rtype, void *dst)
{
    char *sdst = NULL;
    int *idst = NULL;
    float *fdst = NULL;

    sdst = dst;
    idst = dst;
    fdst = dst;

    char fullname[256];
    char *type;
    XrmValue ret;

    snprintf(fullname, sizeof(fullname), "%s.%s", "dwm", name);
    fullname[sizeof(fullname) - 1] = '\0';

    XrmGetResource(db, fullname, "*", &type, &ret);
    if (!(ret.addr == NULL || strncmp("String", type, 64)))
    {
        switch (rtype)
        {
        case STRING:
            strcpy(sdst, ret.addr);
            break;
        case INTEGER:
            *idst = strtoul(ret.addr, NULL, 10);
            break;
        case FLOAT:
            *fdst = strtof(ret.addr, NULL);
            break;
        }
    }
}

void
load_xresources(void)
{
    Display *display;
    char *resm;
    XrmDatabase db;
    ResourcePref *p;

    display = XOpenDisplay(NULL);
    resm = XResourceManagerString(display);
    if (!resm)
        return;

    db = XrmGetStringDatabase(resm);
    for (p = resources; p < resources + LENGTH(resources); p++)
        resource_load(db, p->name, p->type, p->dst);
    XCloseDisplay(display);
}

int
main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("dwm-" VERSION);
    else if (argc != 1)
        die("usage: dwm [-v]");
    if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
        fputs("warning: no locale support\n", stderr);
    if (!(dpy = XOpenDisplay(NULL)))
        die("dwm: cannot open display");
    if (!(xcon = XGetXCBConnection(dpy)))
        die("dwm: cannot get xcb connection\n");
    checkotherwm();
    XrmInitialize();
    load_xresources();
    setup();
#ifdef __OpenBSD__
    if (pledge("stdio rpath proc exec", NULL) == -1)
        die("pledge");
#endif /* __OpenBSD__ */
    scan();
    runAutostart();
    run();
    if (restart)
        execvp(argv[0], argv);
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}

