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
#include <ctype.h> /* for making tab label lowercase, very tiny standard library */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>
#include <X11/Xlib-xcb.h>
#include <xcb/res.h>
#ifdef __OpenBSD__
#include <sys/sysctl.h>
#include <kvm.h>
#endif /* __OpenBSD */
#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define GETINC(X)               ((X) - 2000)
#define INC(X)                  ((X) + 2000)
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISINC(X)                ((X) > 1000 && (X) < 3000)
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define MOD(N,M)                ((N)%(M) < 0 ? (N)%(M) + (M) : (N)%(M))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define PREVSEL                 3000
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define NUMTAGS					        (LENGTH(tags) + LENGTH(scratchpads))
#define OPAQUE                  0xffU
#define SPTAG(i) 				        ((1 << LENGTH(tags)) << (i))
#define SPTAGMASK   			      (((1 << LENGTH(scratchpads))-1) << LENGTH(tags))
#define TAGMASK     			      ((1 << NUMTAGS) - 1)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)
#define TRUNC(X,A,B)            (MAX((A), MIN((X), (B))))

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel, SchemeInv, SchemeStatusNorm, SchemeStatusSel, SchemeTagsNorm, SchemeTagsSel, SchemeInfoNorm, SchemeInfoSel }; /* color schemes */
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetWMSticky, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetWMWindowsOpacity, NetClientInfo, NetLast }; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkMonNum, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
  char *gname;
 	void (*func)(const Arg *arg);
 	const Arg arg;
} Gesture;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;

struct Client {
	char name[256];
	float mina, maxa;
	float cfact;
	int x, y, w, h;
  int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
  int wasfloating, iscentered, isfixed, isfloating, isalwaysontop, isurgent, neverfocus, oldstate, isfullscreen, issticky, cantfocus, isterminal, noswallow, resizehints;
	pid_t pid;
	unsigned char expandmask;
	int expandx1, expandy1, expandx2, expandy2;
  int allowkill;
	Client *next;
	Client *snext;
	Client *swallowing;
  double opacity;
  double unfocusopacity;
	Monitor *mon;
	Window win;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
} Key;

typedef struct {
  unsigned int n;
  const Key keys[5];
	void (*func)(const Arg *);
	const Arg arg;
} Keychord;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
	char ltsymbol[16];
	char monmark[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	int gappih;           /* horizontal gap between windows */
	int gappiv;           /* vertical gap between windows */
	int gappoh;           /* horizontal outer gaps */
	int gappov;           /* vertical outer gaps */
	unsigned int borderpx;
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int showtitle;
	int showtags;
	int showlayout;
	int showstatus;
	int showfloating;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Client *tagmarked[32];
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
	unsigned int alttag;
	int ltcur; /* current layout */
	Pertag *pertag;
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
  int allowkill;
	double opacity;
  double unfocusopacity;
	int isfloating;
	int isterminal;
	int noswallow;
	int monitor;
  int resizehints;
	int bw;
} Rule;

/* Xresources preferences */
enum resource_type {
	STRING = 0,
	INTEGER = 1,
	FLOAT = 2
};

typedef struct {
	char *name;
	enum resource_type type;
	void *dst;
} ResourcePref;

