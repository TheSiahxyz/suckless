/* See LICENSE file for license details. */
#define _XOPEN_SOURCE   500
#define LENGTH(X)       (sizeof X / sizeof X[0])
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <cairo/cairo-xlib.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fontconfig/fontconfig.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/Xrandr.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include <X11/keysym.h>
#include <X11/Xmd.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <Imlib2.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <pthread.h>
#include <time.h>
#include "arg.h"
#include "util.h"

#include <X11/XF86keysym.h>
#define LENGTH(X)       (sizeof X / sizeof X[0])
#define CLEANMASK(mask) (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))

static unsigned int numlockmask = 0;
char *argv0;

/* global count to prevent repeated error messages */
int count_error = 0;

static int pam_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr);
struct pam_conv pamc = {pam_conv, NULL};
char passwd[256];

enum {
	INIT,
	INPUT,
	INPUT_ALT,
	FAILED,
	CAPS,
	CAPS_ALT,
	PAM,
	NUMCOLS
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
} Passthrough;

typedef struct {
    const char *pass;
    const char *command;
} secretpass;

struct displayData{
	struct lock **locks;
	Display* dpy;
	int nscreens;
	cairo_t **crs;
	cairo_surface_t **surfaces;
};
static pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
#include "config.h"

struct lock {
	int screen;
	Window root, win;
	Pixmap pmap;
	Pixmap bgmap;
	unsigned long colors[NUMCOLS];
	unsigned int x, y;
	unsigned int xoff, yoff, mw, mh;
	Drawable drawable;
	GC gc;
	XRectangle rectangles[LENGTH(rectangles)];
};

struct xrandr {
	int active;
	int evbase;
	int errbase;
};

Imlib_Image image;

static void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

static int
pam_conv(int num_msg, const struct pam_message **msg,
		struct pam_response **resp, void *appdata_ptr)
{
	int retval = PAM_CONV_ERR;
	for(int i=0; i<num_msg; i++) {
		if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF &&
				strncmp(msg[i]->msg, "Password: ", 10) == 0) {
			struct pam_response *resp_msg = malloc(sizeof(struct pam_response));
			if (!resp_msg)
				die("malloc failed\n");
			char *password = malloc(strlen(passwd) + 1);
			if (!password)
				die("malloc failed\n");
			memset(password, 0, strlen(passwd) + 1);
			strcpy(password, passwd);
			resp_msg->resp_retcode = 0;
			resp_msg->resp = password;
			resp[i] = resp_msg;
			retval = PAM_SUCCESS;
		}
	}
	return retval;
}

#ifdef __linux__
#include <fcntl.h>
#include <linux/oom.h>

static void
dontkillme(void)
{
	FILE *f;
	const char oomfile[] = "/proc/self/oom_score_adj";

	if (!(f = fopen(oomfile, "w"))) {
		if (errno == ENOENT)
			return;
		die("slock: fopen %s: %s\n", oomfile, strerror(errno));
	}
	fprintf(f, "%d", OOM_SCORE_ADJ_MIN);
	if (fclose(f)) {
		if (errno == EACCES)
			die("slock: unable to disable OOM killer. "
			    "Make sure to suid or sgid slock.\n");
		else
			die("slock: fclose %s: %s\n", oomfile, strerror(errno));
	}
}
#endif

