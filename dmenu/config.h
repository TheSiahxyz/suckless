/* See LICENSE file for copyright and license details. */
/* Default settings; can be overriden by command line. */

#define PADDING 0

static int topbar = 1;              /* -b  option; if 0, dmenu appears at bottom     */
static int fuzzy = 1;               /* -F  option; if 0, dmenu doesn't use fuzzy matching  */
static int dmx = PADDING;           /* put dmenu at this x offset */
static int dmy = PADDING;           /* put dmenu at this y offset (measured from the bottom if topbar is 0) */
static unsigned int dmw = 0;        /* make dmenu this wide */
/* -fn option overrides fonts[0]; default X11 font or font set */
static const char *fonts[] = {
    "monospace:size=10",
    "NotoColorEmoji:pixelsize=8:antialias=true:autohint=true"};
static const unsigned int bgalpha = 0xe0;
static const unsigned int fgalpha = OPAQUE;
static const char *prompt = NULL;   /* -p  option; prompt to the left of input field */
static const char *colors[SchemeLast][2] = {
    /*     fg                   bg       */
    [SchemeNorm]    = {"#bbbbbb", "#222222"},
    [SchemeSel]     = {"#eeeeee", "#005577"},
    [SchemeOut]     = {"#000000", "#00ffff"},
    [SchemeCursor]  = {"#222222", "#bbbbbb"},
};
static const unsigned int alphas[SchemeLast][2] = {
    /*  fgalpha             bgalphga    */
    [SchemeNorm]    = {fgalpha, bgalpha},
    [SchemeSel]     = {fgalpha, bgalpha},
    [SchemeOut]     = {fgalpha, bgalpha},
};

/* -l option; if nonzero, dmenu uses vertical list with given number of lines */
static unsigned int lines = 0;

/*
 * Characters not considered part of a word while deleting words
 * for example: " /?\"&[]"
 */
static const char worddelimiters[] = " ";

/*
 * -vi option; if nonzero, vi mode is always enabled and can be
 * accessed with the global_esc keysym + mod mask
 */
static unsigned int vi_mode = 1;
static unsigned int start_mode = 0;             /* mode to use when -vi is passed. 0 = insert mode, 1 = normal mode */
static Key global_esc = { XK_c, ControlMask};   /* escape key when vi mode is not enabled explicitly */
static Key quit_keys[] = {
    /* keysym   modifier */
    { XK_q,         0 },
    { XK_Escape,    0 }
};