/* function declarations */
static void alttab(const Arg *arg);
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void changefocusopacity(const Arg *arg);
static void changeunfocusopacity(const Arg *arg);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
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
static void focusmaster(const Arg *arg);
static void focusmon(const Arg *arg);
static void focusnext(const Arg *arg);
static void focusnthmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static pid_t getstatusbarpid();
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void keyrelease(XEvent *e);
static void killthis(Client *c);
static void killclient(const Arg *arg);
static void layoutmenu(const Arg *arg);
static void layoutscroll(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void ntoggleview(const Arg *arg);
static Monitor *numtomon(int num);
static void nview(const Arg *arg);
static void gesture(const Arg *arg);
static void movemouse(const Arg *arg);
static void movecenter(const Arg *arg);
static Client *nexttiled(Client *c);
static void opacity(Client *c, double opacity);
static void pop(Client *c);
static void propertynotify(XEvent *e);
static void pushstack(const Arg *arg);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resetlayout(const Arg *arg);
static void resetnmaster(const Arg *arg);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void resetcanfocusfloating();
static void restack(Monitor *m);
static void run(void);
static void runAutostart(void);
static void scan(void);
static void scratchpad_hide();
static void scratchpad_remove();
static void scratchpad_show();
static void scratchpad_show_client(Client *c);
static void scratchpad_show_first(int scratchNum);
static int sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setborderpx(const Arg *arg);
static void setclientstate(Client *c, long state);
static void setclienttagprop(Client *c);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setcfact(const Arg *arg);
static void setlayout(const Arg *arg);
static void setmark(Client *c);
static void setmfact(const Arg *arg);
static void setsticky(Client *c, int sticky);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sighup(int unused);
static void sigstatusbar(const Arg *arg);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static int stackpos(const Arg *arg);
static void swapclient(const Arg *arg);
static void swapfocus(const Arg *arg);
static void tag(const Arg *arg);
static void spawntag(const Arg *arg);
static void tagmon(const Arg *arg);
static void tagnthmon(const Arg *arg);
static void toggleall(const Arg *arg);
static void toggleallowkill(const Arg *arg);
static void togglealttag(const Arg *arg);
static void togglealwaysontop(const Arg *arg);
static void togglebar(const Arg *arg);
static void togglebartags(const Arg *arg);
static void togglebartitle(const Arg *arg);
static void togglebarlt(const Arg *arg);
static void togglebarstatus(const Arg *arg);
static void togglebarfloat(const Arg *arg);
static void toggleborder(const Arg *arg);
static void togglecanfocusfloating(const Arg *arg);
static void togglefloating(const Arg *arg);
static void togglefullscr(const Arg *arg);
static void togglemark(const Arg *arg);
static void togglescratch(const Arg *arg);
static void togglesticky(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggletopbar(const Arg *arg);
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
static void viewall(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static void winview(const Arg* arg);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void xinitvisual();
static void zoom(const Arg *arg);
static void load_xresources(void);
static void resource_load(XrmDatabase db, char *name, enum resource_type rtype, void *dst);

static pid_t getparentprocess(pid_t p);
static int isdescprocess(pid_t p, pid_t c);
static Client *swallowingclient(Window w);
static Client *termforwin(const Client *c);
static pid_t winpid(Window w);

/* variables */
static Client *prevclient = NULL;
static const char broken[] = "broken";
static char stext[256];
static int statusw;
static int statussig;
static pid_t statuspid = -1;
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh;               /* bar height */
static int lrpad;            /* sum of left and right padding for text */
static int vp;               /* vertical padding for bar */
static int sp;               /* side padding for bar */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[KeyRelease] = keyrelease,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Clr **tagscheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;
static Client *mark;
static xcb_connection_t *xcon;

static int useargb = 0;
static Visual *visual;
static int depth;
static Colormap cmap;

/* scratchpad */
#define SCRATCHPAD_MASK_1 (1u << sizeof tags / sizeof * tags)
#define SCRATCHPAD_MASK_2 (1u << (sizeof tags / sizeof * tags + 1))
#define SCRATCHPAD_MASK_3 (1u << (sizeof tags / sizeof * tags + 2))
static int scratchpad_hide_flag = 0;
static Client *scratchpad_last_showed_1 = NULL;
static Client *scratchpad_last_showed_2 = NULL;
static Client *scratchpad_last_showed_3 = NULL;

unsigned int currentkey = 0;

/* configuration, allows nested code to access above variables */
#include "config.h"

struct Pertag {
	unsigned int curtag, prevtag; /* current and previous tag */
	int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
	float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
	unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
	const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
	int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
	Client *sel[LENGTH(tags) + 1]; /* selected client */
};

unsigned int tagw[LENGTH(tags)];

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 28 ? -1 : 1]; };

/* function implementations */
static void
alttab(const Arg *arg) {

	view(&(Arg){ .ui = ~0 });
	focusnext(&(Arg){ .i = alt_tab_direction });

	int grabbed = 1;
	int grabbed_keyboard = 1000;
	for (int i = 0; i < 100; i += 1) {
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 1000000;

		if (grabbed_keyboard != GrabSuccess) {
			grabbed_keyboard = XGrabKeyboard(dpy, DefaultRootWindow(dpy), True,
                                       GrabModeAsync, GrabModeAsync, CurrentTime);
		}
		if (grabbed_keyboard == GrabSuccess) {
			XGrabButton(dpy, AnyButton, AnyModifier, None, False,
                  BUTTONMASK, GrabModeAsync, GrabModeAsync,
                  None, None);
			break;
		}
		nanosleep(&ts, NULL);
		if (i == 100 - 1)
			grabbed = 0;
	}

	XEvent event;
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev;

	while (grabbed) {
		XNextEvent(dpy, &event);
		switch (event.type) {
		case KeyPress:
			if (event.xkey.keycode == tabCycleKey)
				focusnext(&(Arg){ .i = alt_tab_direction });
			break;
		case KeyRelease:
			if (event.xkey.keycode == tabModKey) {
				XUngrabKeyboard(dpy, CurrentTime);
				XUngrabButton(dpy, AnyButton, AnyModifier, None);
				grabbed = 0;
				alt_tab_direction = !alt_tab_direction;
				winview(0);
			}
			break;
	    case ButtonPress:
			ev = &(event.xbutton);
			if ((m = wintomon(ev->window)) && m != selmon) {
				unfocus(selmon->sel, 1);
				selmon = m;
				focus(NULL);
			}
			if ((c = wintoclient(ev->window)))
				focus(c);
			XAllowEvents(dpy, AsyncBoth, CurrentTime);
			break;
		case ButtonRelease:
			XUngrabKeyboard(dpy, CurrentTime);
			XUngrabButton(dpy, AnyButton, AnyModifier, None);
			grabbed = 0;
			alt_tab_direction = !alt_tab_direction;
			winview(0);
			break;
		}
	}
	return;
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
	c->isfloating = 0;
	c->tags = 0;
  c->allowkill = allowkill;
	c->opacity = activeopacity;
	c->unfocusopacity = inactiveopacity;
  c->bw = borderpx;
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
    if ((!r->title || strcmp(c->name, r->title) == 0)
    && (!r->class || strcmp(class, r->class) == 0)
    && (!r->instance || strcmp(instance,r->instance) == 0))
		{
			c->isterminal = r->isterminal;
			c->noswallow  = r->noswallow;
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
      c->allowkill = r->allowkill;
			c->opacity = r->opacity;
			c->unfocusopacity = r->unfocusopacity;
      c->resizehints = r->resizehints;
			if (r->bw != -1)
				c->bw = r->bw;
			if ((r->tags & SPTAGMASK) && r->isfloating) {
				c->x = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
				c->y = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
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
  if (c->tags != SCRATCHPAD_MASK_1 && c->tags != SCRATCHPAD_MASK_2 && c->tags != SCRATCHPAD_MASK_3) {
    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : (c->mon->tagset[c->mon->seltags] & ~SPTAGMASK);
  }
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
	if (c->resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		if (!c->hintsvalid)
			updatesizehints(c);
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
	if (m)
		showhide(m->stack);
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
	if (c->noswallow && !swallowfloating && c->isfloating)
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

void
buttonpress(XEvent *e)
{
	unsigned int i, x, click;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;
	char *text, *s, ch;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	if (ev->window == selmon->barwin) {
		i = x = 0;
		unsigned int occ = 0;
		for(c = m->clients; c; c=c->next)
			occ |= c->tags == TAGMASK ? 0 : c->tags;
		do {
			/* Do not reserve space for vacant tags */
			if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
				continue;
		  if (selmon->showtags)
        x += tagw[i];
		} while (ev->x >= x && ++i < LENGTH(tags));
		if (i < LENGTH(tags) && selmon->showtags) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + TEXTW(selmon->ltsymbol) && selmon->showlayout)
			click = ClkLtSymbol;
		else if (ev->x < x + TEXTW(selmon->ltsymbol) + TEXTW(selmon->monmark))
			click = ClkMonNum;
		else if (ev->x > selmon->ww - statusw && selmon->showstatus) {
			x = selmon->ww - statusw;
			click = ClkStatusText;
			statussig = 0;
			for (text = s = stext; *s && x <= ev->x; s++) {
				if ((unsigned char)(*s) < ' ') {
					ch = *s;
					*s = '\0';
					x += TEXTW(text) - lrpad;
					*s = ch;
					text = s + 1;
					if (x >= ev->x)
						break;
					/* reset on matching signal raw byte */
					if (ch == statussig)
						statussig = 0;
					else
						statussig = ch;
				}
			}
    } else if (selmon->showtitle)
			click = ClkWinTitle;
	} else if ((c = wintoclient(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}
	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
changefocusopacity(const Arg *arg)
{
  if (!selmon->sel)
    return;
  selmon->sel->opacity+=arg->f;
  if (selmon->sel->opacity > 1.0)
    selmon->sel->opacity = 1.0;

  if (selmon->sel->opacity < 0.1)
    selmon->sel->opacity = 0.1;

  opacity(selmon->sel, selmon->sel->opacity);
}

void
changeunfocusopacity(const Arg *arg)
{
  if (!selmon->sel)
    return;
  selmon->sel->unfocusopacity+=arg->f;
  if (selmon->sel->unfocusopacity > 1.0)
    selmon->sel->unfocusopacity = 1.0;

  if (selmon->sel->unfocusopacity < 0.1)
    selmon->sel->unfocusopacity = 0.1;

  opacity(selmon->sel, selmon->sel->unfocusopacity);
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
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (!c)
		return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		|| cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
        if (cme->data.l[1] == netatom[NetWMSticky]
                || cme->data.l[2] == netatom[NetWMSticky])
            setsticky(c, (cme->data.l[0] == 1 || (cme->data.l[0] == 2 && !c->issticky)));
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (!ISVISIBLE(c)) {
			c->mon->seltags ^= 1;
			c->mon->tagset[c->mon->seltags] = c->tags;
		}
		pop(c);
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
				XMoveResizeWindow(dpy, m->barwin, m->wx + sp, m->by + vp, m->ww -  2 * sp, bh);
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
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
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
	m->showtitle = showtitle;
	m->showtags = showtags;
	m->showlayout = showlayout;
	m->showstatus = showstatus;
	m->showfloating = showfloating;
	m->topbar = topbar;
	m->ltcur = 0;
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

	/* this is actually set in updategeom, to avoid a race condition */
  // snprintf(m->monmark, sizeof(m->monmark), "(%d)", m->num);
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

	for (int i = 1; i < LENGTH(tags); i++)
		if (c == c->mon->tagmarked[i])
			c->mon->tagmarked[i] = NULL;

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

Monitor *
numtomon(int num)
{
	Monitor *m = NULL;
	int i = 0;

	for(m = mons, i=0; m->next && i < num; m = m->next){
		i++;
	}
	return m;
}

void
drawbar(Monitor *m)
{
	int x, w, wdelta, tw = 0;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;
	Client *c;
	char tagdisp[64];
	char *masterclientontag[LENGTH(tags)];

	if (!m->showbar)
		return;

	/* draw status first so it can be overdrawn by tags later */
	if ((m == &mons[mainmon] && selmon->showstatus) || (statusall && selmon->showstatus)) { /* status is only drawn on selected monitor */
		char *text, *s, ch;
    if (m == selmon)
      drw_setscheme(drw, scheme[SchemeStatusSel]);
    else
      drw_setscheme(drw, scheme[SchemeStatusNorm]);
		x = 0;
		for (text = s = stext; *s; s++) {
			if ((unsigned char)(*s) < ' ') {
				ch = *s;
				*s = '\0';
				tw = TEXTW(text) - lrpad;
				drw_text(drw, m->ww - statusw + x - 2 * sp, 0, tw, bh, 0, text, 0);
				x += tw;
				*s = ch;
				text = s + 1;
			}
		}
		tw = TEXTW(text) - lrpad + 2;
		drw_text(drw, m->ww - statusw + x - 2 * sp, 0, tw, bh, 0, text, 0);
		tw = statusw;
	}

	for (i = 0; i < LENGTH(tags); i++)
		masterclientontag[i] = NULL;

	for (c = m->clients; c; c = c->next) {
		occ |= c->tags == TAGMASK ? 0 : c->tags;
		if (c->isurgent && selmon->showtags)
			urg |= c->tags;
		for (i = 0; i < LENGTH(tags); i++)
			if (!masterclientontag[i] && c->tags & (1<<i)) {
				XClassHint ch = { NULL, NULL };
				XGetClassHint(dpy, c->win, &ch);
				masterclientontag[i] = ch.res_class;
				if (lcaselbl)
					masterclientontag[i][0] = tolower(masterclientontag[i][0]);
			}
	}
	x = 0;
	for (i = 0; i < LENGTH(tags); i++) {
		/* Do not draw vacant tags */
		if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
			continue;
		if (selmon->showtags) {
      if (taglbl && masterclientontag[i])
        snprintf(tagdisp, 64, ptagf, tags[i], masterclientontag[i]);
      else
        snprintf(tagdisp, 64, etagf, tags[i]);
      masterclientontag[i] = tagdisp;
      tagw[i] = w = TEXTW(masterclientontag[i]);
      wdelta = selmon->alttag ? abs(TEXTW(tags[i]) - TEXTW(tagsalt[i])) / 2 : 0;
      if (m == selmon)
      	drw_setscheme(drw, (m->tagset[m->seltags] & 1 << i ? tagscheme[i] : scheme[SchemeNorm]));
      else
        drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeInv : SchemeTagsNorm]);
      drw_text(drw, x, 0, w,(selmon->alttag ? bh : bh + 2), wdelta - 2 + lrpad / 2, (selmon->alttag ? tagsalt[i] : masterclientontag[i]), urg & 1 << i);
      x += w;
		}
  }

	/* draw layout indicator if selmon->showlayout */
	if (selmon->showlayout) {
		w = TEXTW(m->ltsymbol);
    drw_setscheme(drw, scheme[SchemeTagsNorm]);
		x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);
    w = TEXTW(m->monmark);
    drw_setscheme(drw, scheme[SchemeTagsNorm]);
    x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->monmark, 0);
	}

	if ((w = m->ww - tw - x) > bh) {
		if (m->sel && selmon->showtitle) {
      drw_setscheme(drw, scheme[m == selmon ? SchemeInfoSel : SchemeInfoNorm]);
			drw_text(drw, x, 0, w - 2 * sp, bh, lrpad / 2, m->sel->name, 0);
			if (m->sel->isfloating && selmon->showfloating) {
				drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
				if (m->sel->isalwaysontop)
					drw_rect(drw, x + boxs, bh - boxw, boxw, boxw, 0, 0);
      }
			if (m->sel->issticky)
				drw_polygon(drw, x + boxs, m->sel->isfloating ? boxs * 2 + boxw : boxs, stickyiconbb.x, stickyiconbb.y, boxw, boxw * stickyiconbb.y / stickyiconbb.x, stickyicon, LENGTH(stickyicon), Nonconvex, m->sel->tags & m->tagset[m->seltags]);
		} else {
			// drw_setscheme(drw, scheme[SchemeNorm]);
			drw_setscheme(drw, scheme[SchemeInfoNorm]);
			drw_rect(drw, x, 0, w - 2 * sp, bh, 1, 1);
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

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
	} else if (!c || c == selmon->sel)
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
		for (c = selmon->stack; c && (!ISVISIBLE(c) || (c->issticky && !selmon->sel->issticky)); c = c->snext);
	if (selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, 0);
	if (c) {
		if (c->cantfocus)
			return;
		if (c->mon != selmon)
			selmon = c->mon;
		if (c->isurgent)
			seturgent(c, 0);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		if (c == mark)
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColMark].pixel);
    else if (c->isfloating)
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColFloat].pixel);
		else
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		setfocus(c);
		c->opacity = MIN(1.0, MAX(0, c->opacity));
		opacity(c, c->opacity);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	selmon->pertag->sel[selmon->pertag->curtag] = c;
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
focusmaster(const Arg *arg)
{
	Client *master;

	if (selmon->nmaster > 1)
		return;
	if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
		return;

	master = nexttiled(selmon->clients);

	if (!master)
		return;

	int i;
	for (i = 0; !(selmon->tagset[selmon->seltags] & 1 << i); i++);
	i++;

	if (selmon->sel == master) {
		if (selmon->tagmarked[i] && ISVISIBLE(selmon->tagmarked[i]))
			focus(selmon->tagmarked[i]);
	} else {
		selmon->tagmarked[i] = selmon->sel;
		focus(master);
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
	focus(NULL);
}

void
layoutmenu(const Arg *arg) {
	FILE *p;
	char c[3], *s;
	int i;

	if (!(p = popen(layoutmenu_cmd, "r")))
		 return;
	s = fgets(c, sizeof(c), p);
	pclose(p);

	if (!s || *s == '\0' || c[0] == '\0')
		 return;

	i = atoi(c);
	setlayout(&((Arg) { .v = &layouts[i] }));
}

void
focusnthmon(const Arg *arg)
{
	Monitor *m;

	if (!mons->next)
		return;

	if ((m = numtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, 0);
	selmon = m;
	focus(NULL);
}

static void
focusnext(const Arg *arg) {
	Monitor *m;
	Client *c;
	m = selmon;
	c = m->sel;

	if (arg->i) {
		if (c->next)
			c = c->next;
		else
			c = m->clients;
	} else {
		Client *last = c;
		if (last == m->clients)
			last = NULL;
		for (c = m->clients; c->next != last; c = c->next);
	}
	focus(c);
	return;
}

void
focusstack(const Arg *arg)
{
	int i = stackpos(arg);
	Client *c, *p;

	if (i < 0 || (selmon->sel->isfullscreen && lockfullscreen))
		return;

	for(p = NULL, c = selmon->clients; c && (i || !ISVISIBLE(c) || c->cantfocus);
	i -= (ISVISIBLE(c) && !c->cantfocus) ? 1 : 0, p = c, c = c->next);
	focus(c ? c : p);
	restack(selmon);
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
		&da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

pid_t
getstatusbarpid()
{
	char buf[32], *str = buf, *c;
	FILE *fp;

	if (statuspid > 0) {
		snprintf(buf, sizeof(buf), "/proc/%u/cmdline", statuspid);
		if ((fp = fopen(buf, "r"))) {
			fgets(buf, sizeof(buf), fp);
			while ((c = strchr(str, '/')))
				str = c + 1;
			fclose(fp);
			if (!strcmp(str, STATUSBAR))
				return statuspid;
		}
	}
	if (!(fp = popen("pidof -s "STATUSBAR, "r")))
		return -1;
	fgets(buf, sizeof(buf), fp);
	pclose(fp);
	return strtol(buf, NULL, 10);
}

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
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
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
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
		for (i = 0; i < LENGTH(buttons); i++)
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
		/* unsigned int i, j, k; */
		unsigned int i, c, k;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		int start, end, skip;
		KeySym *syms;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		XDisplayKeycodes(dpy, &start, &end);
		syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
		if (!syms)
			return;

		for (k = start; k <= end; k++)
			for (i = 0; i < LENGTH(keychords); i++)
				/* skip modifier codes, we do that ourselves */
				if (keychords[i]->keys[currentkey].keysym == syms[(k - start) * skip])
					for (c = 0; c < LENGTH(modifiers); c++)
						XGrabKey(dpy, k,
							 keychords[i]->keys[currentkey].mod | modifiers[c],
							 root, True,
							 GrabModeAsync, GrabModeAsync);

    if (currentkey > 0)
      XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Escape), AnyModifier, root, True, GrabModeAsync, GrabModeAsync);

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
	/* unsigned int i; */
  XEvent event = *e;
  unsigned int ran = 0;
	KeySym keysym;
	XKeyEvent *ev;

  Keychord *arr1[sizeof(keychords) / sizeof(Keychord*)];
  Keychord *arr2[sizeof(keychords) / sizeof(Keychord*)];
  memcpy(arr1, keychords, sizeof(keychords));
  Keychord **rpointer = arr1;
  Keychord **wpointer = arr2;

  size_t r = sizeof(keychords)/ sizeof(Keychord*);

  while(1){
    ev = &event.xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    size_t w = 0;
    for (int i = 0; i < r; i++){
      if (keysym == (*(rpointer + i))->keys[currentkey].keysym
         && CLEANMASK((*(rpointer + i))->keys[currentkey].mod) == CLEANMASK(ev->state)
         && (*(rpointer + i))->func){
        if ((*(rpointer + i))->n == currentkey +1){
          (*(rpointer + i))->func(&((*(rpointer + i))->arg));
          ran = 1;
        } else {
          *(wpointer + w) = *(rpointer + i);
          w++;
        }
      }
    }
    currentkey++;
    if (w == 0 || ran == 1)
      break;
    grabkeys();
    while (running && !XNextEvent(dpy, &event) && !ran)
      if (event.type == KeyPress)
         break;
    r = w;
    Keychord **holder = rpointer;
    rpointer = wpointer;
    wpointer = holder;
  }
  currentkey = 0;
  grabkeys();
}

void
killthis(Client *c) {
	if (!sendevent(c, wmatom[WMDelete])) {
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
keyrelease(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);

    for (i = 0; i < LENGTH(keychords); i++)
        if (momentaryalttags
        && keychords[i]->func && keychords[i]->func == togglealttag
        && selmon->alttag
        && (keysym == keychords[i]->keys[currentkey].keysym
        || CLEANMASK(keychords[i]->keys[currentkey].mod) == CLEANMASK(ev->state)))
            keychords[i]->func(&(keychords[i]->arg));
}

void
killclient(const Arg *arg)
{
  Client *c;

	if (!selmon->sel || !selmon->sel->allowkill)
		return;

  if (!arg->ui || arg->ui == 0) {
    killthis(selmon->sel);
    return;
  }

  for (c = selmon->clients; c; c = c->next) {
    if (!ISVISIBLE(c) || (arg->ui == 1 && c == selmon->sel))
      continue;
    killthis(c);
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
  c->resizehints = resizehints;
	c->cfact = 1.0;

	updatetitle(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		applyrules(c);
		term = termforwin(c);
	}
	opacity(c, c->opacity);

	if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);
	if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->wx);
	c->y = MAX(c->y, c->mon->wy);
	c->bw = c->mon->borderpx;
	if (c->isfloating)
		c->bw = fborderpx;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	if (c == mark)
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColMark].pixel);
  else if (c->isfloating)
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColFloat].pixel);
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
		if (XGetWindowProperty(dpy, c->win, netatom[NetClientInfo], 0L, 2L, False, XA_CARDINAL,
				&atom, &format, &n, &extra, (unsigned char **)&data) == Success && n == 2) {
			c->tags = *data;
			for (m = mons; m; m = m->next) {
				if (m->num == *(data+1)) {
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
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);
	c->wasfloating = 0;
	c->expandmask = 0;
	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if (c->isfloating) {
		XRaiseWindow(dpy, c->win);
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColFloat].pixel);
  };
	attach(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (selmon->sel && selmon->sel->isfullscreen && !c->isfloating)
		setfullscreen(selmon->sel, 0);
	if (c->mon == selmon)
		unfocus(selmon->sel, 0);
	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	if (term)
		swallow(term, c);
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
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
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
gesture(const Arg *arg) {
	int x, y, dx, dy, q;
	int valid=0, listpos=0, gestpos=0, count=0;
	char move, currGest[10];
	XEvent ev;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
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
				if (count++ < 10)
					break;
				count = 0;
				dx = ev.xmotion.x - x;
				dy = ev.xmotion.y - y;
				x = ev.xmotion.x;
				y = ev.xmotion.y;

				if ( abs(dx)/(abs(dy)+1) == 0 )
					move = dy<0?'u':'d';
				else
					move = dx<0?'l':'r';

				if (move!=currGest[gestpos-1])
				{
					if (gestpos>9)
					{	ev.type++;
						break;
					}

					currGest[gestpos] = move;
					currGest[++gestpos] = '\0';

					valid = 0;
					for(q = 0; q<LENGTH(gestures); q++)
					{	if (!strcmp(currGest, gestures[q].gname))
						{	valid++;
							listpos = q;
						}
					}
				}

		}
	} while(ev.type != ButtonRelease);

	if (valid)
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
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if ((m = recttomon(nx, ny, c->w, c->h))) {
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
        if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
          resize(c, nx, ny, c->w, c->h, 1);
        else if (selmon->lt[selmon->sellt]->arrange || !c->isfloating) {
          if ((m = recttomon(ev.xmotion.x_root, ev.xmotion.y_root, 1, 1)) != selmon) {
            sendmon(c, m);
            selmon = m;
            focus(NULL);
          }

          Client *cc = c->mon->clients;
          while (1) {
            if (cc == 0) break;
            if (
             cc != c && !cc->isfloating && ISVISIBLE(cc) &&
             ev.xmotion.x_root > cc->x &&
             ev.xmotion.x_root < cc->x + cc->w &&
             ev.xmotion.y_root > cc->y &&
             ev.xmotion.y_root < cc->y + cc->h ) {
              break;
            }

            cc = cc->next;
          }

          if (cc) {
            Client *cl1, *cl2, ocl1;

            if (!selmon->lt[selmon->sellt]->arrange) return;

            cl1 = c;
            cl2 = cc;
            ocl1 = *cl1;
            strcpy(cl1->name, cl2->name);
            cl1->win = cl2->win;
            cl1->x = cl2->x;
            cl1->y = cl2->y;
            cl1->w = cl2->w;
            cl1->h = cl2->h;

            cl2->win = ocl1.win;
            strcpy(cl2->name, ocl1.name);
            cl2->x = ocl1.x;
            cl2->y = ocl1.y;
            cl2->w = ocl1.w;
            cl2->h = ocl1.h;

            selmon->sel = cl2;

            c = cc;
            focus(c);

            arrange(cl1->mon);
          }
        }
      }
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
movecenter(const Arg *arg)
{
	if (selmon->sel) {
		selmon->sel->x = selmon->sel->mon->mx + (selmon->sel->mon->mw - WIDTH(selmon->sel)) / 2;
		selmon->sel->y = selmon->sel->mon->my + (selmon->sel->mon->mh - HEIGHT(selmon->sel)) / 2;
		arrange(selmon);
	}
}