static void
writemessage(Display *dpy, Window win, int screen)
{
	int len, line_len, width, height, s_width, s_height, i, j, k, tab_replace, tab_size;
	int newlines;
	XftFont *fontinfo;
	XftColor xftcolor;
	XftDraw *xftdraw;
	XGlyphInfo ext_msg, ext_space;

	xftdraw = XftDrawCreate(dpy, win, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
	fontinfo = XftFontOpenName(dpy, screen, font_name);
	XftColorAllocName(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), text_color, &xftcolor);

	if (fontinfo == NULL) {
		if (count_error == 0) {
			fprintf(stderr, "slock: Unable to load font \"%s\"\n", font_name);
			count_error++;
		}
		return;
	}

	XftTextExtentsUtf8(dpy, fontinfo, (XftChar8 *) " ", 1, &ext_space);
	tab_size = 8 * ext_space.width;

	len = strlen(message);

	/* Count newlines and find max line length */
	line_len = 0;
	newlines = 0;
	for (i = j = 0; i < len; i++) {
		if (message[i] == '\n') {
			if (i - j > line_len)
				line_len = i - j;
			newlines++;
			i++;
			j = i;
		}
	}
	if (line_len == 0)
		line_len = len;

	XftTextExtentsUtf8(dpy, fontinfo, (XftChar8 *)message, line_len, &ext_msg);

#ifdef XINERAMA
	XineramaScreenInfo *xsi = NULL;
	int n = 0;
	if (XineramaIsActive(dpy))
		xsi = XineramaQueryScreens(dpy, &n);
	if (xsi && n > 0) {
		int limit = showallmonitors ? n : 1;
		for (int m = 0; m < limit; m++) {
			s_width  = xsi[m].width;
			s_height = xsi[m].height;
			height = xsi[m].y_org + s_height*3/4 - (newlines*20)/3;
			width  = xsi[m].x_org + (s_width - ext_msg.width)/2;
			for (i = j = k = 0; i <= len; i++) {
				if (i == len || message[i] == '\n') {
					tab_replace = 0;
					while (message[j] == '\t' && j < i) {
						tab_replace++;
						j++;
					}
					XftDrawStringUtf8(xftdraw, &xftcolor, fontinfo,
						width + tab_size*tab_replace, height + 20*k,
						(XftChar8 *)(message + j), i - j);
					while (i < len && message[i] == '\n') {
						i++;
						j = i;
						k++;
					}
				}
			}
		}
		XFree(xsi);
	} else {
#endif
		s_width  = DisplayWidth(dpy, screen);
		s_height = DisplayHeight(dpy, screen);
		height = s_height*3/4 - (newlines*20)/3;
		width  = (s_width - ext_msg.width)/2;
		for (i = j = k = 0; i <= len; i++) {
			if (i == len || message[i] == '\n') {
				tab_replace = 0;
				while (message[j] == '\t' && j < i) {
					tab_replace++;
					j++;
				}
				XftDrawStringUtf8(xftdraw, &xftcolor, fontinfo,
					width + tab_size*tab_replace, height + 20*k,
					(XftChar8 *)(message + j), i - j);
				while (i < len && message[i] == '\n') {
					i++;
					j = i;
					k++;
				}
			}
		}
#ifdef XINERAMA
	}
#endif

	XftFontClose(dpy, fontinfo);
	XftColorFree(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), &xftcolor);
	XftDrawDestroy(xftdraw);
}



