/* See LICENSE file for copyright and license details. */

/* Default settings; can be overriden by command line. */
#define PADDING 0

static int topbar                   = 1;        /* -b  option; if 0, dmenu appears at bottom */
static int fuzzy                    = 1;        /* -F  option; if 0, dmenu doesn't use fuzzy matching */
static int dmx                      = PADDING;  /* put dmenu at this x offset */
static int dmy                      = PADDING;  /* put dmenu at this y offset (measured from the bottom if topbar is 0) */
static const unsigned int alpha     = 0xe0;     /* Amount of opacity. 0xff is opaque */
static unsigned int dmw             = 0;        /* make dmenu this wide */
static const char *dynamic          = NULL;     /* -dy option; dynamic command to run on input change */
static char *prompt                 = NULL;     /* -p  option; prompt to the left of input field */

/*
 * Characters not considered part of a word while deleting words
 * for example: " /?\"&[]"
 */
static const char worddelimiters[]  = " /?\"&[]";

/* Size of the window border */
static unsigned int border_width    = 0;

/* -l option; if nonzero, dmenu uses vertical list with given number of lines */
static unsigned int lines           = 0;
static unsigned int maxhist         = 64;
static int histnodup                = 1;	      /* if 0, record repeated histories */

/* -h option; minimum height of a menu line */
static unsigned int lineheight      = 0;
static unsigned int min_lineheight  = 8;

/*
 * -vi option; if nonzero, vi mode is always enabled and can be
 * accessed with the global_esc keysym + mod mask
 */
static unsigned int vi_mode         = 1;
static unsigned int start_mode      = 0;			  /* mode to use when -vi is passed. 0 = insert mode, 1 = normal mode */
static Key global_esc = { XK_Escape, 0};	      /* escape key when vi mode is not enabled explicitly */
static Key quit_keys[] = {
	/* keysym	modifier */
  { XK_q,         0 },
  { XK_Escape,    0 }
};

/* -fn option overrides fonts[0]; default X11 font or font set */
static char font[] = "monospace:size=10";
static const char *fonts[] = {
	font,
	"monospace:size=10",
};

static char normfgcolor[] = "#bbbbbb";
static char normbgcolor[] = "#222222";
static char selfgcolor[]  = "#eeeeee";
static char selbgcolor[]  = "#005577";
static char outfgcolor[]  = "#000000";
static char outbgcolor[]  = "#00ffff";
static char selhlfgcolor[]  = "#ffc978";
static char selhlbgcolor[]  = "#005577";
static char normhlfgcolor[]  = "#ffc978";
static char normhlbgcolor[]  = "#222222";
static char caretfgcolor[]  = "#eeeeee";
static char caretbgcolor[]  = "#222222";
static char cursorfgcolor[]  = "#222222";
static char cursorbgcolor[]  = "#bbbbbb";

static char *colors[SchemeLast][2] = {
	/*     fg                         bg       */
	[SchemeNorm]          = { normfgcolor, normbgcolor },
	[SchemeSel]           = { selfgcolor,  selbgcolor  },
	[SchemeOut]           = { outfgcolor, outbgcolor },
	[SchemeSelHighlight]  = { selhlfgcolor, selhlbgcolor },
	[SchemeNormHighlight] = { normhlfgcolor, normhlbgcolor },
	[SchemeCaret]         = { caretfgcolor, caretbgcolor },
	[SchemeCursor]        = { cursorfgcolor, cursorbgcolor},
};

static const unsigned int alphas[SchemeLast][2] = {
	[SchemeNorm]  = { OPAQUE, alpha },
	[SchemeSel]   = { OPAQUE, alpha },
	[SchemeOut]   = { OPAQUE, alpha },
};

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
	{ "font",           STRING,   &font },
	{ "normfgcolor",    STRING,   &normfgcolor },
	{ "normbgcolor",    STRING,   &normbgcolor },
	{ "selfgcolor",     STRING,   &selfgcolor },
	{ "selbgcolor",     STRING,   &selbgcolor },
	{ "outfgcolor",     STRING,   &outfgcolor },
	{ "outbgcolor",     STRING,   &outbgcolor },
	{ "selhlfgcolor",   STRING,   &selhlfgcolor },
	{ "selhlbgcolor",   STRING,   &selhlbgcolor },
	{ "normhlfgcolor",  STRING,  &normhlfgcolor },
	{ "normhlbgcolor",  STRING,  &normhlbgcolor },
	{ "caretfgcolor",   STRING,   &caretfgcolor },
	{ "caretbgcolor",   STRING,   &caretbgcolor },
	{ "cursorfgcolor",  STRING,   &cursorfgcolor },
	{ "cursorbgcolor",  STRING,   &cursorbgcolor },
	{ "prompt",         STRING,   &prompt },
};

