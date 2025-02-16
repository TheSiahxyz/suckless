/* See LICENSE file for copyright and license details. */

/* constants */
#define TERMINAL                        "st"
#define TERMCLASS                       "St"
#define BROWSER                         "qutebrowser"
#define BROWSERCLASS                    "Qutebrowser"
#define GAP                             8
#define PADDING                         0

/* appearance */
static unsigned int borderpx            = 3;        /* border pixel of windows */
static unsigned int snap                = 32;       /* snap pixel */
static unsigned int gappih              = GAP;      /* horiz inner gap between windows */
static unsigned int gappiv              = GAP;      /* vert inner gap between windows */
static unsigned int gappoh              = GAP;      /* horiz outer gap between windows and screen edge */
static unsigned int gappov              = GAP;      /* vert outer gap between windows and screen edge */
static int swallowfloating              = 0;        /* 1 means swallow floating windows by default */
static int smartgaps                    = 0;        /* 1 means no outer gap when there is only one window */
static int showbar                      = 1;        /* 0 means no bar */
static int topbar                       = 1;        /* 0 means bottom bar */
static const int allowkill              = 1;        /* allow killing clients by default? */
static const int vertpad                = PADDING;  /* vertical padding of bar */
static const int sidepad                = PADDING;  /* horizontal padding of bar */
static const char *barlayout            = "tln|s";  /* t: tag, l: layout, n: name, s: status */
static const unsigned int ulinepad	    = 5;	      /* horizontal padding between the underline and tag */
static const unsigned int ulinestroke	  = 2;	      /* thickness / height of the underline */
static const unsigned int ulinevoffset	= 0;	      /* how far above the bottom of the bar the line should appear */
static const int ulineall 		          = 0;	      /* 1 to show underline on all tags, 0 for just the active ones */

static char *fonts[] = {
  "monospace:size=10",
  "NotoColorEmoji:pixelsize=10:antialias=true:autohint=true"
};
static char normbgcolor[]               = "#222222";
static char normbordercolor[]           = "#444444";
static char normfgcolor[]               = "#bbbbbb";
static char selfgcolor[]                = "#eeeeee";
static char selbordercolor[]            = "#770000";
static char selbgcolor[]                = "#005577";
static char normmarkcolor[]             = "#009900";	/*border color for marked client*/
static char selmarkcolor[]              = "#00CC00";	/*border color for marked client on focus*/
static const unsigned int baralpha      = 0xd0;
static const unsigned int borderalpha   = OPAQUE;
static char *colors[][4] = {
  /*                      fg              bg              border          mark*/
  [SchemeNorm]    = { normfgcolor,     normbgcolor,    normbordercolor,   normmarkcolor },
  [SchemeSel]     = { selfgcolor,      selbgcolor,     selbordercolor,    selmarkcolor },
};
static const unsigned int alphas[][3] = {
  /*                  fg              bg              border  */
  [SchemeNorm]    = { OPAQUE,      baralpha,       borderalpha },
  [SchemeSel]     = { OPAQUE,      baralpha,       borderalpha },
};

typedef struct {
  const char *name;
  const void *cmd;
} Sp;

/* scratchpads */
const char *spcmd1[] = { TERMINAL, "-n", "spterm", "-g", "120x34", NULL };          /* Terminal */
const char *spcmd2[] = { TERMINAL, "-n",    "spcalc", "-f", "monospace:size=16",    /* Calculator */
                        "-g",     "50x20", "-e",     "bc", "-lq", NULL };
static Sp scratchpads[] = {
  /* name         cmd */
  { "spterm",     spcmd1 },
  { "spcalc",     spcmd2 },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
  /* xprop(1):
   *	WM_CLASS(STRING) = instance, class
   *	WM_NAME(STRING) = title
   */
  /* class        instance        title           tags mask       allowkill isfloating isterminal noswallow monitor */
  { "Gimp",       NULL,           NULL,           1 << 7,         1,  1,  0,  0, -1 },
  { "kakaotalk",  NULL,           NULL,           1 << 8,         1,  0,  0,  0, -1 },
  // { "sooptvstreamer",  NULL,      NULL,           1 << 6,         1,  0,  0,  0, -1 },
  { BROWSERCLASS, NULL,           NULL,           0,              1,  0,  0, -1, -1 },
  { TERMCLASS,    NULL,           NULL,           0,              1,  0,  1,  0, -1 },
  { TERMCLASS,    "floatterm",    NULL,           0,              1,  1,  1,  0, -1 },
  { TERMCLASS,    "bg",           NULL,           1 << 7,         1,  0,  1,  0, -1 },
  { TERMCLASS,    "spterm",       NULL,           SPTAG(0),       1,  1,  1,  0, -1 },
  { TERMCLASS,    "spcalc",       NULL,           SPTAG(1),       1,  1,  1,  0, -1 },
  { NULL,         NULL,           "Event Tester", 0,              1,  0,  0,  1, -1 },
};

/* layout(s) */
static float mfact = 0.55;              /* factor of master area size [0.05..0.95] */
static int nmaster = 1;                 /* number of clients in master area */
static int resizehints = 0;             /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1;    /* 1 will force focus on the fullscreen window */

#define FORCE_VSPLIT 1                  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

static const Layout layouts[] = {
  /* symbol   arrange function */
  { "[]=",    tile },                     /* 0: Default: Master on left, slaves on right */
  { "[M]",    monocle },                  /* 1: All windows on top of eachother */
  { "|||",    col },                      /* 2: Column */
  { "[@]",    spiral },                   /* 3: Fibonacci spiral */
  { "[\\]",   dwindle },                  /* 4: Decreasing in size right and leftward */
  { "H[]",    deck },                     /* 5: Master on left, slaves in monocle-like mode on right */
  { "TTT",    bstack },                   /* 6: Master on top, slaves on bottom horizontally */
  { "===",    bstackhoriz },              /* 7: Master on top, slaves on bottom vertically */
  { "HHH",    grid },                     /* 8: Equal grid */
  { "###",    nrowgrid },                 /* 9: Number of row grid */
  { "---",    horizgrid },                /* 10: Horizontal grid */
  { ":::",    gaplessgrid },              /* 11: Gapless grid */
  { "|M|",    centeredmaster },           /* 12: Master in middle, slaves on sides */
  { ">M>",    centeredfloatingmaster },   /* 13: Same but master floats */
  { "><>",    NULL },                     /* 14: no layout function means floating behavior */
};