static const char *
gethash(void)
{
	const char *hash;
	struct passwd *pw;

	/* Check if the current user has a password entry */
	errno = 0;
	if (!(pw = getpwuid(getuid()))) {
		if (errno)
			die("slock: getpwuid: %s\n", strerror(errno));
		else
			die("slock: cannot retrieve password entry\n");
	}
	hash = pw->pw_passwd;

#if HAVE_SHADOW_H
	if (!strcmp(hash, "x")) {
		struct spwd *sp;
		if (!(sp = getspnam(pw->pw_name)))
			die("slock: getspnam: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = sp->sp_pwdp;
	}
#else
	if (!strcmp(hash, "*")) {
#ifdef __OpenBSD__
		if (!(pw = getpwuid_shadow(getuid())))
			die("slock: getpwnam_shadow: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = pw->pw_passwd;
#else
		die("slock: getpwuid: cannot retrieve shadow entry. "
		    "Make sure to suid or sgid slock.\n");
#endif /* __OpenBSD__ */
	}
#endif /* HAVE_SHADOW_H */

	/* pam, store user name */
	hash = pw->pw_name;
	return hash;
}

static void
resizerectangles(struct lock *lock)
{
	int i;

	for (i = 0; i < LENGTH(rectangles); i++){
		lock->rectangles[i].x = (rectangles[i].x * logosize)
                                + lock->xoff + ((lock->mw) / 2) - (logow / 2 * logosize);
		lock->rectangles[i].y = (rectangles[i].y * logosize)
                                + lock->yoff + ((lock->mh) / 2) - (logoh / 2 * logosize);
		lock->rectangles[i].width = rectangles[i].width * logosize;
		lock->rectangles[i].height = rectangles[i].height * logosize;
	}
}

static void
drawlogo(Display *dpy, struct lock *lock, int color)
{
	lock->drawable = lock->bgmap;
	XSetForeground(dpy, lock->gc, lock->colors[color]);
#ifdef XINERAMA
	XineramaScreenInfo *info;
	int n;
	if ((info = XineramaQueryScreens(dpy, &n))) {
		int limit = showallmonitors ? n : 1;
		for (int m = 0; m < limit; m++) {
			lock->xoff = info[m].x_org;
			lock->yoff = info[m].y_org;
			lock->mw = info[m].width;
			lock->mh = info[m].height;
			resizerectangles(lock);
			XFillRectangles(dpy, lock->drawable, lock->gc, lock->rectangles, LENGTH(rectangles));
		}
		XFree(info);
	} else {
#endif
		XFillRectangles(dpy, lock->drawable, lock->gc, lock->rectangles, LENGTH(rectangles));
#ifdef XINERAMA
	}
#endif
	XCopyArea(dpy, lock->drawable, lock->win, lock->gc, 0, 0, lock->x, lock->y, 0, 0);
	XSync(dpy, False);
}

static void
refresh(Display *dpy, Window win, int screen, struct tm time, cairo_t* cr, cairo_surface_t* sfc)
{
	static char tm[64] = "";
	int xpos, ypos;
	double text_width;
	int s_width, s_height, message_height, message_lines;
	XftFont *fontinfo;

	sprintf(tm, "%02d/%02d/%02d %02d:%02d",
		time.tm_year+1900, time.tm_mon+1, time.tm_mday,
		time.tm_hour, time.tm_min);
	cairo_set_source_rgb(cr, textcolorred, textcolorgreen, textcolorblue);
	cairo_select_font_face(cr, textfamily, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, textsize);

	cairo_text_extents_t extents;
	cairo_text_extents(cr, tm, &extents);
	text_width = extents.width;

	/* Count message lines */
	message_lines = 1;
	for (int i = 0; message[i] != '\0'; i++) {
		if (message[i] == '\n')
			message_lines++;
	}

	fontinfo = XftFontOpenName(dpy, screen, font_name);

#ifdef XINERAMA
	XineramaScreenInfo *xsi = NULL;
	int n = 0;
	if (XineramaIsActive(dpy))
		xsi = XineramaQueryScreens(dpy, &n);
	if (xsi && n > 0) {
		int limit = showallmonitors ? n : 1;
		for (int m = 0; m < limit; m++) {
			s_width  = xsi[m].width;
			s_height = xsi[m].height;
			if (fontinfo != NULL) {
				int msg_y = xsi[m].y_org + s_height*3/4 - ((message_lines-1)*20)/3;
				message_height = msg_y + 20*(message_lines-1) + fontinfo->ascent + fontinfo->descent;
			} else {
				message_height = xsi[m].y_org + (s_height*3/4) + 30;
			}
			xpos = xsi[m].x_org + (s_width/2) - (text_width/2) - extents.x_bearing;
			ypos = message_height + 40;
			cairo_move_to(cr, xpos, ypos);
			cairo_show_text(cr, tm);
		}
		XFree(xsi);
	} else {
#endif
		s_width  = DisplayWidth(dpy, screen);
		s_height = DisplayHeight(dpy, screen);
		if (fontinfo != NULL) {
			int msg_y = s_height*3/4 - ((message_lines-1)*20)/3;
			message_height = msg_y + 20*(message_lines-1) + fontinfo->ascent + fontinfo->descent;
		} else {
			message_height = (s_height*3/4) + 30;
		}
		xpos = (s_width/2) - (text_width/2) - extents.x_bearing;
		ypos = message_height + 40;
		cairo_move_to(cr, xpos, ypos);
		cairo_show_text(cr, tm);
#ifdef XINERAMA
	}
#endif

	if (fontinfo != NULL)
		XftFontClose(dpy, fontinfo);
	cairo_surface_flush(sfc);
}

static void*
displayTime(void* input)
{ /*Thread that keeps track of time and refreshes it every 5 seconds */
 struct displayData* displayData=(struct displayData*)input;
 while (1){
 pthread_mutex_lock(&mutex); /*Mutex to prevent interference with refreshing screen while typing password*/
 time_t rawtime;
 time(&rawtime);
 struct tm tm = *localtime(&rawtime);
 for (int k=0;k<displayData->nscreens;k++){
	 refresh(displayData->dpy, displayData->locks[k]->win, displayData->locks[k]->screen, tm,displayData->crs[k],displayData->surfaces[k]);
	 }
 pthread_mutex_unlock(&mutex);
 sleep(5);
 }
 return NULL;
}

static void
readpw(Display *dpy, struct xrandr *rr, struct lock **locks, int nscreens,
       const char *hash,cairo_t **crs,cairo_surface_t **surfaces)
{
	XRRScreenChangeNotifyEvent *rre;
	char buf[32];
  int caps, num, screen, running, failure, oldc, retval, i, passing;
	unsigned int len, color, indicators;
	KeySym ksym;
	XEvent ev;
	pam_handle_t *pamh;

	len = 0;
	caps = 0;
	running = 1;
	failure = 0;
	oldc = INIT;

	if (!XkbGetIndicatorState(dpy, XkbUseCoreKbd, &indicators))
		caps = indicators & 1;

	while (running && !XNextEvent(dpy, &ev)) {
		if (ev.type == KeyPress) {
			explicit_bzero(&buf, sizeof(buf));
			num = XLookupString(&ev.xkey, buf, sizeof(buf), &ksym, 0);
			if (IsKeypadKey(ksym)) {
				if (ksym == XK_KP_Enter)
					ksym = XK_Return;
				else if (ksym >= XK_KP_0 && ksym <= XK_KP_9)
					ksym = (ksym - XK_KP_0) + XK_0;
			}
			if (IsFunctionKey(ksym) ||
			    IsKeypadKey(ksym) ||
			    IsMiscFunctionKey(ksym) ||
			    IsPFKey(ksym) ||
			    IsPrivateKeypadKey(ksym))
				continue;

			passing = 0;
			for (i = 0; i < LENGTH(passthroughs); i++) {
				if (ksym == passthroughs[i].keysym &&
						CLEANMASK(passthroughs[i].mod) == CLEANMASK(ev.xkey.state)) {
					passing = 1;
					XSendEvent(dpy, DefaultRootWindow(dpy), True, KeyPressMask, &ev);
					break;
				}
			}

			if(passing)
				continue;

			switch (ksym) {
			case XK_Return:
				passwd[len] = '\0';
				errno = 0;

				for (int i = 0; i < entrylen; i++){
					if (strcmp(scom[i].pass, passwd) == 0){
						system(scom[i].command);
					}
				}

				retval = pam_start(pam_service, hash, &pamc, &pamh);
				color = PAM;
				for (screen = 0; screen < nscreens; screen++) {
					XSetWindowBackground(dpy, locks[screen]->win, locks[screen]->colors[color]);
					XClearWindow(dpy, locks[screen]->win);
					XRaiseWindow(dpy, locks[screen]->win);
				}
				XSync(dpy, False);

				if (retval == PAM_SUCCESS)
					retval = pam_authenticate(pamh, 0);
				if (retval == PAM_SUCCESS)
					retval = pam_acct_mgmt(pamh, 0);

				running = 1;
				if (retval == PAM_SUCCESS)
					running = 0;
				else
					fprintf(stderr, "slock: %s\n", pam_strerror(pamh, retval));
				pam_end(pamh, retval);
				if (running) {
					if (xbell == 1)
						XBell(dpy, 100);
					failure = 1;
				}
				explicit_bzero(&passwd, sizeof(passwd));
				len = 0;
				break;
			case XK_Escape:
				explicit_bzero(&passwd, sizeof(passwd));
				len = 0;
				break;
			case XK_BackSpace:
				if (len)
					passwd[--len] = '\0';
				break;
			case XK_Caps_Lock:
				caps = !caps;
				break;
			default:
				if (controlkeyclear && iscntrl((int)buf[0]))
					continue;
				if (num && (len + num < sizeof(passwd))) {
					memcpy(passwd + len, buf, num);
					len += num;
				} else {
					continue; /* Don't trigger fail screen when pressing control characters */
				}
				break;
			}
      color = len ? (caps ? (len % 2 ? CAPS : CAPS_ALT) : (len % 2 ? INPUT : INPUT_ALT))
                  : ((failure || failonclear) ? FAILED : INIT);
			if (running && oldc != color) {
        pthread_mutex_lock(&mutex); /*Stop the time refresh thread from interfering*/
				for (screen = 0; screen < nscreens; screen++) {
					/* Draw background and logo first */
					drawlogo(dpy, locks[screen], color);
					/* Then draw message */
          writemessage(dpy, locks[screen]->win, screen);
					/* Finally draw time on top to keep it visible */
          time_t rawtime;
          time(&rawtime);
	        refresh(dpy, locks[screen]->win,locks[screen]->screen, *localtime(&rawtime),crs[screen],surfaces[screen]);
				}
        pthread_mutex_unlock(&mutex);
				oldc = color;
			}
		} else if (rr->active && ev.type == rr->evbase + RRScreenChangeNotify) {
			rre = (XRRScreenChangeNotifyEvent*)&ev;
		  pthread_mutex_lock(&mutex); /*Stop the time refresh thread from interfering.*/
			for (screen = 0; screen < nscreens; screen++) {
				if (locks[screen]->win == rre->window) {
					if (rre->rotation == RR_Rotate_90 ||
					    rre->rotation == RR_Rotate_270)
						XResizeWindow(dpy, locks[screen]->win,
						              rre->height, rre->width);
					else
						XResizeWindow(dpy, locks[screen]->win,
						              rre->width, rre->height);
					XClearWindow(dpy, locks[screen]->win);
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
		} else {
			for (screen = 0; screen < nscreens; screen++)
				XRaiseWindow(dpy, locks[screen]->win);
		}
	}
}

static struct lock *
lockscreen(Display *dpy, struct xrandr *rr, int screen)
{
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i, ptgrab, kbgrab;
	struct lock *lock;
	XColor color, dummy;
	XSetWindowAttributes wa;
	Cursor invisible;
#ifdef XINERAMA
	XineramaScreenInfo *info;
	int n;
#endif

	if (dpy == NULL || screen < 0 || !(lock = malloc(sizeof(struct lock))))
		return NULL;

	lock->screen = screen;
	lock->root = RootWindow(dpy, lock->screen);

  if(image)
  {
    lock->bgmap = XCreatePixmap(dpy, lock->root, DisplayWidth(dpy, lock->screen), DisplayHeight(dpy, lock->screen), DefaultDepth(dpy, lock->screen));
    imlib_context_set_image(image);
    imlib_context_set_display(dpy);
    imlib_context_set_visual(DefaultVisual(dpy, lock->screen));
    imlib_context_set_colormap(DefaultColormap(dpy, lock->screen));
    imlib_context_set_drawable(lock->bgmap);
    imlib_render_image_on_drawable(0, 0);
  }
	for (i = 0; i < NUMCOLS; i++) {
		XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen),
		                 colorname[i], &color, &dummy);
		lock->colors[i] = color.pixel;
	}

	lock->x = DisplayWidth(dpy, lock->screen);
	lock->y = DisplayHeight(dpy, lock->screen);
#ifdef XINERAMA
	if ((info = XineramaQueryScreens(dpy, &n))) {
		int idx = (screen < n) ? screen : 0;
		lock->xoff = info[idx].x_org;
		lock->yoff = info[idx].y_org;
		lock->mw = info[idx].width;
		lock->mh = info[idx].height;
		XFree(info);
	} else
#endif
	{
		lock->xoff = lock->yoff = 0;
		lock->mw = lock->x;
		lock->mh = lock->y;
	}
	lock->drawable = XCreatePixmap(dpy, lock->root,
            lock->x, lock->y, DefaultDepth(dpy, screen));
	lock->gc = XCreateGC(dpy, lock->root, 0, NULL);
	XSetLineAttributes(dpy, lock->gc, 1, LineSolid, CapButt, JoinMiter);

	/* init */
	wa.override_redirect = 1;
	lock->win = XCreateWindow(dpy, lock->root, 0, 0,
	                          lock->x, lock->y,
	                          0, DefaultDepth(dpy, lock->screen),
	                          CopyFromParent,
	                          DefaultVisual(dpy, lock->screen),
	                          CWOverrideRedirect | CWBackPixel, &wa);
  if(lock->bgmap)
    XSetWindowBackgroundPixmap(dpy, lock->win, lock->bgmap);
	lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap,
	                                &color, &color, 0, 0);
	XDefineCursor(dpy, lock->win, invisible);

	resizerectangles(lock);

	/* Try to grab mouse pointer *and* keyboard for 600ms, else fail the lock */
	for (i = 0, ptgrab = kbgrab = -1; i < 6; i++) {
		if (ptgrab != GrabSuccess) {
			ptgrab = XGrabPointer(dpy, lock->root, False,
			                      ButtonPressMask | ButtonReleaseMask |
			                      PointerMotionMask, GrabModeAsync,
			                      GrabModeAsync, None, invisible, CurrentTime);
		}
		if (kbgrab != GrabSuccess) {
			kbgrab = XGrabKeyboard(dpy, lock->root, True,
			                       GrabModeAsync, GrabModeAsync, CurrentTime);
		}

		/* input is grabbed: we can lock the screen */
		if (ptgrab == GrabSuccess && kbgrab == GrabSuccess) {
			XMapRaised(dpy, lock->win);
			if (rr->active)
				XRRSelectInput(dpy, lock->win, RRScreenChangeNotifyMask);

			XSelectInput(dpy, lock->root, SubstructureNotifyMask);
			drawlogo(dpy, lock, INIT);
			return lock;
		}

		/* retry on AlreadyGrabbed but fail on other errors */
		if ((ptgrab != AlreadyGrabbed && ptgrab != GrabSuccess) ||
		    (kbgrab != AlreadyGrabbed && kbgrab != GrabSuccess))
			break;

		usleep(100000);
	}

	/* we couldn't grab all input: fail out */
	if (ptgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab mouse pointer for screen %d\n",
		        screen);
	if (kbgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab keyboard for screen %d\n",
		        screen);
	return NULL;
}

static void
usage(void)
{
	die("usage: slock [-v] [-m message] [cmd [arg ...]]\n");
}

int
main(int argc, char **argv) {
	struct xrandr rr;
	struct lock **locks;
	struct passwd *pwd;
	struct group *grp;
	uid_t duid;
	gid_t dgid;
	const char *hash;
	Display *dpy;
	int s, nlocks, nscreens;
	CARD16 standby, suspend, off;
	BOOL dpms_state;

	ARGBEGIN {
	case 'v':
		puts("slock-"VERSION);
		return 0;
	case 'm':
		message = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND

	/* validate drop-user and -group */
	errno = 0;
	if (!(pwd = getpwnam(getenv("USER"))))
		die("slock: getpwnam %s: %s\n", getenv("USER"),
		    errno ? strerror(errno) : "user entry not found");
	duid = pwd->pw_uid;
	errno = 0;
	if (!(grp = getgrnam(getenv("USER"))))
		die("slock: getgrnam %s: %s\n", getenv("USER"),
		    errno ? strerror(errno) : "group entry not found");
	dgid = grp->gr_gid;

#ifdef __linux__
	dontkillme();
#endif

	/* the contents of hash are used to transport the current user name */
	hash = gethash();
	errno = 0;

	XInitThreads();

	if (!(dpy = XOpenDisplay(NULL)))
		die("slock: cannot open display\n");

	/* drop privileges */
	if (setgroups(0, NULL) < 0)
		die("slock: setgroups: %s\n", strerror(errno));
	if (setgid(dgid) < 0)
		die("slock: setgid: %s\n", strerror(errno));
	if (setuid(duid) < 0)
		die("slock: setuid: %s\n", strerror(errno));

  /* Load picture */
  char* home_path = getenv("HOME");
  int size_needed = snprintf(NULL, 0, "mount | grep -q ' %s/Private '", home_path) + 1;
  char* command = malloc(size_needed);
  snprintf(command, size_needed, "mount | grep -q ' %s/Private '", home_path);
  int result = system(command);
  free(command);
  if (result == 0) {
      background_image = private_image;
      blurRadius = privateBlurRadius;
  }
  if (strcmp(background_image, "") == 0) {
      background_image = ".local/share/wallpapers/lock";
  }
  size_needed = strlen(home_path) + strlen(background_image) + 2;  // +2 for slash and null terminator
  char* full_background_image = malloc(size_needed);
  strcpy(full_background_image, home_path);
  strcat(full_background_image, "/");
  strcat(full_background_image, background_image);

  Imlib_Image buffer = imlib_load_image(full_background_image);
  if (buffer) {
    /* Load picture */
    imlib_context_set_image(buffer);
    int background_image_width = imlib_image_get_width();
    int background_image_height = imlib_image_get_height();

    /* Create an image to be rendered */
    Screen *scr = ScreenOfDisplay(dpy, DefaultScreen(dpy));
    image = imlib_create_image(scr->width, scr->height);
    imlib_context_set_image(image);

    /* Fill the image for every X monitor */
    XRRMonitorInfo	*monitors;
    int number_of_monitors;
    monitors = XRRGetMonitors(dpy, RootWindow(dpy, XScreenNumberOfScreen(scr)), True, &number_of_monitors);

    int i;
    for (i = 0; i < number_of_monitors; i++) {
        imlib_blend_image_onto_image(buffer, 0, 0, 0, background_image_width, background_image_height, monitors[i].x, monitors[i].y, monitors[i].width, monitors[i].height);
    }

    /* Clean up */
    imlib_context_set_image(buffer);
    imlib_free_image();
    imlib_context_set_image(image);
  } else {
    /*Create screenshot Image*/
    Screen *scr = ScreenOfDisplay(dpy, DefaultScreen(dpy));
    image = imlib_create_image(scr->width,scr->height);
    imlib_context_set_image(image);
    imlib_context_set_display(dpy);
    imlib_context_set_visual(DefaultVisual(dpy,0));
    imlib_context_set_drawable(RootWindow(dpy,XScreenNumberOfScreen(scr)));
    imlib_copy_drawable_to_image(0,0,0,scr->width,scr->height,0,0,1);
  }

#ifdef BLUR
	/*Blur function*/
	imlib_image_blur(blurRadius);
#endif // BLUR

#ifdef PIXELATION
	/*Pixelation*/
	int width = scr->width;
	int height = scr->height;

	for(int y = 0; y < height; y += pixelSize)
	{
		for(int x = 0; x < width; x += pixelSize)
		{
			int red = 0;
			int green = 0;
			int blue = 0;

			Imlib_Color pixel;
			Imlib_Color* pp;
			pp = &pixel;
			for(int j = 0; j < pixelSize && j < height; j++)
			{
				for(int i = 0; i < pixelSize && i < width; i++)
				{
					imlib_image_query_pixel(x+i,y+j,pp);
					red += pixel.red;
					green += pixel.green;
					blue += pixel.blue;
				}
			}
			red /= (pixelSize*pixelSize);
			green /= (pixelSize*pixelSize);
			blue /= (pixelSize*pixelSize);
			imlib_context_set_color(red,green,blue,pixel.alpha);
			imlib_image_fill_rectangle(x,y,pixelSize,pixelSize);
			red = 0;
			green = 0;
			blue = 0;
		}
	}
#endif

	/* check for Xrandr support */
	rr.active = XRRQueryExtension(dpy, &rr.evbase, &rr.errbase);

	/* get number of screens in display "dpy" and blank them */
	nscreens = ScreenCount(dpy);
	if (!(locks = calloc(nscreens, sizeof(struct lock *))))
		die("slock: out of memory\n");
	for (nlocks = 0, s = 0; s < nscreens; s++) {
		if ((locks[s] = lockscreen(dpy, &rr, s)) != NULL) {
			writemessage(dpy, locks[s]->win, s);
			nlocks++;
		} else {
			break;
		}
	}
	XSync(dpy, 0);

	/* image is no longer needed after all bgmaps are set up */
	if (image) {
		imlib_context_set_image(image);
		imlib_free_image();
		image = NULL;
	}

	/* did we manage to lock everything? */
	if (nlocks != nscreens)
		return 1;

	/* DPMS magic to disable the monitor */
	if (!DPMSCapable(dpy))
		die("slock: DPMSCapable failed\n");
	if (!DPMSInfo(dpy, &standby, &dpms_state))
		die("slock: DPMSInfo failed\n");
	if (!DPMSEnable(dpy) && !dpms_state)
		die("slock: DPMSEnable failed\n");
	if (!DPMSGetTimeouts(dpy, &standby, &suspend, &off))
		die("slock: DPMSGetTimeouts failed\n");
	if (!standby || !suspend || !off)
		die("slock: at least one DPMS variable is zero\n");
	if (!DPMSSetTimeouts(dpy, monitortime, monitortime, monitortime))
		die("slock: DPMSSetTimeouts failed\n");

	XSync(dpy, 0);

	/* run post-lock command */
	if (argc > 0) {
		switch (fork()) {
		case -1:
			die("slock: fork failed: %s\n", strerror(errno));
		case 0:
			if (close(ConnectionNumber(dpy)) < 0)
				die("slock: close: %s\n", strerror(errno));
			execvp(argv[0], argv);
			fprintf(stderr, "slock: execvp %s: %s\n", argv[0], strerror(errno));
			_exit(1);
		}
	}

	/* everything is now blank. Wait for the correct password */
	pthread_t thredid;
  /* Create Cairo drawables upon which the time will be shown. */
  struct displayData displayData;
	cairo_surface_t **surfaces;
	cairo_t **crs;
    if (!(surfaces=calloc(nscreens, sizeof(cairo_surface_t*)))){
		die("Out of memory");
	}
	if (!(crs=calloc(nscreens, sizeof(cairo_t*)))){
		die("Out of memory");
	}
	for (int k=0;k<nscreens;k++){
		Drawable win=locks[k]->win;
		int screen=locks[k]->screen;
		XMapWindow(dpy, win);
		surfaces[k]=cairo_xlib_surface_create(dpy, win, DefaultVisual(dpy, screen),DisplayWidth(dpy, screen) , DisplayHeight(dpy, screen));
		crs[k]=cairo_create(surfaces[k]);
	}
	displayData.dpy=dpy;
	displayData.locks=locks;
	displayData.nscreens=nscreens;
	displayData.crs=crs;
	displayData.surfaces=surfaces;
	
	/* Draw initial time on all screens */
	time_t rawtime;
	time(&rawtime);
	struct tm tm = *localtime(&rawtime);
	for (int k = 0; k < nscreens; k++) {
		refresh(dpy, locks[k]->win, locks[k]->screen, tm, crs[k], surfaces[k]);
	}
	
  /*Start the thread that redraws time every 5 seconds*/
	pthread_create(&thredid, NULL, displayTime, &displayData);
	/*Wait for the password*/
	readpw(dpy, &rr, locks, nscreens, hash,crs,surfaces);

	for (nlocks = 0, s = 0; s < nscreens; s++) {
		XFreePixmap(dpy, locks[s]->drawable);
		XFreeGC(dpy, locks[s]->gc);
	}

	/* reset DPMS values to inital ones */
	DPMSSetTimeouts(dpy, standby, suspend, off);
	if (!dpms_state)
		DPMSDisable(dpy);

	XSync(dpy, 0);
	XCloseDisplay(dpy);
	return 0;
}