Client *
nexttiled(Client *c)
{
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
opacity(Client *c, double opacity)
{
  if (opacity > 0 && opacity < 1) {
    unsigned long real_opacity[] = { opacity * 0xffffffff };
    XChangeProperty(dpy, c->win, netatom[NetWMWindowsOpacity], XA_CARDINAL,
                    32, PropModeReplace, (unsigned char *)real_opacity,
                    1);
  } else
    XDeleteProperty(dpy, c->win, netatom[NetWMWindowsOpacity]);
}

void
pop(Client *c)
{
	int i;
	for (i = 0; !(selmon->tagset[selmon->seltags] & 1 << i); i++);
	i++;

	c->mon->tagmarked[i] = nexttiled(c->mon->clients);
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = wintoclient(ev->window))) {
		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
				(c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			c->hintsvalid = 0;
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel && selmon->showtitle)
				drawbar(c->mon);
		}
		if (ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

void
pushstack(const Arg *arg) {
	int i = stackpos(arg);
	Client *sel = selmon->sel, *c, *p;

	if (i < 0)
		return;
	else if (i == 0) {
		detach(sel);
		attach(sel);
	}
	else {
		for(p = NULL, c = selmon->clients; c; p = c, c = c->next)
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
quit(const Arg *arg)
{
	if (arg->i) restart = 1;

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
resetlayout(const Arg *arg)
{
	Arg default_layout = {.v = &layouts[0]};
	Arg default_mfact = {.f = mfact + 1};

	setlayout(&default_layout);
	setmfact(&default_mfact);
}

void
resetnmaster(const Arg *arg)
{
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = 1;
	arrange(selmon);
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
	c->expandmask = 0;
	wc.border_width = c->bw;

	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
	int x, y, ocw, och, nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocw = c->w;
	och = c->h;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			nw = MAX(ocw + (ev.xmotion.x - x), 1);
			nh = MAX(och + (ev.xmotion.y - y), 1);
			if ((m = recttomon(c->x, c->y, nw, nh))) {
				if (m != selmon)
					sendmon(c, m);
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
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

	/* raise the aot window */
	for(Monitor *m_search = mons; m_search; m_search = m_search->next){
		for(c = m_search->clients; c; c = c->next){
			if (c->isalwaysontop){
				XRaiseWindow(dpy, c->win);
				break;
			}
		}
	}

	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && ISVISIBLE(c)) {
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
runAutostart(void)
{
  system("kill $(pidof dwmblocks); killall -q dwmblocks; dwmblocks &");
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

static void
scratchpad_hide(const Arg *arg)
{
  if (scratchpad_hide_flag < 4) {
    if (arg->i == 1) {
      if (selmon->sel) {
        selmon->sel->tags = SCRATCHPAD_MASK_1;
        selmon->sel->isfloating = 1;
        focus(NULL);
        arrange(selmon);
        scratchpad_hide_flag++;
      }
    } else if (arg->i == 2) {
      if (selmon->sel) {
        selmon->sel->tags = SCRATCHPAD_MASK_2;
        selmon->sel->isfloating = 1;
        focus(NULL);
        arrange(selmon);
        scratchpad_hide_flag++;
      }
    } else if (arg->i == 3) {
      if (selmon->sel) {
        selmon->sel->tags = SCRATCHPAD_MASK_3;
        selmon->sel->isfloating = 1;
        focus(NULL);
        arrange(selmon);
        scratchpad_hide_flag++;
      }
    }
  }
}

static void
scratchpad_remove()
{
  if (selmon->sel && (scratchpad_last_showed_1 != NULL || scratchpad_last_showed_2 != NULL || scratchpad_last_showed_3 != NULL) && (selmon->sel == scratchpad_last_showed_1 || selmon->sel == scratchpad_last_showed_2 || selmon->sel == scratchpad_last_showed_3)) {
    if (scratchpad_last_showed_1 == selmon->sel) {
      scratchpad_last_showed_1 = NULL;
      scratchpad_hide_flag--;
      selmon->sel->isfloating = 0;
    } else if (scratchpad_last_showed_2 == selmon->sel) {
      scratchpad_last_showed_2 = NULL;
      scratchpad_hide_flag--;
      selmon->sel->isfloating = 0;
    } else if (scratchpad_last_showed_3 == selmon->sel) {
      scratchpad_last_showed_3 = NULL;
      scratchpad_hide_flag--;
      selmon->sel->isfloating = 0;
    }
  }
}

static void
scratchpad_show(const Arg *arg)
{
  if (arg->i == 1) {
    if (scratchpad_last_showed_1 == NULL) {
      scratchpad_show_first(arg->i);
    } else {
      if (scratchpad_last_showed_1->tags != SCRATCHPAD_MASK_1) {
        scratchpad_last_showed_1->tags = SCRATCHPAD_MASK_1;
        focus(NULL);
        arrange(selmon);
      } else {
        scratchpad_show_first(arg->i);
      }
    }
  } else if (arg->i == 2) {
    if (scratchpad_last_showed_2 == NULL) {
      scratchpad_show_first(arg->i);
    } else {
      if (scratchpad_last_showed_2->tags != SCRATCHPAD_MASK_2) {
        scratchpad_last_showed_2->tags = SCRATCHPAD_MASK_2;
        focus(NULL);
        arrange(selmon);
      } else {
        scratchpad_show_first(arg->i);
      }
    }
  } else if (arg->i == 3) {
    if (scratchpad_last_showed_3 == NULL) {
      scratchpad_show_first(arg->i);
    } else {
      if (scratchpad_last_showed_3->tags != SCRATCHPAD_MASK_3) {
        scratchpad_last_showed_3->tags = SCRATCHPAD_MASK_3;
        focus(NULL);
        arrange(selmon);
      } else {
        scratchpad_show_first(arg->i);
      }
    }
  }
}

static void
scratchpad_show_client(Client *c)
{
  c->tags = selmon->tagset[selmon->seltags];
  focus(c);
  arrange(selmon);
}

static void
scratchpad_show_first(int scratchNum)
{
  for(Client *c = selmon->clients; c !=NULL; c = c->next) {
    if (c->tags == SCRATCHPAD_MASK_1 && scratchNum == 1) {
      scratchpad_last_showed_1 = c;
      scratchpad_show_client(c);
    } else if (c->tags == SCRATCHPAD_MASK_2 && scratchNum == 2) {
      scratchpad_last_showed_2 = c;
      scratchpad_show_client(c);
    } else if (c->tags == SCRATCHPAD_MASK_3 && scratchNum == 3) {
      scratchpad_last_showed_3 = c;
      scratchpad_show_client(c);
    }
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
  c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
  c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
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
setborderpx(const Arg *arg)
{
	Client *c;
	int prev_borderpx = selmon->borderpx;

	if (arg->i == 0)
		selmon->borderpx = borderpx;
	else if (selmon->borderpx + arg->i < 0)
		selmon->borderpx = 0;
	else
		selmon->borderpx += arg->i;

	for (c = selmon->clients; c; c = c->next)
	{
		if (c->bw + arg->i < 0)
			c->bw = selmon->borderpx = 0;
		else
			c->bw = selmon->borderpx;
		if (c->isfloating || !selmon->lt[selmon->sellt]->arrange)
		{
			if (arg->i != 0 && prev_borderpx + arg->i >= 0)
				resize(c, c->x, c->y, c->w-(arg->i*2), c->h-(arg->i*2), 0);
			else if (arg->i != 0)
				resizeclient(c, c->x, c->y, c->w, c->h);
			else if (prev_borderpx > borderpx)
				resize(c, c->x, c->y, c->w + 2*(prev_borderpx - borderpx), c->h + 2*(prev_borderpx - borderpx), 0);
			else if (prev_borderpx < borderpx)
				resize(c, c->x, c->y, c->w-2*(borderpx - prev_borderpx), c->h-2*(borderpx - prev_borderpx), 0);
		}
	}
	arrange(selmon);
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

int
sendevent(Client *c, Atom proto)
{
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;

	if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while (!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if (exists) {
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
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}
	sendevent(c, wmatom[WMTakeFocus]);
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

void
layoutscroll(const Arg *arg)
{
	if (!arg || !arg->i)
		return;
	int switchto = selmon->ltcur + arg->i;
	int l = LENGTH(layouts);

	if (switchto == l)
		switchto = 0;
	else if (switchto < 0)
		switchto = l - 1;

	selmon->ltcur = switchto;
	Arg arg2 = {.v= &layouts[switchto] };
	setlayout(&arg2);
}

void
	 setsticky(Client *c, int sticky)
	 {

		 if (sticky && !c->issticky) {
			 XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
					 PropModeReplace, (unsigned char *) &netatom[NetWMSticky], 1);
			 c->issticky = 1;
		 } else if (!sticky && c->issticky){
			 XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
					 PropModeReplace, (unsigned char *)0, 0);
			 c->issticky = 0;
			 arrange(c->mon);
		 }
	 }


void
setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt]) {
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
		if (!selmon->lt[selmon->sellt]->arrange) {
			for (Client *c = selmon->clients ; c ; c = c->next) {
				if (!c->isfloating) {
					/*restore last known float dimensions*/
					resize(c, selmon->mx + c->sfx, selmon->my + c->sfy,
					       c->sfw, c->sfh, 0);
				}
			}
		}
	}
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);
}

void
setcfact(const Arg *arg) {
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
	struct sigaction sa;

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while (waitpid(-1, NULL, WNOHANG) > 0);

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
	bh = drw->fonts->h + user_bh;
	sp = sidepad;
	vp = (topbar == 1) ? vertpad : - vertpad;
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
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMSticky] = XInternAtom(dpy, "_NET_WM_STATE_STICKY", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	netatom[NetClientInfo] = XInternAtom(dpy, "_NET_CLIENT_INFO", False);
	netatom[NetWMWindowsOpacity] = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */
	if (LENGTH(tags) > LENGTH(tagsel))
		die("too few color schemes for the tags");
	scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 5);
	tagscheme = ecalloc(LENGTH(tagsel), sizeof(Clr *));
	for (i = 0; i < LENGTH(tagsel); i++)
		tagscheme[i] = drw_scm_create(drw, tagsel[i], tagalpha, 2);
	/* init bars */
	updatebars();
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
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	XDeleteProperty(dpy, root, netatom[NetClientInfo]);
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
	if (ISVISIBLE(c)) {
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

void
sigstatusbar(const Arg *arg)
{
	union sigval sv;

	if (!statussig)
		return;
	sv.sival_int = arg->i;
	if ((statuspid = getstatusbarpid()) <= 0)
		return;

	sigqueue(statuspid, SIGRTMIN+statussig, sv);
}

void
spawn(const Arg *arg)
{
	struct sigaction sa;

	if (arg->v == dmenucmd)
		dmenumon[0] = '0' + selmon->num;
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

void
setclienttagprop(Client *c)
{
	long data[] = { (long) c->tags, (long) c->mon->num };
	XChangeProperty(dpy, c->win, netatom[NetClientInfo], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 2);
}

void
swapclient(const Arg *arg)
{
  if (!selmon || !selmon->sel || !mons->next)
    return;

  Monitor *m1 = selmon;
  Monitor *m2 = dirtomon(+1);

  Client *c1 = m1->sel;
  Client *c2 = m2->sel;

  if (!c2) {
    detach(c1);
    detachstack(c1);
    c1->mon = m2;
    attach(c1);
    attachstack(c1);
    focus(c1);
    selmon = m2;
    arrange(m1);
    arrange(m2);
    return;
  }

  detach(c1);
  detachstack(c1);
  detach(c2);
  detachstack(c2);

  c1->mon = m2;
  attach(c1);
  attachstack(c1);
  focus(c1);

  c2->mon = m1;
  attach(c2);
  attachstack(c2);
  focus(c2);

  selmon = m1;
  arrange(m1);
  arrange(m2);
}

void
swapfocus(const Arg *arg)
{
	Client *c, *t;

	for(c = selmon->clients; c && c != prevclient; c = c->next) ;
	if (c == prevclient) {
		focus(prevclient);
		restack(prevclient->mon);
	}

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

int
stackpos(const Arg *arg) {
	int n, i;
	Client *c, *l;

	if (!selmon->clients)
		return -1;

	if (arg->i == PREVSEL) {
		for(l = selmon->stack; l && (!ISVISIBLE(l) || l == selmon->sel); l = l->snext);
		if (!l)
			return -1;
		for(i = 0, c = selmon->clients; c != l; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return i;
	}
	else if (ISINC(arg->i)) {
		if (!selmon->sel)
			return -1;
		for(i = 0, c = selmon->clients; c != selmon->sel; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		for(n = i; c; n += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return MOD(i + GETINC(arg->i), n);
	}
	else if (arg->i < 0) {
		for(i = 0, c = selmon->clients; c; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return MAX(i + arg->i, 0);
	}
	else
		return arg->i;
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
tagnthmon(const Arg *arg)
{
	if (!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, numtomon(arg->i));
}


void
toggleall(const Arg *arg)
{
	int i;
	unsigned int tmptag;

	Monitor* m;
	for(m = mons; m; m = m->next){

		if ((arg->ui & TAGMASK) == m->tagset[m->seltags])
			return;
		m->seltags ^= 1; /* toggle sel tagset */
		if (arg->ui & TAGMASK) {
			m->tagset[m->seltags] = arg->ui & TAGMASK;
			m->pertag->prevtag = m->pertag->curtag;

			if (arg->ui == ~0)
				m->pertag->curtag = 0;
			else {
				for (i = 0; !(arg->ui & 1 << i); i++) ;
				m->pertag->curtag = i + 1;
			}
		} else {
			tmptag = m->pertag->prevtag;
			m->pertag->prevtag = m->pertag->curtag;
			m->pertag->curtag = tmptag;
		}

		m->nmaster = m->pertag->nmasters[m->pertag->curtag];
		m->mfact = m->pertag->mfacts[m->pertag->curtag];
		m->sellt = m->pertag->sellts[m->pertag->curtag];
		m->lt[m->sellt] = m->pertag->ltidxs[m->pertag->curtag][m->sellt];
		m->lt[m->sellt^1] = m->pertag->ltidxs[m->pertag->curtag][m->sellt^1];

		if (m->showbar != m->pertag->showbars[m->pertag->curtag])
			togglebar(NULL);

		focus(NULL);
		arrange(m);
	}
}

void
togglealttag(const Arg *arg)
{
	selmon->alttag = !selmon->alttag;
	drawbar(selmon);
}

void
togglebar(const Arg *arg)
{
	selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx + sp, selmon->by + vp, selmon->ww - 2 * sp, bh);
	arrange(selmon);
}

void
toggleallowkill(const Arg *arg)
{
  if (!selmon->sel) return;
  selmon->sel->allowkill = !selmon->sel->allowkill;
}

void
togglebartags(const Arg *arg)
{
  selmon->showtags = !selmon->showtags;
	arrange(selmon);
}

void
togglebartitle(const Arg *arg)
{
  selmon->showtitle = !selmon->showtitle;
	arrange(selmon);
}

void
togglebarlt(const Arg *arg)
{
  selmon->showlayout = !selmon->showlayout;
	arrange(selmon);
}

void
togglebarstatus(const Arg *arg)
{
  selmon->showstatus = !selmon->showstatus;
	arrange(selmon);
}

void
togglebarfloat(const Arg *arg)
{
  selmon->showfloating = !selmon->showfloating;
	arrange(selmon);
}

void
toggleborder(const Arg *arg)
{
  selmon->sel->bw = (selmon->sel->bw == borderpx ? 0 : borderpx);
  arrange(selmon);
}

void
toggletopbar(const Arg *arg)
{
  selmon->topbar = !selmon->topbar;
	updatebarpos(selmon);

	if (selmon->topbar)
	  XMoveResizeWindow(dpy, selmon->barwin, selmon->wx + sp, selmon->by + vp, selmon->ww - 2 * sp, bh);
	else
	  XMoveResizeWindow(dpy, selmon->barwin, selmon->wx + sp, selmon->by - vp, selmon->ww - 2 * sp, bh);

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
		XSetWindowBorder(dpy, selmon->sel->win, scheme[SchemeSel][ColFloat].pixel);
	else
		XSetWindowBorder(dpy, selmon->sel->win, scheme[SchemeSel][ColBorder].pixel);
	if (selmon->sel->isfloating) {
		selmon->sel->bw = fborderpx;
		configure(selmon->sel);
    int borderdiff = (fborderpx - borderpx) * 2;
		/*restore last known float dimensions*/
		resize(selmon->sel, selmon->mx + selmon->sel->sfx, selmon->my + selmon->sel->sfy,
		       selmon->sel->sfw - borderdiff, selmon->sel->sfh - borderdiff, 0);
	} else {
		selmon->sel->isalwaysontop = 0; /* disabled, turn this off too */
		selmon->sel->bw = borderpx;
		configure(selmon->sel);
		if (selmon->sel->isfullscreen)
			setfullscreen(selmon->sel, 0);
		/*save last known float dimensions*/
		selmon->sel->sfx = selmon->sel->x - selmon->mx;
		selmon->sel->sfy = selmon->sel->y - selmon->my;
		selmon->sel->sfw = selmon->sel->w;
		selmon->sel->sfh = selmon->sel->h;
	}

  resetcanfocusfloating();

  selmon->sel->x = selmon->sel->mon->mx + (selmon->sel->mon->mw - WIDTH(selmon->sel)) / 2;
  selmon->sel->y = selmon->sel->mon->my + (selmon->sel->mon->mh - HEIGHT(selmon->sel)) / 2;

	arrange(selmon);
}

void
togglealwaysontop(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (selmon->sel->isfullscreen)
		return;

	if (selmon->sel->isalwaysontop){
		selmon->sel->isalwaysontop = 0;
	} else {
		/* disable others */
		for(Monitor *m = mons; m; m = m->next)
			for(Client *c = m->clients; c; c = c->next)
				c->isalwaysontop = 0;

		/* turn on, make it float too */
		selmon->sel->isfloating = 1;
		selmon->sel->isalwaysontop = 1;
	}
 	arrange(selmon);
}


void
resetcanfocusfloating()
{
	unsigned int i, n;
	Client *c;

	for (n = 0, c = selmon->clients; c; c = c->next, n++);
	if (n == 0)
		return;

	for (i = 0, c = selmon->clients; c; c = c->next, i++)
    if (c->isfloating)
      c->cantfocus = 0;

	arrange(selmon);
}

void
togglecanfocusfloating(const Arg *arg)
{
	unsigned int n;
	Client *c, *cf = NULL;

  if (!selmon->sel)
      return;

  for (c = selmon->clients; c; c = c->next)
      if (c->cantfocus == 1) {
          cf = c;
      }

  if (cf) {
      resetcanfocusfloating();
      focus(cf);
  } else {
    for (n = 0, c = selmon->clients; c; c = c->next)
        if (c->isfloating)
            c->cantfocus = !c->cantfocus;
        else
            n++;

    if (n && selmon->sel->isfloating) {
        c = nexttiled(selmon->clients);
        focus(c);
    }
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
togglesticky(const Arg *arg)
{
	if (!selmon->sel)
		return;
	setsticky(selmon->sel, !selmon->sel->issticky);
	arrange(selmon);
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
ntoggleview(const Arg *arg)
{
	const Arg n = {.i = +1};
	const int mon = selmon->num;
	do {
		focusmon(&n);
		toggleview(arg);
	}
	while (selmon->num != mon);
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
unfocus(Client *c, int setfocus)
{
	if (!c)
		return;
	prevclient = c;
	grabbuttons(c, 0);
	c->unfocusopacity = MIN(1.0, MAX(0, c->unfocusopacity));
	opacity(c, c->unfocusopacity);
	if (c == mark)
		XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColMark].pixel);
  else if (c->isfloating)
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColFloat].pixel);
  else
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void
unmanage(Client *c, int destroyed)
{
	int i;
	Monitor *m = c->mon;
	XWindowChanges wc;

	if (c == mark)
		setmark(0);

	for (i = 0; i < LENGTH(tags) + 1; i++)
		if (c->mon->pertag->sel[i] == c)
			c->mon->pertag->sel[i] = NULL;

	if (c->swallowing) {
		unswallow(c);
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
		XSelectInput(dpy, c->win, NoEventMask);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}

  if (scratchpad_last_showed_1 == c) {
    scratchpad_last_showed_1 = NULL;
  }
  if (scratchpad_last_showed_2 == c) {
    scratchpad_last_showed_2 = NULL;
  }
  if (scratchpad_last_showed_3 == c) {
    scratchpad_last_showed_3 = NULL;
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
	}
}

void
updatebars(void)
{
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = cmap,
		.event_mask = ButtonPressMask|ExposureMask
	};

	XClassHint ch = {"dwm", "dwm"};
	for (m = mons; m; m = m->next) {
		if (m->barwin)
			continue;
		m->barwin = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, m->ww - 2 * sp, bh, 0, depth,
				InputOutput, visual,
				CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &wa);
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
	if (m->showbar) {
		m->wh = m->wh - vertpad - bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh + vertpad;
		m->wy = m->topbar ? m->wy + bh + vp : m->wy;
	} else
		m->by = -bh - vp;
}

void
updateclientlist(void)
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
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

		/* new monitors if nn > n */
		for (i = n; i < nn; i++) {
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
				/* this is ugly, but it is a race condition otherwise */
				snprintf(m->monmark, sizeof(m->monmark), "(%d)", m->num);
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
				updatebarpos(m);
			}
		/* removed monitors if n > nn */
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
	c->hintsvalid = 1;
}

void
updatestatus(void)
{
	if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)) && selmon->showstatus) {
		strcpy(stext, "dwm-"VERSION);
    statusw = TEXTW(stext) - lrpad + 2;
	} else {
		char *text, *s, ch;

		statusw  = 0;
		for (text = s = stext; *s; s++) {
			if ((unsigned char)(*s) < ' ') {
				ch = *s;
				*s = '\0';
				statusw += TEXTW(text) - lrpad;
				*s = ch;
				text = s + 1;
			}
		}
		statusw += TEXTW(text) - lrpad + 2;
	}

	statusall ? drawbars() : drawbar(selmon);
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
	if (state == netatom[NetWMSticky]) {
		setsticky(c, 1);
	}
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
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
nview(const Arg *arg)
{
	const Arg n = {.i = +1};
	const int mon = selmon->num;
	do {
		focusmon(&n);
		view(arg);
	}
	while (selmon->num != mon);
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

	focus(selmon->pertag->sel[selmon->pertag->curtag]);
	arrange(selmon);
}

pid_t
winpid(Window w)
{

	pid_t result = 0;

#ifdef __linux__
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

#endif /* __linux__ */

#ifdef __OpenBSD__
  Atom type;
  int format;
  unsigned long len, bytes;
  unsigned char *prop;
  pid_t ret;

  if (XGetWindowProperty(dpy, w, XInternAtom(dpy, "_NET_WM_PID", 0), 0, 1, False, AnyPropertyType, &type, &format, &len, &bytes, &prop) != Success || !prop)
    return 0;

  ret = *(pid_t*)prop;
  XFree(prop);
  result = ret;
#endif /* __OpenBSD__ */
	return result;
}

pid_t
getparentprocess(pid_t p)
{
	unsigned int v = 0;

#ifdef __linux__
	FILE *f;
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

	if (!(f = fopen(buf, "r")))
		return 0;

	fscanf(f, "%*u %*s %*c %u", &v);
	fclose(f);
#endif /* __linux__*/

#ifdef __OpenBSD__
	int n;
	kvm_t *kd;
	struct kinfo_proc *kp;

	kd = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, NULL);
	if (!kd)
		return 0;

	kp = kvm_getprocs(kd, KERN_PROC_PID, p, sizeof(*kp), &n);
	v = kp->p_ppid;
#endif /* __OpenBSD__ */

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
viewall(const Arg *arg)
{
	Monitor *m;

	for (m = mons; m; m = m->next) {
		m->tagset[m->seltags] = arg->ui;
		arrange(m);
	}
	focus(NULL);
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

/* Selects for the view of the focused window. The list of tags */
/* to be displayed is matched to the focused window tag list. */
void
winview(const Arg* arg){
	Window win, win_r, win_p, *win_c;
	unsigned nc;
	int unused;
	Client* c;
	Arg a;

	if (!XGetInputFocus(dpy, &win, &unused)) return;
	while(XQueryTree(dpy, win, &win_r, &win_p, &win_c, &nc)
	      && win_p != win_r) win = win_p;

	if (!(c = wintoclient(win))) return;

	a.ui = c->tags;
	view(&a);
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
xinitvisual()
{
    XVisualInfo *infos;
	XRenderPictFormat *fmt;
	int nitems;
	int i;

	XVisualInfo tpl = {
        .screen = screen,
		.depth = 32,
		.class = TrueColor
	};
	long masks = VisualScreenMask | VisualDepthMask | VisualClassMask;

	infos = XGetVisualInfo(dpy, masks, &tpl, &nitems);
	visual = NULL;
	for(i = 0; i < nitems; i ++) {
        fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
		if (fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
            visual = infos[i].visual;
			depth = infos[i].depth;
			cmap = XCreateColormap(dpy, root, visual, AllocNone);
			useargb = 1;
			break;
        }
    }

	XFree(infos);

	if (! visual) {
        visual = DefaultVisual(dpy, screen);
		depth = DefaultDepth(dpy, screen);
		cmap = DefaultColormap(dpy, screen);
    }
}

void
zoom(const Arg *arg)
{
	Client *c = selmon->sel;
	prevclient = nexttiled(selmon->clients);

	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
		return;
	if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
		if (!c || !(c = prevclient = nexttiled(c->next)))
			return;
	pop(c);
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
		switch (rtype) {
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
		die("dwm-"VERSION);
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
	if (pledge("stdio rpath proc exec ps", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
  runAutostart();
	run();
	if (restart) execvp(argv[0], argv);
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}