/* imports */
#include <X11/XF86keysym.h>
#include "shift-tools-scratchpads.c"
#include "tagandview.c"
#include "exresize.c"

/* key definitions */
#define MODKEY Mod4Mask
#define MODKEY2 Mod1Mask
#define WINMOD (MODKEY | ShiftMask)
#define WINMOD2 (MODKEY | ControlMask)
#define ULTRAMOD (MODKEY | ControlMask | ShiftMask)
#define TERMMOD (MODKEY2 | ShiftMask)
#define TERMMOD2 (MODKEY2 | ControlMask)
#define ULTRAMOD2 (MODKEY2 | ControlMask | ShiftMask)
#define EXTRAMOD (ControlMask | ShiftMask)

#define TAGKEYS(KEY, TAG)                                   \
  { MODKEY,     KEY, view,           { .ui = 1 << TAG } },  \
  { WINMOD2,    KEY, toggleview,     { .ui = 1 << TAG } },  \
  { WINMOD,     KEY, tag,            { .ui = 1 << TAG } },  \
  { ULTRAMOD,   KEY, toggletag,      { .ui = 1 << TAG } },  \
  { TERMMOD,    KEY, tagandview,     { .ui = 1 << TAG } },

#define CTAGKEYS(KEY, TAG)                                                      \
  { {0,0,0,0},              {KEY,0,0,0}, view,          { .ui = 1 << TAG} },    \
  { {ControlMask,0,0,0},    {KEY,0,0,0}, toggleview,    { .ui = 1 << TAG} },    \
  { {ShiftMask,0,0,0},      {KEY,0,0,0}, tag,           { .ui = 1 << TAG} },    \
  { {EXTRAMOD,0,0,0},       {KEY,0,0,0}, toggletag,     { .ui = 1 << TAG} },

#define STACKKEYS(MOD, ACTION)                               \
  { MOD,  XK_j,       ACTION##stack, { .i = INC(+1) } },     \
  { MOD,  XK_k,       ACTION##stack, { .i = INC(-1) } },     \
  { MOD,  XK_Tab,     ACTION##stack, { .i = PREVSEL } },     \
  { MOD,  XK_space,   ACTION##stack, { .i = 0 } },           \
  { MOD,  XK_a,       ACTION##stack, { .i = 1 } },           \
  { MOD,  XK_z,       ACTION##stack, { .i = -1 } },

#define CSTACKKEYS(MOD, ACTION)                                           \
  { {MOD,0,0,0},  {XK_j,      0,0,0}, ACTION##stack, { .i = INC(+1) } },  \
  { {MOD,0,0,0},  {XK_k,      0,0,0}, ACTION##stack, { .i = INC(-1) } },  \
  { {MOD,0,0,0},  {XK_Tab,    0,0,0}, ACTION##stack, { .i = PREVSEL } },  \
  { {MOD,0,0,0},  {XK_space,  0,0,0}, ACTION##stack, { .i = 0 } },        \
  { {MOD,0,0,0},  {XK_a,      0,0,0}, ACTION##stack, { .i = 1 } },        \
  { {MOD,0,0,0},  {XK_z,      0,0,0}, ACTION##stack, { .i = -1 } },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char *[]) { "/bin/sh", "-c", cmd, NULL } }

/* helper for launching gtk application */
#define GTKCMD(cmd) { .v = (const char*[]){ "/usr/bin/gtk-launch", cmd, NULL } }

/* commands */
// static const char *dmenucmd[] = { "dmenu_run",    "-fn", dmenufont,   "-nb",
//                                  normbgcolor,    "-nf", normfgcolor, "-sb",
//                                  selbordercolor, "-sf", selfgcolor,  NULL };
static const char *termcmd[] = { TERMINAL, NULL };
static const char *layoutmenu_cmd = "layoutmenu";

/* Xresources preferences to load at startup */
ResourcePref resources[] = {
  { "color0", STRING, &normbordercolor },
  { "color8", STRING, &selbordercolor },
  { "color0", STRING, &normbgcolor },
  { "color4", STRING, &normfgcolor },
  { "color0", STRING, &selfgcolor },
  { "color4", STRING, &selbgcolor },
  { "borderpx", INTEGER, &borderpx },
  { "snap", INTEGER, &snap },
  { "showbar", INTEGER, &showbar },
  { "topbar", INTEGER, &topbar },
  { "nmaster", INTEGER, &nmaster },
  { "resizehints", INTEGER, &resizehints },
  { "mfact", FLOAT, &mfact },
  { "gappih", INTEGER, &gappih },
  { "gappiv", INTEGER, &gappiv },
  { "gappoh", INTEGER, &gappoh },
  { "gappov", INTEGER, &gappov },
  { "swallowfloating", INTEGER, &swallowfloating },
  { "smartgaps", INTEGER, &smartgaps },
};

/* gestures
 * u means up
 * d means down
 * l means left
 * r means right
 * ud means up and down
 */
static Gesture gestures[] = {
	{ "u",  spawn, { .v = termcmd } },
  { "d",  spawn, { .v = (const char *[]){ BROWSER, NULL } } },
  { "l",  spawn, SHCMD(TERMINAL " -e neomutt ; pkill -RTMIN+20 dwmblocks; rmdir ~/.abook 2>/dev/null") },
  { "r",  spawn, SHCMD(TERMINAL " -e newsboat ; pkill -RTMIN+19 dwmblocks") },
};

static const Arg tagexec[] = {
	{ .v = termcmd },	                                                  // 1
  { .v = (const char *[]){ BROWSER, NULL } },                         // 2
	SHCMD(TERMINAL " -e neomutt ; pkill -RTMIN+20 dwmblocks"),          // 3
	SHCMD(TERMINAL " -e newsboat ; pkill -RTMIN+19 dwmblocks"),	        // 4
	{ .v = (const char *[]){ TERMINAL, "-e", "ncmpcpp", NULL } },       // 5
	{ .v = (const char *[]){ "torwrap", NULL } },                       // 6
	{ .v = (const char *[]){ TERMINAL, "-e", "sudo", "nmtui", NULL } }, // 7
	{ .v = (const char *[]){ TERMINAL, "-e", "htop", NULL } },          // 8
	{ .v = (const char *[]){ "kakaotalk", NULL } }	                    // 9
};

static const Key keys[] = {
  // STACKKEYS
  STACKKEYS(MODKEY, focus)
  STACKKEYS(WINMOD, push)

  // TAGKEYS
  TAGKEYS(XK_1, 0)
  TAGKEYS(XK_2, 1)
  TAGKEYS(XK_3, 2)
  TAGKEYS(XK_4, 3)
  TAGKEYS(XK_5, 4)
  TAGKEYS(XK_6, 5)
  TAGKEYS(XK_7, 6)
  TAGKEYS(XK_8, 7)
  TAGKEYS(XK_9, 8)

  /* modifier     key                 function            argument */
  // AUDIO CONTROLS
  { MODKEY,       XK_m,               spawn,              SHCMD("mpc random on; mpc load entire; mpc play; sleep 1 && mpc volume 50 && pkill -RTMIN+23") },
  { WINMOD,       XK_m,               spawn,              SHCMD("mpdmenu && pkill -RTMIN+23 dwmblocks") },
  { WINMOD2,      XK_m,               spawn,              SHCMD("mpc stop; sleep 1 && mpc repeat off && mpc random off && mpc single off && mpc consume off && mpc clear") },
  { ULTRAMOD,     XK_m,               spawn,              { .v = (const char *[]){ "delmusic", NULL } } },
  { MODKEY,       XK_p,               spawn,              SHCMD("mpc toggle") },
  { WINMOD,       XK_p,               spawn,              SHCMD("mpc pause; sleep 1 && pauseallmpv") },
  { WINMOD2,      XK_p,               spawn,              SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle; sleep 1 && kill -39 $(pidof dwmblocks)") },
  { MODKEY,       XK_comma,           spawn,              { .v = (const char *[]){ "mpc", "prev", NULL } } },
  { MODKEY,       XK_period,          spawn,              { .v = (const char *[]){ "mpc", "next", NULL } } },
  { WINMOD,       XK_comma,           spawn,              { .v = (const char *[]){ "mpc", "seek", "-10", NULL } } },
  { WINMOD,       XK_period,          spawn,              { .v = (const char *[]){ "mpc", "seek", "+10", NULL } } },
  { WINMOD2,      XK_comma,           spawn,              { .v = (const char *[]){ "mpc", "seek", "-60", NULL } } },
  { WINMOD2,      XK_period,          spawn,              { .v = (const char *[]){ "mpc", "seek", "+60", NULL } } },
  { ULTRAMOD,     XK_comma,           spawn,              { .v = (const char *[]){ "mpc", "seek", "0%", NULL } } },
  { ULTRAMOD,     XK_period,          spawn,              { .v = (const char *[]){ "mpc", "repeat", NULL } } },
  { MODKEY ,      XK_slash,           spawn,              SHCMD("mpc single on; mpc random on; mpc repeat on") },
  { WINMOD,       XK_slash,           spawn,              SHCMD("mpc single off; mpc random on; mpc repeat on") },
  { WINMOD2,      XK_slash,           spawn,              SHCMD("mpc repeat off; mpc random off; mpc single off") },

  // FLOATING POSITIONS
  { ULTRAMOD2,    XK_u,               explace,            { .ui = EX_NW } },
  { ULTRAMOD2,    XK_i,               explace,            { .ui = EX_N } },
  { ULTRAMOD2,    XK_o,               explace,            { .ui = EX_NE } },
  { ULTRAMOD2,    XK_j,               explace,            { .ui = EX_W } },
  { ULTRAMOD2,    XK_k,               explace,            { .ui = EX_C } },
  { ULTRAMOD2,    XK_l,               explace,            { .ui = EX_E } },
  { ULTRAMOD2,    XK_m,               explace,            { .ui = EX_SW } },
  { ULTRAMOD2,    XK_comma,           explace,            { .ui = EX_S } },
  { ULTRAMOD2,    XK_period,          explace,            { .ui = EX_SE } },

  // FLOATING SIZES
  { TERMMOD2,     XK_h,               exresize,           { .v = (int []){ -25,   0 } } },
  { TERMMOD2,     XK_l,               exresize,           { .v = (int []){  25,   0 } } },
  { TERMMOD2,     XK_j,               exresize,           { .v = (int []){   0,  25 } } },
  { TERMMOD2,     XK_k,               exresize,           { .v = (int []){   0, -25 } } },
  { TERMMOD2,     XK_comma,           exresize,           { .v = (int []){ -25, -25 } } },
  { TERMMOD2,     XK_period,          exresize,           { .v = (int []){  25,  25 } } },

  // LAYOUTS
  { MODKEY,       XK_f,               togglefullscr,      {0} },
  { WINMOD2,      XK_f,               togglefloating,     {0} },
	{ WINMOD,       XK_h,               layoutscroll,       { .i = -1 } },
	{ WINMOD,       XK_l,               layoutscroll,       { .i = +1 } },
  { WINMOD2,      XK_s,               togglesticky,       {0} },
  { EXTRAMOD,     XK_grave,           setlayout,          { .v = &layouts[0] } },
  { EXTRAMOD,     XK_1,               setlayout,          { .v = &layouts[1] } },
  { EXTRAMOD,     XK_2,               setlayout,          { .v = &layouts[2] } },
  { EXTRAMOD,     XK_3,               setlayout,          { .v = &layouts[3] } },
  { EXTRAMOD,     XK_4,               setlayout,          { .v = &layouts[4] } },
  { EXTRAMOD,     XK_5,               setlayout,          { .v = &layouts[5] } },
  { EXTRAMOD,     XK_6,               setlayout,          { .v = &layouts[6] } },
  { EXTRAMOD,     XK_7,               setlayout,          { .v = &layouts[7] } },
  { EXTRAMOD,     XK_8,               setlayout,          { .v = &layouts[8] } },
  { EXTRAMOD,     XK_9,               setlayout,          { .v = &layouts[9] } },
  { EXTRAMOD,     XK_0,               setlayout,          { .v = &layouts[10] } },
  { EXTRAMOD,     XK_minus,           setlayout,          { .v = &layouts[11] } },
  { EXTRAMOD,     XK_equal,           setlayout,          { .v = &layouts[12] } },
  { EXTRAMOD,     XK_backslash,       setlayout,          { .v = &layouts[13] } },

  // LAYOUT SIZES
  { MODKEY,       XK_h,               setmfact,           { .f = -0.05 } },
  { MODKEY,       XK_l,               setmfact,           { .f = +0.05 } },
  { WINMOD2,      XK_j,               setcfact,           { .f = -0.25 } },
  { WINMOD2,      XK_k,               setcfact,           { .f = +0.25 } },
  { WINMOD2,      XK_l,               setcfact,           { .f = 0.00 } },
  { WINMOD,       XK_c,               incrgaps,           { .i = -5 } },
  { WINMOD2,      XK_c,               incrgaps,           { .i = +5 } },
  { WINMOD,       XK_i,               incrigaps,          { .i = -5 } },
  { WINMOD2,      XK_i,               incrigaps,          { .i = +5 } },
  { WINMOD,       XK_n,               incnmaster,         { .i = -1 } },
  { WINMOD2,      XK_n,               incnmaster,         { .i = +1 } },
  { WINMOD,       XK_o,               incrogaps,          { .i = -5 } },
  { WINMOD2,      XK_o,               incrogaps,          { .i = +5 } },
  { WINMOD,       XK_y,               incrihgaps,         { .i = -5 } },
  { WINMOD2,      XK_y,               incrihgaps,         { .i = +5 } },
  { WINMOD,       XK_t,               incrivgaps,         { .i = -5 } },
  { WINMOD2,      XK_t,               incrivgaps,         { .i = +5 } },
  { WINMOD,       XK_u,               incrohgaps,         { .i = -5 } },
  { WINMOD2,      XK_u,               incrohgaps,         { .i = +5 } },
  { WINMOD,       XK_r,               incrovgaps,         { .i = -5 } },
  { WINMOD2,      XK_r,               incrovgaps,         { .i = +5 } },
  { WINMOD,       XK_g,               defaultgaps,        {0} },
  { WINMOD2,      XK_g,               togglegaps,         {0} },
  { WINMOD,       XK_space,           zoom,               {0} },

  // MEDIA CONTROLS
  { 0, NoSymbol,                          spawn,  { .v = termcmd } },
  { 0, XF86XK_Battery,                    spawn,  SHCMD("pkill -RTMIN+4 dwmblocks") },
  { 0, XF86XK_WWW,                        spawn,  { .v = (const char *[]){ BROWSER, NULL } } },
  { 0, XF86XK_DOS,                        spawn,  { .v = termcmd } },
  { 0, XF86XK_AudioMute,                  spawn,  SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle; kill -39 $(pidof dwmblocks)") },
  { 0, XF86XK_AudioRaiseVolume,           spawn,  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 0%- && wpctl set-volume @DEFAULT_AUDIO_SINK@ 3%+; kill -39 $(pidof dwmblocks)") },
  { 0, XF86XK_AudioLowerVolume,           spawn,  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 0%+ && wpctl set-volume @DEFAULT_AUDIO_SINK@ 3%-; kill -39 $(pidof dwmblocks)") },
  { 0, XF86XK_AudioPrev,                  spawn,  { .v = (const char *[]){ "mpc", "prev", NULL } } },
  { 0, XF86XK_AudioNext,                  spawn,  { .v = (const char *[]){ "mpc", "next", NULL } } },
  { 0, XF86XK_AudioPause,                 spawn,  { .v = (const char *[]){ "mpc", "pause", NULL } } },
  { 0, XF86XK_AudioPlay,                  spawn,  { .v = (const char *[]){ "mpc", "play", NULL } } },
  { 0, XF86XK_AudioStop,                  spawn,  { .v = (const char *[]){ "mpc", "stop", NULL } } },
  { 0, XF86XK_AudioRewind,                spawn,  { .v = (const char *[]){ "mpc", "seek", "-10", NULL } } },
  { 0, XF86XK_AudioForward,               spawn,  { .v = (const char *[]){ "mpc", "seek", "+10", NULL } } },
  { 0, XF86XK_AudioMedia,                 spawn,  { .v = (const char *[]){ TERMINAL, "-e", "ncmpcpp", NULL } } },
  { 0, XF86XK_AudioMicMute,               spawn,  SHCMD("pactl set-source-mute @DEFAULT_SOURCE@ toggle") },
  { 0, XF86XK_Calculator,                 spawn,  { .v = (const char *[]){ TERMINAL, "-e", "bc", "-l", NULL } } },
  { 0, XF86XK_Launch1,                    spawn,  { .v = (const char *[]){ "xset", "dpms", "force", "off", NULL } } },
  { 0, XF86XK_Mail,                       spawn,  SHCMD(TERMINAL " -e neomutt ; pkill -RTMIN+20 dwmblocks") },
  { 0, XF86XK_MonBrightnessDown,          spawn,  SHCMD("pkexec brillo -U 5 -q; kill -43 $(pidof dwmblocks)") },
  /* { 0, XF86XK_MonBrightnessDown,	        spawn,	{.v = (const char*[]){ "xbacklight", "-dec", "15", NULL } } }, */
  { 0, XF86XK_MonBrightnessUp,            spawn,  SHCMD("pkexec brillo -A 5 -q; kill -43 $(pidof dwmblocks)") },
  /* { 0, XF86XK_MonBrightnessUp,	        spawn,  {.v = (const char*[]){ "xbacklight", "-inc", "15", NULL } } }, */
  { 0, XF86XK_MyComputer,                 spawn,  { .v = (const char *[]){ TERMINAL, "-e", "lfub", "/", NULL } } },
  { 0, XF86XK_PowerOff,                   spawn,  { .v = (const char*[]){ "sysact", NULL } } },
  { 0, XF86XK_RotateWindows,              spawn,  { .v = (const char *[]){ "tablet", NULL } } },
  { 0, XF86XK_ScreenSaver,                spawn,  SHCMD("slock & xset dpms force off; mpc pause; pauseallmpv") },
  { 0, XF86XK_Sleep,                      spawn,  { .v = (const char *[]){ "sudo", "-A", "zzz", NULL } } },
  { 0, XF86XK_TaskPane,                   spawn,  { .v = (const char *[]){ TERMINAL, "-e", "htop", NULL } } },
  { 0, XF86XK_TouchpadOff,                spawn,  { .v = (const char *[]){ "synclient", "TouchpadOff=1", NULL } } },
  { 0, XF86XK_TouchpadOn,                 spawn,  { .v = (const char *[]){ "synclient", "TouchpadOff=0", NULL } } },
  { 0, XF86XK_TouchpadToggle,             spawn,  SHCMD("(synclient | grep 'TouchpadOff.*1' && synclient TouchpadOff=0) || synclient TouchpadOff=1") },

  // MODE
  { MODKEY,       XK_Escape,          setkeymode,         { .ui = ModeCommand } },

  // MOUSE
  { ULTRAMOD,     XK_h,               spawn,              { .v = (const char *[]){ "xdotmouse", "h", NULL } } },
  { ULTRAMOD,     XK_j,               spawn,              { .v = (const char *[]){ "xdotmouse", "j", NULL } } },
  { ULTRAMOD,     XK_k,               spawn,              { .v = (const char *[]){ "xdotmouse", "k", NULL } } },
  { ULTRAMOD,     XK_l,               spawn,              { .v = (const char *[]){ "xdotmouse", "l", NULL } } },
  { ULTRAMOD,     XK_i,               spawn,              { .v = (const char *[]){ "xdotmouse", "c", NULL } } },
  { ULTRAMOD,     XK_u,               spawn,              { .v = (const char *[]){ "xdotmouse", "C", NULL } } },
  { ULTRAMOD,     XK_o,               spawn,              { .v = (const char *[]){ "xdotmouse", "m", NULL } } },

  // PROGRAMS
  { MODKEY,       XK_c,               spawn,              { .v = (const char *[]){ TERMINAL, "-e", "calcurse", NULL } } },
  { MODKEY,       XK_d,               spawn,              { .v = (const char *[]){ "dmenu_run", NULL } } },
  { MODKEY,       XK_e,               spawn,              SHCMD(TERMINAL " -e neomutt ; pkill -RTMIN+20 dwmblocks; rmdir ~/.abook 2>/dev/null") },
  { MODKEY,       XK_g,               gesture,            {0} },
  { MODKEY,       XK_n,               spawn,              SHCMD(TERMINAL " -e newsboat ; pkill -RTMIN+19 dwmblocks") },
  { MODKEY,       XK_o,               spawn,              { .v = (const char *[]){ TERMINAL, "-e", "lfub", NULL } } },
  { MODKEY,       XK_r,               spawn,              { .v = (const char *[]){ TERMINAL, "-e", "htop", NULL } } },
  { MODKEY,       XK_t,               spawn,              { .v = (const char *[]){ "torwrap", NULL } } },
  { MODKEY,       XK_w,               spawn,              { .v = (const char *[]){ BROWSER, NULL } } },
  { WINMOD,       XK_w,               spawn,              { .v = (const char *[]){ BROWSER, "--target", "private-window", NULL } } },
  { WINMOD2,      XK_w,               spawn,              { .v = (const char *[]){ "pkill", "-f", BROWSER, NULL } } },
  { MODKEY,       XK_grave,           togglescratch,      { .ui = 1 } }, // calculator //
  { WINMOD,       XK_grave,           spawn,              { .v = (const char *[]){ "dmenuunicode", NULL } } },
  { WINMOD2,      XK_grave,           spawn,              { .v = (const char *[]){ "crontog", NULL } } },
  { MODKEY,       XK_Return,          spawn,              { .v = termcmd } },
  { WINMOD,       XK_Return,          spawn,              { .v = (const char *[]){ "sd", NULL } } },
  { WINMOD2,      XK_Return,          togglescratch,      { .ui = 0 } }, // terminal //

  // SCRIPTS
  { MODKEY,       XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-t", NULL } } },
  { WINMOD,       XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-o", NULL } } },
  { ULTRAMOD,     XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-p", NULL } } },
  { WINMOD2,      XK_b,               togglebar,          {0} },
  { EXTRAMOD,     XK_b,               togglebar,          { .i = 1 } },
  { MODKEY2 ,     XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-c", NULL } } },
  { TERMMOD,      XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-b", NULL } } },
  { TERMMOD2,     XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-s", NULL } } },
  { ULTRAMOD2,    XK_b,               spawn,              { .v = (const char *[]){ "bookmarks", "-v", NULL } } },
  { WINMOD,       XK_d,               spawn,              { .v = (const char *[]){ "passmenu", NULL } } },
  { WINMOD2,      XK_d,               spawn,              { .v = (const char *[]){ "passmenu2", NULL } } },
  { WINMOD2,      XK_e,               spawn,              SHCMD("ecrypt; pkill -RTMIN+2 dwmblocks") },
  { MODKEY,       XK_s,               spawn,              { .v = (const char *[]){ "dmenubrowse", NULL } } },
  { MODKEY,       XK_v,               spawn,              { .v = (const char *[]){ "mpvplay", NULL } } },
  { WINMOD2,      XK_v,               spawn,              SHCMD("ovpn; kill -41 $(pidof dwmblocks)") },
  { MODKEY,       XK_Insert,          spawn,              SHCMD("xdotool type $(grep -v '^#' ~/.local/share/thesiah/snippets | dmenu -i -l 50 | cut -d' ' -f1)") },
  { 0,            XK_Print,           spawn,              SHCMD("maim | tee ~/Pictures/screenshots/$(date '+%y%m%d-%H%M-%S').png | xclip -selection clipboard") },
  { ShiftMask,    XK_Print,           spawn,              { .v = (const char *[]){ "maimpick", NULL } } },
  { MODKEY,       XK_Print,           spawn,              { .v = (const char *[]){ "dmenurecord", NULL } } },
  { WINMOD2,      XK_Print,           spawn,              { .v = (const char *[]){ "dmenurecord", "kill", NULL } } },
  { MODKEY,       XK_Scroll_Lock,     spawn,              SHCMD("remaps") },
  { WINMOD2,      XK_Scroll_Lock,     spawn,              SHCMD("killall screenkey || screenkey -t 3 -p fixed -s small -g 20%x5%+40%-5% --key-mode keysyms --bak-mode normal --mods-mode normal -f ttf-font-awesome --opacity 0.5 &") },
  { MODKEY,       XK_F1,              spawn,              SHCMD("groff -mom /usr/local/share/dwm/thesiah.mom -Tpdf | zathura -") },
  { WINMOD,       XK_F1,              spawn,              SHCMD("nsxiv -a ${XDG_PICTURES_DIR:-${HOME}/Pictures}/resources") },
  { WINMOD2,      XK_F1,              spawn,              { .v = (const char *[]){ "dmenuman", NULL } } },
  { MODKEY,       XK_F2,              spawn,              { .v = (const char *[]){ "tutorialvids", NULL } } },
  { MODKEY,       XK_F3,              spawn,              { .v = (const char *[]){ "displayselect", NULL } } },
  { MODKEY,       XK_F4,              spawn,              SHCMD(TERMINAL " -e pulsemixer; kill -39 $(pidof dwmblocks)") },
  { WINMOD2,      XK_F4,              spawn,              { .v = (const char *[]){ "toggleoutput", NULL } } },
  { MODKEY,       XK_F5,              xrdb,               { .v = NULL } },
  { WINMOD,       XK_F5,              spawn,              { .v = (const char *[]){ "stw", NULL } } },
  { WINMOD2,      XK_F5,              spawn,              { .v = (const char *[]){ "rbackup", NULL } } },
  { ULTRAMOD ,    XK_F5,              spawn,              { .v = (const char *[]){ "dmenuupgrade", NULL } } },
  { MODKEY,       XK_F6,              spawn,              { .v = (const char *[]){ "qndl", "-v", NULL } } },
  { WINMOD,       XK_F6,              spawn,              { .v = (const char *[]){ "qndl", "-m", NULL } } },
  { MODKEY,       XK_F7,              spawn,              { .v = (const char *[]){ "transadd", "-l", NULL } } },
  { WINMOD2,      XK_F7,              spawn,              { .v = (const char *[]){ "td-toggle", NULL } } },
  { MODKEY,       XK_F8,              spawn,              { .v = (const char *[]){ "mailsync", NULL } } },
  { MODKEY,       XK_F9,              spawn,              { .v = (const char *[]){ "mounter", NULL } } },
  { WINMOD,       XK_F9,              spawn,              { .v = (const char *[]){ "dmenumountcifs", NULL } } },
  { WINMOD2,      XK_F9,              spawn,              { .v = (const char *[]){ "unmounter", NULL } } },
  { MODKEY,       XK_F10,             spawn,              { .v = (const char *[]){ "dmenuconnections", NULL } } },
  { WINMOD,       XK_F10,             spawn,              { .v = (const char *[]){ "dmenusmbadd", NULL } } },
  { WINMOD2,      XK_F10,             spawn,              { .v = (const char *[]){ "dmenusmbdel", NULL } } },
  { MODKEY,       XK_F11,             spawn,              { .v = (const char *[]){ "webcam", "h", NULL } } },
  { WINMOD,       XK_F11,             spawn,              { .v = (const char *[]){ "webcam", NULL } } },
  { MODKEY,       XK_F12,             spawn,              SHCMD("remaps") },
  { ShiftMask,    XK_F12,             spawn,              SHCMD("remaps") },
  { MODKEY,       XK_F12,             spawn,              { .v = (const char *[]){ "fcitx5-configtool", NULL } } },

  // SYSTEMS
  { WINMOD2,      XK_k,               spawn,              { .v = (const char *[]){ "pkill", "-f", "kakaotalk", NULL } } },
  { MODKEY,       XK_q,               killclient,         {0} }, // kill only current client
  { WINMOD,       XK_q,               killclient,         { .ui = 1 } }, // kill other clients in the same tag
  { WINMOD2,      XK_q,               killclient,         { .ui = 2 } }, // kill all clients in the same tag
  { MODKEY2,      XK_q,               toggleallowkill,    {0} },
  { MODKEY,       XK_BackSpace,       spawn,              { .v = (const char *[]){ "slock", NULL } } },
  { WINMOD,       XK_BackSpace,       spawn,              { .v = (const char *[]){ "sysact", NULL } } },
  { MODKEY,       XK_minus,           spawn,              SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-; kill -39 $(pidof dwmblocks)") },
  { MODKEY,       XK_equal,           spawn,              SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+; kill -39 $(pidof dwmblocks)") },
  { WINMOD,       XK_minus,           spawn,              SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 1%-; kill -39 $(pidof dwmblocks)") },
  { WINMOD,       XK_equal,           spawn,              SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 1%+; kill -39 $(pidof dwmblocks)") },
  { WINMOD2,      XK_minus,           spawn,              SHCMD("pkexec brillo -U 5 -q; kill -43 $(pidof dwmblocks)") },
  { WINMOD2,      XK_equal,           spawn,              SHCMD("pkexec brillo -A 5 -q; kill -43 $(pidof dwmblocks)") },
  { ULTRAMOD,     XK_minus,           spawn,              SHCMD("monitorbright -dec 5; kill -42 $(pidof dwmblocks)") },
  { ULTRAMOD,     XK_equal,           spawn,              SHCMD("monitorbright -inc 5; kill -42 $(pidof dwmblocks)") },
  { 0,            XK_Alt_R,           spawn,              SHCMD("fcitx5-remote -t; kill -44 $(pidof dwmblocks)") },
  { ControlMask,  XK_F5,              quit,               {1} },
  { EXTRAMOD,     XK_F5,              spawn,              SHCMD("killall -q dwmblocks; setsid -f dwmblocks") },

  // TRAVERSALS
  { MODKEY,       XK_apostrophe,      togglemark,         {0} },
  { MODKEY2,      XK_Tab,             view,               {0} },
  { TERMMOD,      XK_Tab,             swapfocus,          {0} },
  { TERMMOD2,     XK_Tab,             swapclient,         {0} },
  { MODKEY,       XK_0,               view,               { .ui = ~0 } },
  { WINMOD,       XK_0,               tag,                { .ui = ~0 } },
  { MODKEY2,      XK_bracketleft,     shiftview,          { .i = -1 } },
  { MODKEY2,      XK_bracketright,    shiftview,          { .i = 1 } },
  { TERMMOD,      XK_bracketleft,     shifttag,           { .i = -1 } },
  { TERMMOD,      XK_bracketright,    shifttag,           { .i = 1 } },
  { TERMMOD2,     XK_bracketleft,     shifttagandview,    { .i = -1 } },
  { TERMMOD2,     XK_bracketright,    shifttagandview,    { .i = 1 } },
  { MODKEY,       XK_bracketleft,     shiftviewclients,   { .i = -1 } },
  { MODKEY,       XK_bracketright,    shiftviewclients,   { .i = +1 } },
  { WINMOD,       XK_bracketleft,     shifttagclients,    { .i = -1 } },
  { WINMOD,       XK_bracketright,    shifttagclients,    { .i = +1 } },
  { WINMOD2,      XK_bracketleft,     shiftboth,          { .i = -1 } },
  { WINMOD2,      XK_bracketright,    shiftboth,          { .i = +1 } },
  { ULTRAMOD,     XK_bracketleft,     shiftswaptags,      { .i = -1 }	},
  { ULTRAMOD,     XK_bracketright,    shiftswaptags,      { .i = +1 }	},
  { MODKEY,       XK_Left,            focusmon,           { .i = -1 } },
  { MODKEY,       XK_Right,           focusmon,           { .i = +1 } },
  { WINMOD,       XK_Left,            tagmon,             { .i = -1 } },
  { WINMOD,       XK_Right,           tagmon,             { .i = +1 } },

  /* { WINMOD,    XK_apostrophe,      togglesmartgaps,    {0} }, */
};

static Key cmdkeys[] = {
  /* modifier     keys                function        argument */
  // COMMANDS
  { 0,            XK_Escape,          clearcmd,       {0} },
  { ControlMask,  XK_c,               clearcmd,       {0} },
  { ControlMask,  XK_x,               setkeymode,     { .ui = ModeInsert } },
};

static Command commands[] = {
  // STACKKEYS
  CSTACKKEYS(MODKEY, focus)
  CSTACKKEYS(WINMOD, push)

  // TAGKEYS
  CTAGKEYS(XK_1, 0)
  CTAGKEYS(XK_2, 1)
  CTAGKEYS(XK_3, 2)
  CTAGKEYS(XK_4, 3)
  CTAGKEYS(XK_5, 4)
  CTAGKEYS(XK_6, 5)
  CTAGKEYS(XK_7, 6)
  CTAGKEYS(XK_8, 7)
  CTAGKEYS(XK_9, 8)

  /* Modifier (4 keys)        keysyms (4 keys)                    function                argument */
  // APPEARANCES
  { { ShiftMask, 0, 0, 0 },   { XK_b, 0, 0, 0 },                  togglebar,              {0} },
  { { ControlMask, 0, 0, 0 }, { XK_b, 0, 0, 0 },                  togglebar,              { .i = 1 } },
  { { ControlMask, 0, 0, 0 }, { XK_o, 0, 0, 0 },                  toggleborder,           {0} },
  { { ControlMask, 0, 0, 0 }, { XK_f, 0, 0, 0 },                  togglefloating,         {0} },
  { { ControlMask, 0, 0, 0 }, { XK_g, 0, 0, 0 },                  togglegaps,             {0} },
  { { ShiftMask, 0, 0, 0 },   { XK_g, 0, 0, 0 },                  defaultgaps,            {0} },

  // LAYOUTS
  { { 0, 0, 0, 0 },           { XK_l, XK_d, 0, 0 },               setlayout,              { .v = &layouts[0] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_s, 0, 0 },               setlayout,              { .v = &layouts[1] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_l, XK_l, 0 },            setlayout,              { .v = &layouts[2] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_a, 0, 0 },               setlayout,              { .v = &layouts[3] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_backslash, 0, 0 },       setlayout,              { .v = &layouts[4] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_h, XK_l, 0 },            setlayout,              { .v = &layouts[5] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_t, XK_t, 0 },            setlayout,              { .v = &layouts[6] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_e, XK_e, 0 },            setlayout,              { .v = &layouts[7] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_h, XK_h, 0 },            setlayout,              { .v = &layouts[8] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_3, XK_3, 0 },            setlayout,              { .v = &layouts[9] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_minus, XK_minus, 0 },    setlayout,              { .v = &layouts[10] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_period, XK_period, 0 },  setlayout,              { .v = &layouts[11] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_m, XK_l, 0 },            setlayout,              { .v = &layouts[12] } },
  { { 0, 0, 0, 0 },           { XK_l, XK_m, XK_s, 0 },            setlayout,              { .v = &layouts[13] } },

  // FLOATING POSITIONS
  { { 0, 0, 0, 0 },           { XK_f, XK_u, 0, 0 },               explace,                { .ui = EX_NW } },
  { { 0, 0, 0, 0 },           { XK_f, XK_i, 0, 0 },               explace,                { .ui = EX_N } },
  { { 0, 0, 0, 0 },           { XK_f, XK_o, 0, 0 },               explace,                { .ui = EX_NE } },
  { { 0, 0, 0, 0 },           { XK_f, XK_j, 0, 0 },               explace,                { .ui = EX_W } },
  { { 0, 0, 0, 0 },           { XK_f, XK_k, 0, 0 },               explace,                { .ui = EX_C } },
  { { 0, 0, 0, 0 },           { XK_f, XK_l, 0, 0 },               explace,                { .ui = EX_E } },
  { { 0, 0, 0, 0 },           { XK_f, XK_m, 0, 0 },               explace,                { .ui = EX_SW } },
  { { 0, 0, 0, 0 },           { XK_f, XK_comma, 0, 0 },           explace,                { .ui = EX_S } },
  { { 0, 0, 0, 0 },           { XK_f, XK_period, 0, 0 },          explace,                { .ui = EX_SE } },

  // FLOATING SIZES
  { { 0, 0, 0, 0 },           { XK_s, XK_j, 0, 0 },               exresize,               { .v = (int []){   0,  25 } } },
  { { 0, 0, 0, 0 },           { XK_s, XK_k, 0, 0 },               exresize,               { .v = (int []){   0, -25 } } },
  { { 0, 0, 0, 0 },           { XK_s, XK_l, 0, 0 },               exresize,               { .v = (int []){  25,   0 } } },
  { { 0, 0, 0, 0 },           { XK_s, XK_h, 0, 0 },               exresize,               { .v = (int []){ -25,   0 } } },
  { { 0, 0, 0, 0 },           { XK_s, XK_comma, 0, 0 },           exresize,               { .v = (int []){ -25, -25 } } },
  { { 0, 0, 0, 0 },           { XK_s, XK_period, 0, 0 },          exresize,               { .v = (int []){  25,  25 } } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_i, XK_h },         togglehorizontalexpand, { .i = +1 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_r, XK_h },         togglehorizontalexpand, { .i =  0 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_d, XK_h },         togglehorizontalexpand, { .i = -1 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_i, XK_v },         toggleverticalexpand,   { .i = +1 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_r, XK_v },         toggleverticalexpand,   { .i =  0 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_d, XK_v },         toggleverticalexpand,   { .i = -1 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_i, XK_m },         togglemaximize,         { .i = +1 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_r, XK_m },         togglemaximize,         { .i =  0 } },
  { { 0, 0, 0, 0 },           { XK_f, XK_s, XK_d, XK_m },         togglemaximize,         { .i = -1 } },

  // PROGRAMS
  { { 0, 0, 0, 0 },           { XK_a, 0, 0, 0 },              spawn,                  SHCMD(TERMINAL " -e abook -C ${XDG_CONFIG_HOME:-${HOME}/.config}/abook/abookrc --datafile ${XDG_CONFIG_HOME:-${HOME}/.config}/abook/addressbook") },
  { { 0, 0, 0, 0 },           { XK_e, 0, 0, 0 },              spawn,                  SHCMD(TERMINAL " -e neomutt ; pkill -RTMIN+20 dwmblocks; rmdir ~/.abook 2>/dev/null") },
  { { 0, 0, 0, 0 },           { XK_g, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ "gimp", NULL } } },
  { { 0, 0, 0, 0 },           { XK_i, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ TERMINAL, "-e", "nmtui", NULL } } },
  { { 0, 0, 0, 0 },           { XK_k, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ "kakaotalk", NULL } } },
  { { 0, 0, 0, 0 },           { XK_m, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ TERMINAL, "-e", "ncmpcpp", NULL } } },
  { { 0, 0, 0, 0 },           { XK_n, 0, 0, 0 },              spawn,                  SHCMD(TERMINAL " -e newsboat ; pkill -RTMIN+19 dwmblocks") },
  { { 0, 0, 0, 0 },           { XK_p, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ TERMINAL, "-e", "profanity", NULL } } },
  { { 0, 0, 0, 0 },           { XK_r, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ TERMINAL, "-e", "htop", NULL } } },
  { { 0, 0, 0, 0 },           { XK_t, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ "torwrap", NULL } } },
  { { 0, 0, 0, 0 },           { XK_v, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ "ovpn", NULL } } },
  { { ShiftMask, 0, 0, 0 },   { XK_v, 0, 0, 0 },              spawn,                  { .v = (const char *[]){ TERMINAL, "-e", "nvim", "-c", "VimwikiIndex", "1", NULL } } },
  { { 0, 0, 0, 0 },           { XK_w, 0, 0, 0 },              spawn,                  SHCMD(TERMINAL " -e less -Sf ${XDG_CACHE_HOME:-${HOME}/.cache}/weatherreport") },
  { { 0, 0, 0, 0 },           { XK_Return, 0, 0, 0 },         spawn,                  { .v = (const char *[]){ TERMINAL, "-e", "lfub", NULL } } },

  // SUCKLESS CONFIGS
  { { ShiftMask, 0, 0, 0 },   { XK_p, 0, 0, 0 },              spawn,                  SHCMD(TERMINAL " -e sc-im ${THESIAH_WWW:-${HOME}/Private/git/THESIAH}/static/progs.csv") },
  { { ShiftMask, 0, 0, 0 },   { XK_s, XK_d, XK_b, 0 },        spawn,                  SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwmblocks/config.def.h") },
  { { ShiftMask, 0, 0, 0 },   { XK_s, XK_d, XK_m, 0 },        spawn,                  SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dmenu/config.def.h") },
  { { ShiftMask, 0, 0, 0 },   { XK_s, XK_d, XK_w, 0 },        spawn,                  SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwm/config.def.h") },
  { { ShiftMask, 0, 0, 0 },   { XK_s, XK_s, XK_t, 0 },        spawn,                  SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/st/config.def.h") },
  { { ShiftMask, 0, 0, 0 },   { XK_s, XK_s, XK_l, 0 },        spawn,                  SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/slock/config.def.h") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
 * ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
  /* click            event mask    button      function        argument */
  // MOUSE BUTTONS
#ifndef __OpenBSD__
  { ClkWinTitle,      0,            Button2,    zoom,           {0} },
  { ClkStatusText,    0,            Button1,    sigdwmblocks,   { .i = 1 } },
  { ClkStatusText,    0,            Button2,    sigdwmblocks,   { .i = 2 } },
  { ClkStatusText,    0,            Button3,    sigdwmblocks,   { .i = 3 } },
  { ClkStatusText,    0,            Button4,    sigdwmblocks,   { .i = 4 } },
  { ClkStatusText,    0,            Button5,    sigdwmblocks,   { .i = 5 } },
  { ClkStatusText,    ShiftMask,    Button1,    sigdwmblocks,   { .i = 6 } },
  { ClkStatusText,    ShiftMask,    Button2,    sigdwmblocks,   { .i = 7 } },
  { ClkStatusText,    ShiftMask,    Button3,    sigdwmblocks,   { .i = 8 } },
  { ClkStatusText,    ShiftMask,    Button4,    sigdwmblocks,   { .i = 9 } },
  { ClkStatusText,    ShiftMask,    Button5,    sigdwmblocks,   { .i = 10 } },
  { ClkStatusText,    ControlMask,  Button1,    sigdwmblocks,   { .i = 11 } },
  { ClkStatusText,    ControlMask,  Button2,    sigdwmblocks,   { .i = 12 } },
  { ClkStatusText,    ControlMask,  Button3,    sigdwmblocks,   { .i = 13 } },
  { ClkStatusText,    ControlMask,  Button4,    sigdwmblocks,   { .i = 14 } },
  { ClkStatusText,    ControlMask,  Button5,    sigdwmblocks,   { .i = 15 } },
#endif
  { ClkStatusText,    MODKEY,       Button1,    spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwm/config.def.h") },
  { ClkStatusText,    WINMOD,       Button1,    spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwmblocks/config.def.h") },
  { ClkStatusText,    WINMOD2,      Button1,    spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dmenu/config.def.h") },
  { ClkStatusText,    MODKEY,       Button3,    spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/st/config.def.h") },
  { ClkStatusText,    WINMOD,       Button3,    spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/slock/config.def.h") },
  { ClkClientWin,     MODKEY,       Button1,    movemouse,      {0} },
  { ClkClientWin,     WINMOD,       Button1,    killclient,     {0} },
  { ClkClientWin,     WINMOD2,      Button1,    killclient,     { .ui = 2 } },
  { ClkClientWin,     MODKEY,       Button2,    defaultgaps,    {0} },
  { ClkClientWin,     MODKEY,       Button3,    resizemouse,    {0} },
  { ClkClientWin,     WINMOD,       Button3,    gesture,        {0} },
  { ClkClientWin,     MODKEY,       Button4,    incrgaps,       { .i = -1 } },
  { ClkClientWin,     MODKEY,       Button5,    incrgaps,       { .i = +1 } },
  { ClkLtSymbol,      0,            Button3,    layoutmenu,     {0} },
  { ClkTagBar,        0,            Button1,    view,           {0} },
  { ClkTagBar,        0,            Button2,    spawntag,       {0} },
  { ClkTagBar,        0,            Button3,    toggleview,     {0} },
  { ClkTagBar,        MODKEY,       Button1,    tag,            {0} },
  { ClkTagBar,        MODKEY,       Button3,    toggletag,      {0} },
  { ClkTagBar,        0,            Button4,    shiftview,      { .i = 1 } },
  { ClkTagBar,        0,            Button5,    shiftview,      { .i = -1 } },
  { ClkRootWin,       0,            Button2,    togglebar,      {0} },
  { ClkRootWin,       MODKEY,       Button2,    togglebar,      { .i = 1 } },
};

