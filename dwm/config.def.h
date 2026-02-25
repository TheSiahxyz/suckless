/* See LICENSE file for copyright and license details. */

/* Default settings; can be overriden by command line. */
#define BROWSER                         "librewolf"
#define BROWSER2                        "firefox"
#define CLEAR                           0x00U
#define GAP                             8
#define PADDING                         0
#define STATUSBAR                       "dwmblocks"
#define TERMINAL                        "st"
#define TERMCLASS                       "St"

/* Appearance */
static const double activeopacity       = 1.0f;     /* Window opacity when it's focused (0 <= opacity <= 1) */
static const double inactiveopacity     = 1.0f;     /* Window opacity when it's inactive (0 <= opacity <= 1) */
static const int allowkill              = 1;        /* allow killing clients by default? */
static       int alt_tab_direction      = 1;        /* 1 means cycle forward */
static const int mainmon                = 0;        /* xsetroot will only change the bar on this monitor */
static       int showbar                = 1;        /* 0 means no bar */
static const int showfloating           = 1;        /* 0 means no floating indicator */
static const int showlayout             = 1;        /* 0 means no layout indicator */
static const int showstatus             = 1;        /* 0 means no status bar */
static const int showtitle              = 1;        /* 0 means no title */
static const int showtags               = 1;        /* 0 means no tags */
static const int sidepad                = PADDING;  /* horizontal padding of bar */
static       int smartgaps              = 0;        /* 1 means no outer gap when there is only one window */
static const int stairdirection         = 1;        /* 0: left-aligned, 1: right-aligned */
static const int stairsamesize          = 1;        /* 1 means shrink all the staired windows to the same size */
static const int statusall              = 1;        /* 1 means status is shown in all bars, not just active monitor */
static const int swallowfloating        = 0;        /* 1 means swallow floating windows by default */
static       int topbar                 = 1;        /* 0 means bottom bar */
static const int user_bh                = 2;        /* 2 is the default spacing around the bar's font */
static const int vertpad                = PADDING;  /* vertical padding of bar */
static const unsigned int baralpha      = 0xc8;
static const unsigned int borderalpha   = OPAQUE;
static const unsigned int floatalpha    = OPAQUE;
static const unsigned int markalpha     = OPAQUE;
static       unsigned int borderpx      = 4;        /* border pixel of windows */
static const unsigned int fborderpx     = 6;        /* border pixel of floating windows */
static const unsigned int gappih        = GAP;      /* horiz inner gap between windows */
static const unsigned int gappiv        = GAP;      /* vert inner gap between windows */
static const unsigned int gappoh        = GAP;      /* horiz outer gap between windows and screen edge */
static const unsigned int gappov        = GAP;      /* vert outer gap between windows and screen edge */
static       unsigned int snap          = 32;       /* snap pixel */
static const unsigned int stairpx       = 20;       /* depth of the stairs layout */
static const unsigned int tabModKey     = 0x40;
static const unsigned int tabCycleKey   = 0x17;
static       char dmenufont[]           = "monospace:size=10";
static       char font[]                = "monospace:size=10";
static const char *fonts[]              = {
  font,
  "NotoColorEmoji:pixelsize=10:antialias=true:autohint=true",
};
static       char normbgcolor[]         = "#222222";
static       char normbordercolor[]     = "#444444";
static       char normfgcolor[]         = "#bbbbbb";
static       char normfloatcolor[]      = "#330000";
static       char normmarkcolor[]       = "#550000";
static       char norminfobgcolor[]     = "#222222";
static       char norminfofgcolor[]     = "#f8f8f2";
static       char normstatusbgcolor[]   = "#222222";
static       char normstatusfgcolor[]   = "#eeeeee";
static       char normtagbgcolor[]      = "#222222";
static       char normtagfgcolor[]      = "#cccccc";
static       char selbgcolor[]          = "#005577";
static       char selbordercolor[]      = "#770000";
static       char selfgcolor[]          = "#eeeeee";
static       char selfloatcolor[]       = "#770000";
static       char selmarkcolor[]        = "#aa0000";
static       char selinfobgcolor[]      = "#005577";
static       char selinfofgcolor[]      = "#eeeeee";
static       char selstatusbgcolor[]    = "#222222";
static       char selstatusfgcolor[]    = "#eeeeee";
static       char seltagbgcolor[]       = "#005577";
static       char seltagfgcolor[]       = "#ffffff";
static       char *colors[][5]          = {
  /*                     fg                 bg                 border           float           mark          */
  [SchemeNorm]       = { normfgcolor,       normbgcolor,       normbordercolor, normfloatcolor, normmarkcolor },
  [SchemeSel]        = { selfgcolor,        selbgcolor,        selbordercolor,  selfloatcolor,  selmarkcolor  },
  [SchemeInv]        = { normbgcolor,       normfgcolor,       normbordercolor, normfloatcolor, normmarkcolor },
  [SchemeStatusNorm] = { normstatusfgcolor, normstatusbgcolor, normbordercolor, normfloatcolor, normmarkcolor },  // Statusbar right unselected {text,background,not used but cannot be empty}
  [SchemeStatusSel]  = { selstatusfgcolor,  selstatusbgcolor,  selbordercolor,  selfloatcolor,  selmarkcolor },   // Statusbar right selected {text,background,not used but cannot be empty}
  [SchemeTagsNorm]   = { normtagfgcolor,    normtagbgcolor,    normbordercolor, normfloatcolor, normmarkcolor },  // Tagbar left unselected {text,background,not used but cannot be empty}
  [SchemeTagsSel]    = { seltagfgcolor,     seltagbgcolor,     selbordercolor,  selfloatcolor,  selmarkcolor  },  // Tagbar left selected {text,background,not used but cannot be empty}
  [SchemeInfoNorm]   = { norminfofgcolor,   norminfobgcolor,   normbordercolor, normfloatcolor, normmarkcolor },  // infobar middle  unselected {text,background,not used but cannot be empty}
  [SchemeInfoSel]    = { selinfofgcolor,    selinfobgcolor,    selbordercolor,  selfloatcolor,  selmarkcolor  },  // infobar middle  selected {text,background,not used but cannot be empty}
};
static const unsigned int alphas[][5]      = {
  /*                     fg      bg        border       float       mark     */
  [SchemeNorm]       = { OPAQUE, baralpha, borderalpha, floatalpha, markalpha },
  [SchemeSel]        = { OPAQUE, baralpha, borderalpha, floatalpha, markalpha },
  [SchemeInv]        = { CLEAR,  CLEAR,    CLEAR,       CLEAR,      CLEAR     },
  [SchemeStatusNorm] = { OPAQUE, CLEAR,    CLEAR,       CLEAR,      CLEAR     },
  [SchemeStatusSel]  = { OPAQUE, baralpha, borderalpha, floatalpha, markalpha },
  [SchemeTagsNorm]   = { OPAQUE, CLEAR,    CLEAR,       CLEAR,      CLEAR     },
  [SchemeTagsSel]    = { OPAQUE, baralpha, borderalpha, floatalpha, markalpha },
  [SchemeInfoNorm]   = { OPAQUE, CLEAR,    CLEAR,       CLEAR,      CLEAR     },
  [SchemeInfoSel]    = { OPAQUE, baralpha, borderalpha, floatalpha, markalpha },
};
static const XPoint stickyicon[] = { {0,0}, {4,0}, {4,8}, {2,6}, {0,8}, {0,0} }; /* represents the icon as an array of vertices */
static const XPoint stickyiconbb = {4,8}; /* defines the bottom right corner of the polygon's bounding box (speeds up scaling) */

typedef struct {
  const char *name;
  const void *cmd;
} Sp;
const char *spcmd1[] = { TERMINAL, "-n", "spterm", "-g", "120x34", NULL };
const char *spcmd2[] = { TERMINAL, "-n", "splf", "-g", "144x41", "-e", "lf", NULL };
const char *spcmd3[] = { TERMINAL, "-n", "spcalc", "-f", "monospace:size=16", "-g", "50x20", "-e", "bc", "-lq", NULL };
const char *spcmd4[] = { TERMINAL, "-n", "vimwikitodo", "-f", "monospace:size=12", "-g", "35x15", "-e", "vimwikitodo", NULL };
static Sp scratchpads[] = {
  /* name          cmd  */
  {"spterm",      spcmd1},
  {"splf",        spcmd2},
  {"spcalc",      spcmd3},
  {"vimwikitodo", spcmd4},
};

/* Tagging */
static const char *tags[] = { "", "", "󱔗", "", "", "", "", "", "󰭹" };
static const char *tagsalt[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static       char *tagsel[][2] = {
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
  { "#f8f8f2", "#005577" },
};

static const unsigned int tagalpha[] = { OPAQUE, baralpha };
static const int momentaryalttags = 0; /* 1 means alttags will show only when key is held down*/
static const char ptagf[] = "%s. %s"; /* format of a tag label */
static const char etagf[] = "%s"; /* format of an empty tag */
static const int taglbl   = 0;    /* 1 means enable tag label */
static const int lcaselbl = 0;    /* 1 means make tag label lowercase */
static const Rule rules[] = {
  /* xprop(1):
   * xprop | awk '
   *  /^WM_CLASS/{sub(/.* =/, "instance:"); sub(/,/, "\nclass:"); print}
   *  /^WM_NAME/{sub(/.* =/, "title:"); print}'
   * WM_CLASS(STRING) = instance, class
   * WM_NAME(STRING) = title
   */
  /* class              instance          title           tags mask  allowkill  focusopacity    unfocusopacity    isfloating  isterminal  noswallow  monitor  resizehints  border width */
  { TERMCLASS,          TERMINAL,         TERMINAL,       1 << 0,     1,        activeopacity,  inactiveopacity,  0,          1,          0,        -1,       1,           -1 },
  { "Cursor",           "cursor",         NULL,           1 << 0,     1,        activeopacity,  inactiveopacity,  0,          0,          0,         1,       1,           -1 },
  { BROWSER,            "Navigator",      NULL,           1 << 1,     1,        activeopacity,  inactiveopacity,  0,          0,         -1,         0,       1,           -1 },
  { BROWSER2,           "Navigator",      NULL,           1 << 1,     1,        activeopacity,  inactiveopacity,  0,          0,         -1,         1,       1,           -1 },
  { NULL,               "libreoffice",    NULL,           1 << 2,     1,        activeopacity,  inactiveopacity,  0,          0,          0,         0,       1,           -1 },
  { "mpv",              "video",          NULL,           1 << 3,     1,        activeopacity,  inactiveopacity,  0,          1,         -1,         0,       0,            0 },
  { "mpv",              "music",          NULL,           1 << 4,     1,        activeopacity,  inactiveopacity,  0,          1,         -1,         0,       0,            0 },
  { TERMCLASS,          "ncmpcpp",        NULL,           1 << 4,     1,        activeopacity,  inactiveopacity,  0,          1,          0,         0,       1,           -1 },
  { "YTMusic",          "yt-music",       NULL,           1 << 4,     1,        activeopacity,  inactiveopacity,  0,          1,          0,         0,       1,           -1 },
  { "DBeaver",          "DBeaver",        NULL,           1 << 5,     1,        activeopacity,  inactiveopacity,  0,          0,          0,         1,       1,           -1 },
  { "lazydocker",       "lazydocker",     NULL,           1 << 5,     1,        activeopacity,  inactiveopacity,  0,          1,          0,        -1,       1,           -1 },
  { "Virt-viewer",      "virt-viewer",    NULL,           1 << 5,     1,        activeopacity,  inactiveopacity,  0,          0,          0,         0,       1,           -1 },
  { "Virt-manager",     "virt-manager",   NULL,           1 << 5,     1,        activeopacity,  inactiveopacity,  0,          0,          0,         1,       1,           -1 },
  { TERMCLASS,          "bg",             NULL,           1 << 6,     1,        activeopacity,  inactiveopacity,  0,          1,          0,         0,       1,           -1 },
  { "Gimp",             NULL,             NULL,           1 << 6,     1,        activeopacity,  inactiveopacity,  0,          0,          0,         1,       1,           -1 },
  { "obs",              "obs",            NULL,           1 << 7,     1,        activeopacity,  inactiveopacity,  0,          0,         -1,         1,       1,            0 },
  { "mpv",              "webcam",         NULL,           1 << 7,     1,        activeopacity,  inactiveopacity,  0,          1,         -1,         1,       0,            0 },
  { "discord",          "discord",        NULL,           1 << 8,     1,        activeopacity,  inactiveopacity,  0,          0,         -1,         1,       1,            0 },
  { "kakaotalk.exe",    "kakaotalk.exe",  NULL,           1 << 8,     1,        activeopacity,  inactiveopacity,  0,          0,         -1,         0,       1,            0 },
  { "teams-for-linux",  "teams-for-linux",NULL,           1 << 8,     1,        activeopacity,  inactiveopacity,  0,          0,         -1,         1,       1,            0 },
  { TERMCLASS,          "spterm",         NULL,           SPTAG(0),   1,        activeopacity,  inactiveopacity,  1,          1,          0,        -1,       1,           -1 },
  { TERMCLASS,          "splf",           NULL,           SPTAG(1),   1,        activeopacity,  inactiveopacity,  1,          1,          0,        -1,       1,           -1 },
  { TERMCLASS,          "spcalc",         NULL,           SPTAG(2),   1,        activeopacity,  inactiveopacity,  1,          1,          0,        -1,       1,           -1 },
  { TERMCLASS,          "vimwikitodo",    NULL,           SPTAG(3),   1,        activeopacity,  inactiveopacity,  1,          1,          0,        -1,       1,           -1 },
  { TERMCLASS,          "floatterm",      NULL,           0,          1,        activeopacity,  inactiveopacity,  1,          1,          0,        -1,       1,            0 },
  { TERMCLASS,          "stig",           NULL,           0,          1,        activeopacity,  inactiveopacity,  0,          1,          1,        -1,       0,           -1 },
  { NULL,               NULL,             "Event Tester", 0,          1,        activeopacity,  inactiveopacity,  0,          0,          1,        -1,       1,           -1 }, /* xev */
};

/* Layout(s) */
static float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 0; /* 1 will force focus on the fullscreen window */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include <X11/XF86keysym.h>
#include "exresize.c"
#include "shift-tools-scratchpads.c"
#include "vanitygaps.c"

static const Layout layouts[] = {
  /* symbol     arrange function */
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
  { "[S]",    stairs },                   /* 14: Stairs */
  { "><>",    NULL },                     /* 15: no layout function means floating behavior */
};

/* Key definitions */
#define WINKEY      Mod4Mask
#define WINMOD      (WINKEY|ShiftMask)
#define WINMOD2     (WINKEY|ControlMask)
#define WINMODALL   (WINKEY|ControlMask|ShiftMask)
#define ALTKEY      Mod1Mask
#define ALTMOD      (ALTKEY|ShiftMask)
#define ALTMOD2     (ALTKEY|ControlMask)
#define ALTMODALL   (ALTKEY|ControlMask|ShiftMask)
#define ULTRAKEY    (WINKEY|ALTKEY)
#define ULTRAMOD    (WINKEY|ALTKEY|ShiftMask)
#define ULTRAMOD2   (WINKEY|ALTKEY|ControlMask)
#define ULTRAMODALL (WINKEY|ALTKEY|ShiftMask|ControlMask)
#define EXTRAMOD    (ControlMask|ShiftMask)
#define TAGKEYS(KEY,TAG)                                                    \
  &((Keychord){1, {{WINKEY,     KEY}},  view,         {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{WINMOD,     KEY}},  tag,          {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{WINMOD2,    KEY}},  toggleview,   {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{WINMODALL,  KEY}},  tagandview,   {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{ALTKEY,     KEY}},  focusnthmon,  {.i  = TAG } }),      \
  &((Keychord){1, {{ALTMOD,     KEY}},  tagnthmon,    {.i  = TAG } }),      \
  &((Keychord){1, {{ALTMOD2,    KEY}},  toggletag,    {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{ULTRAKEY,   KEY}},  nview,        {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{ULTRAMOD,   KEY}},  viewall,      {.ui = 1 << TAG} }),  \
  &((Keychord){1, {{ULTRAMOD2,  KEY}},  ntoggleview,  {.ui = 1 << TAG} }),
#define STACKKEYS(MOD,ACTION)                                                                   \
  &((Keychord){1, {{MOD, XK_j}},                            ACTION##stack,  {.i = INC(+1) } }), \
  &((Keychord){1, {{MOD, XK_k}},                            ACTION##stack,  {.i = INC(-1) } }), \
  &((Keychord){1, {{MOD, XK_Tab}},                          ACTION##stack,  {.i = PREVSEL } }), \
  &((Keychord){2, {{MOD, XK_BackSpace},{0, XK_BackSpace}},  ACTION##stack,  {.i = 0 } }),       \
  &((Keychord){2, {{MOD, XK_BackSpace},{0, XK_a}},          ACTION##stack,  {.i = 1 } }),       \
  &((Keychord){2, {{MOD, XK_BackSpace},{0, XK_z}},          ACTION##stack,  {.i = 2 } }),       \
  &((Keychord){2, {{MOD, XK_BackSpace},{0, XK_x}},          ACTION##stack,  {.i = -1 } }),

/* Helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* Helper for launching gtk application */
#define GTKCMD(cmd) { .v = (const char*[]){ "/usr/bin/gtk-launch", cmd, NULL } }

/* Commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { TERMINAL, NULL };
static const char *layoutmenu_cmd = "layoutmenu";
static const Arg tagexec[] = {
  { .v = termcmd },                                                   // 1
  { .v = (const char *[]){ BROWSER, NULL } },                         // 2
  SHCMD(TERMINAL " -e neomutt; pkill -RTMIN+18 dwmblocks"),           // 3
  SHCMD(TERMINAL " -e newsboat; pkill -RTMIN+17 dwmblocks"),          // 4
  SHCMD(TERMINAL " -n ncmpcpp -e ncmpcpp"),                           // 5
  { .v = (const char *[]){ "torwrap", NULL } },                       // 6
  { .v = (const char *[]){ TERMINAL, "-e", "sudo", "nmtui", NULL } }, // 7
  { .v = (const char *[]){ TERMINAL, "-e", "htop", NULL } },          // 8
  { .v = (const char *[]){ "kakaotalk", NULL } }                      // 9
};

/* Gestures
 * u means up
 * d means down
 * l means left
 * r means right
 * ud means up and down
 */
static Gesture gestures[] = {
  { "u",  spawn, { .v = termcmd } },
  { "d",  spawn, { .v = (const char *[]){ BROWSER, NULL } } },
  { "l",  spawn, SHCMD(TERMINAL " -e neomutt; pkill -RTMIN+18 dwmblocks; rmdir ~/.abook 2>/dev/null") },
  { "r",  spawn, SHCMD(TERMINAL " -e newsboat; pkill -RTMIN+17 dwmblocks") },
};

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
  { "borderpx",           INTEGER, &borderpx            },
  { "dmenufont",          STRING,  &dmenufont           },
  { "font",               STRING,  &font                },
  { "mfact",              FLOAT,   &mfact               },
  { "nmaster",            INTEGER, &nmaster             },
  { "normbgcolor",        STRING,  &normbgcolor         },
  { "normbordercolor",    STRING,  &normbordercolor     },
  { "normfgcolor",        STRING,  &normfgcolor         },
  { "normfloatcolor",     STRING,  &normfloatcolor      },
  { "normmarkcolor",      STRING,  &normmarkcolor       },
  { "normstatusbgcolor",  STRING,  &normstatusbgcolor   },
  { "normstatusfgcolor",  STRING,  &normstatusfgcolor   },
  { "normtagbgcolor",     STRING,  &normtagbgcolor      },
  { "normtagfgcolor",     STRING,  &normtagfgcolor      },
  { "norminfobgcolor",    STRING,  &norminfobgcolor     },
  { "norminfofgcolor",    STRING,  &norminfofgcolor     },
  { "resizehints",        INTEGER, &resizehints         },
  { "selbgcolor",         STRING,  &selbgcolor          },
  { "selbordercolor",     STRING,  &selbordercolor      },
  { "selfgcolor",         STRING,  &selfgcolor          },
  { "selfloatcolor",      STRING,  &selfloatcolor       },
  { "selmarkcolor",       STRING,  &selmarkcolor        },
  { "seltagbgcolor",      STRING,  &seltagbgcolor       },
  { "seltagfgcolor",      STRING,  &seltagfgcolor       },
  { "selinfobgcolor",     STRING,  &selinfobgcolor      },
  { "selinfofgcolor",     STRING,  &selinfofgcolor      },
  { "showbar",            INTEGER, &showbar             },
  { "snap",               INTEGER, &snap                },
  { "topbar",             INTEGER, &topbar              },
};

static Keychord *keychords[] = {
  /*           num  keys                                            function                argument */
  // STACKKEYS
  STACKKEYS(        WINKEY,                                         focus)
  STACKKEYS(        WINMOD,                                         push)

  // TAGKEYS
  TAGKEYS(          XK_1,                                                                   0)
  TAGKEYS(          XK_2,                                                                   1)
  TAGKEYS(          XK_3,                                                                   2)
  TAGKEYS(          XK_4,                                                                   3)
  TAGKEYS(          XK_5,                                                                   4)
  TAGKEYS(          XK_6,                                                                   5)
  TAGKEYS(          XK_7,                                                                   6)
  TAGKEYS(          XK_8,                                                                   7)
  TAGKEYS(          XK_9,                                                                   8)

  // APPEARANCES
  &((Keychord){1, {{WINKEY, XK_a}},                                 changefocusopacity,     {.f = -0.025}}),
  &((Keychord){1, {{WINKEY, XK_s}},                                 changefocusopacity,     {.f = +0.025}}),
  &((Keychord){1, {{WINMOD, XK_a}},                                 changeunfocusopacity,   {.f = -0.025}}),
  &((Keychord){1, {{WINMOD, XK_s}},                                 changeunfocusopacity,   {.f = +0.025}}),
  &((Keychord){1, {{WINKEY, XK_o}},                                 setborderpx,            {.i = -1 } }),
  &((Keychord){1, {{WINMOD, XK_o}},                                 setborderpx,            {.i = +1 } }),
  &((Keychord){1, {{WINMOD2, XK_o}},                                setborderpx,            {.i = 0 } }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_a}},             togglebartags,          {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_b}},             togglebar,              {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{ShiftMask, XK_b}},     togglebar,              {.i = 1} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_d}},             defaultgaps,            {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_f}},             togglebarfloat,         {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_g}},             togglegaps,             {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_l}},             togglebarlt,            {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_o}},             toggleborder,           {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_s}},             togglebarstatus,        {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{0, XK_t}},             togglebartitle,         {0} }),
  &((Keychord){3, {{WINKEY, XK_t},{0, XK_b},{ShiftMask, XK_t}},     toggletopbar,           {0} }),

  // FLOATING POSITIONS
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_c}},                       movecenter,             {0} }),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_m}},                       explace,                {.ui = EX_SW }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_comma}},                   explace,                {.ui = EX_S  }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_period}},                  explace,                {.ui = EX_SE }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_j}},                       explace,                {.ui = EX_W  }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_k}},                       explace,                {.ui = EX_C  }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_l}},                       explace,                {.ui = EX_E  }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_u}},                       explace,                {.ui = EX_NW }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_i}},                       explace,                {.ui = EX_N  }}),
  &((Keychord){2, {{WINKEY, XK_f},{0, XK_o}},                       explace,                {.ui = EX_NE }}),

  // FLOATING SIZES
  &((Keychord){1, {{ALTKEY, XK_KP_Down}},                           exresize,               {.v = (int []){   0,  25 }}}),
  &((Keychord){1, {{ALTKEY, XK_KP_Left}},                           exresize,               {.v = (int []){ -25,   0 }}}),
  &((Keychord){1, {{ALTKEY, XK_KP_Begin}},                          exresize,               {.v = (int []){  25,  25 }}}),
  &((Keychord){1, {{ALTMOD, XK_KP_Begin}},                          exresize,               {.v = (int []){ -25, -25 }}}),
  &((Keychord){1, {{ALTKEY, XK_KP_Right}},                          exresize,               {.v = (int []){  25,   0 }}}),
  &((Keychord){1, {{ALTKEY, XK_KP_Up}},                             exresize,               {.v = (int []){   0, -25 }}}),
  &((Keychord){1, {{ALTMOD2, XK_KP_End}},                           toggleverticalexpand,   {.i =  0} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Down}},                          toggleverticalexpand,   {.i = -1} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Next}},                          togglehorizontalexpand, {.i =  0} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Left}},                          togglehorizontalexpand, {.i = -1} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Begin}},                         togglemaximize,         {.i =  0} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Right}},                         togglehorizontalexpand, {.i = +1} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Home}},                          togglemaximize,         {.i = +1} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Up}},                            toggleverticalexpand,   {.i = +1} }),
  &((Keychord){1, {{ALTMOD2, XK_KP_Prior}},                         togglemaximize,         {.i = -1} }),

  // LAYOUTS
  &((Keychord){1, {{WINMOD, XK_h}},                                 layoutscroll,           {.i = -1 } }),
  &((Keychord){1, {{WINMOD, XK_l}},                                 layoutscroll,           {.i = +1 } }),
  &((Keychord){2, {{WINMOD2, XK_l},{0, XK_r}},                      resetlayout,            {0} }),
  &((Keychord){2, {{WINMOD2, XK_l},{0, XK_n}},                      resetnmaster,           {0} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_0}},             setlayout,              {0} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_1}},             setlayout,              {.v = &layouts[0]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_2}},             setlayout,              {.v = &layouts[1]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_3}},             setlayout,              {.v = &layouts[2]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_4}},             setlayout,              {.v = &layouts[3]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_5}},             setlayout,              {.v = &layouts[4]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_6}},             setlayout,              {.v = &layouts[5]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_7}},             setlayout,              {.v = &layouts[6]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_8}},             setlayout,              {.v = &layouts[7]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_0},{0,XK_9}},             setlayout,              {.v = &layouts[8]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_0}},             setlayout,              {.v = &layouts[9]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_1}},             setlayout,              {.v = &layouts[10]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_2}},             setlayout,              {.v = &layouts[11]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_3}},             setlayout,              {.v = &layouts[12]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_4}},             setlayout,              {.v = &layouts[13]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_5}},             setlayout,              {.v = &layouts[14]} }),
  &((Keychord){3, {{WINMOD2, XK_l},{0, XK_1},{0,XK_6}},             setlayout,              {.v = &layouts[15]} }),

  // LAYOUT SIZES
  &((Keychord){1, {{WINKEY, XK_KP_Insert}},                         setcfact,               {.f =  0.00} }),
  &((Keychord){1, {{WINKEY, XK_KP_End}},                            incrgaps,               {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_End}},                           incrgaps,               {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Down}},                           incrigaps,              {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Down}},                          incrigaps,              {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Next}},                           incrogaps,              {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Next}},                          incrogaps,              {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Left}},                           incrihgaps,             {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Left}},                          incrihgaps,             {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Begin}},                          incrivgaps,             {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Begin}},                         incrivgaps,             {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Right}},                          incrohgaps,             {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Right}},                         incrohgaps,             {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Home}},                           incrovgaps,             {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Home}},                          incrovgaps,             {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_KP_Up}},                             incnmaster,             {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_KP_Up}},                            incnmaster,             {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_h}},                                 setmfact,               {.f = -0.05} }),
  &((Keychord){1, {{WINKEY, XK_l}},                                 setmfact,               {.f = +0.05} }),

  // MEDIA CONTROLS
  &((Keychord){1, {{0, NoSymbol}},                                  spawn,                  {.v = termcmd } }),
  &((Keychord){1, {{0, XF86XK_Battery}},                            spawn,                  SHCMD("pkill -RTMIN+3 dwmblocks") }),
  &((Keychord){1, {{0, XF86XK_WWW}},                                spawn,                  {.v = (const char *[]){ BROWSER, NULL } } }),
  &((Keychord){1, {{0, XF86XK_DOS}},                                spawn,                  {.v = termcmd } }),
  &((Keychord){1, {{0, XF86XK_AudioMute}},                          spawn,                  SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{0, XF86XK_AudioRaiseVolume}},                   spawn,                  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 0%- && wpctl set-volume @DEFAULT_AUDIO_SINK@ 3%+; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{0, XF86XK_AudioLowerVolume}},                   spawn,                  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 0%+ && wpctl set-volume @DEFAULT_AUDIO_SINK@ 3%-; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{0, XF86XK_AudioPrev}},                          spawn,                  {.v = (const char *[]){ "mpc", "prev", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioNext}},                          spawn,                  {.v = (const char *[]){ "mpc", "next", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioPause}},                         spawn,                  {.v = (const char *[]){ "mpc", "pause", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioPlay}},                          spawn,                  {.v = (const char *[]){ "mpc", "play", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioStop}},                          spawn,                  {.v = (const char *[]){ "mpc", "stop", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioRewind}},                        spawn,                  {.v = (const char *[]){ "mpc", "seek", "-10", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioForward}},                       spawn,                  {.v = (const char *[]){ "mpc", "seek", "+10", NULL } } }),
  &((Keychord){1, {{0, XF86XK_AudioMedia}},                         spawn,                  SHCMD(TERMINAL " -n ncmpcpp -e ncmpcpp") }),
  &((Keychord){1, {{0, XF86XK_AudioMicMute}},                       spawn,                  SHCMD("pactl set-source-mute @DEFAULT_SOURCE@ toggle") }),
  &((Keychord){1, {{0, XF86XK_Calculator}},                         spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "bc", "-l", NULL } } }),
  &((Keychord){1, {{0, XF86XK_Launch1}},                            spawn,                  {.v = (const char *[]){ "xset", "dpms", "force", "off", NULL } } }),
  &((Keychord){1, {{0, XF86XK_Mail}},                               spawn,                  SHCMD(TERMINAL " -e neomutt ; pkill -RTMIN+18 dwmblocks") }),
  &((Keychord){1, {{0, XF86XK_MonBrightnessDown}},                  spawn,                  SHCMD("pkexec brillo -U 5 -q; kill -42 $(pidof dwmblocks)") }),
  /* &((Keychord){1, {{0, XF86XK_MonBrightnessDown}},                  spawn,                  {.v = (const char*[]){ "xbacklight", "-dec", "15", NULL } } }), */
  &((Keychord){1, {{0, XF86XK_MonBrightnessUp}},                    spawn,                  SHCMD("pkexec brillo -A 5 -q; kill -42 $(pidof dwmblocks)") }),
  /* &((Keychord){1, {{0, XF86XK_MonBrightnessUp}},                    spawn,                  {.v = (const char*[]){ "xbacklight", "-inc", "15", NULL } } }), */
  &((Keychord){1, {{0, XF86XK_MyComputer}},                         spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "lfub", "/", NULL } } }),
  &((Keychord){1, {{0, XF86XK_PowerOff}},                           spawn,                  {.v = (const char*[]){ "sysact", NULL } } }),
  &((Keychord){1, {{0, XF86XK_RotateWindows}},                      spawn,                  {.v = (const char *[]){ "tablet", NULL } } }),
  &((Keychord){1, {{0, XF86XK_ScreenSaver}},                        spawn,                  SHCMD("slock & xset dpms force off; mpc pause; pauseallmpv") }),
  &((Keychord){1, {{0, XF86XK_Sleep}},                              spawn,                  {.v = (const char *[]){ "sudo", "-A", "zzz", NULL } } }),
  &((Keychord){1, {{0, XF86XK_TaskPane}},                           spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "htop", NULL } } }),
  &((Keychord){1, {{0, XF86XK_TouchpadOff}},                        spawn,                  {.v = (const char *[]){ "synclient", "TouchpadOff=1", NULL } } }),
  &((Keychord){1, {{0, XF86XK_TouchpadOn}},                         spawn,                  {.v = (const char *[]){ "synclient", "TouchpadOff=0", NULL } } }),
  &((Keychord){1, {{0, XF86XK_TouchpadToggle}},                     spawn,                  SHCMD("(synclient | grep 'TouchpadOff.*1' && synclient TouchpadOff=0) || synclient TouchpadOff=1") }),

  // MUSIC CONTROLS
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_a}},                       spawn,                  SHCMD("mpc single on; mpc random on; mpc repeat on; mpc consume off") }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_d}},                       spawn,                  SHCMD("mpdmenu && pkill -RTMIN+21 dwmblocks") }),
  &((Keychord){2, {{WINKEY, XK_m},{ShiftMask, XK_d}},               spawn,                  {.v = (const char *[]){ "dmenudelmusic", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_m}},                       spawn,                  SHCMD("mpc random on; mpc load entire; mpc play && sleep 1 && mpc volume 50 && pkill -RTMIN+21 dwmblocks") }),
  &((Keychord){2, {{WINKEY, XK_m},{ControlMask, XK_m}},             spawn,                  SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle; sleep 1 && kill -38 $(pidof dwmblocks)") }),
  &((Keychord){2, {{WINKEY, XK_m},{ShiftMask, XK_m}},               spawn,                  {.v = (const char *[]){ "youtube-music", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_o}},                       spawn,                  SHCMD("mpc repeat off; mpc random off; mpc single off; mpc consume off") }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_p}},                       spawn,                  {.v = (const char *[]){ "mpc", "toggle", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_m},{ControlMask, XK_p}},             spawn,                  SHCMD("mpc pause; sleep 1 && pauseallmpv") }),
  &((Keychord){2, {{WINKEY, XK_m},{ShiftMask, XK_p}},               spawn,                  {.v = (const char *[]){ "playerctl", "play-pause", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_r}},                       spawn,                  SHCMD("mpc single off; mpc random on; mpc repeat on; mpc consume off") }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_s}},                       spawn,                  SHCMD("mpc single on; mpc random off; mpc repeat on; mpc consume off") }),
  &((Keychord){2, {{WINKEY, XK_m},{0, XK_x}},                       spawn,                  SHCMD("mpc stop; sleep 1 && mpc repeat off && mpc random off && mpc single off && mpc consume off && mpc clear") }),
  &((Keychord){2, {{WINKEY, XK_m},{ShiftMask, XK_x}},               spawn,                  {.v = (const char *[]){ "playerctl", "stop", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_comma}},                             spawn,                  {.v = (const char *[]){ "mpc", "prev", NULL } } }),
  &((Keychord){1, {{ALTKEY, XK_comma}},                             spawn,                  {.v = (const char *[]){ "playerctl", "previous", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_period}},                            spawn,                  {.v = (const char *[]){ "mpc", "next", NULL } } }),
  &((Keychord){1, {{ALTKEY, XK_period}},                            spawn,                  {.v = (const char *[]){ "playerctl", "next", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_comma}},                             spawn,                  {.v = (const char *[]){ "mpc", "seek", "-10", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_period}},                            spawn,                  {.v = (const char *[]){ "mpc", "seek", "+10", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_comma}},                            spawn,                  {.v = (const char *[]){ "mpc", "seek", "-60", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_period}},                           spawn,                  {.v = (const char *[]){ "mpc", "seek", "+60", NULL } } }),
  &((Keychord){1, {{WINMODALL, XK_bracketleft}},                    spawn,                  {.v = (const char *[]){ "mpc", "seek", "0%", NULL } } }),
  &((Keychord){1, {{WINMODALL, XK_bracketright}},                   spawn,                  {.v = (const char *[]){ "mpc", "seek", "90%", NULL } } }),

  // MOUSE
  &((Keychord){1, {{ULTRAMOD, XK_j}},                               spawn,                  { .v = (const char *[]){ "xdotmouse", "h", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_k}},                               spawn,                  { .v = (const char *[]){ "xdotmouse", "j", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_l}},                               spawn,                  { .v = (const char *[]){ "xdotmouse", "k", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_semicolon}},                       spawn,                  { .v = (const char *[]){ "xdotmouse", "l", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_i}},                               spawn,                  { .v = (const char *[]){ "xdotmouse", "c", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_u}},                               spawn,                  { .v = (const char *[]){ "xdotmouse", "C", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_o}},                               spawn,                  { .v = (const char *[]){ "xdotmouse", "m", NULL } } }),

  // PROGRAMS
  &((Keychord){1, {{WINKEY, XK_e}},                                 spawn,                  SHCMD(TERMINAL " -e neomutt; pkill -RTMIN+18 dwmblocks; rmdir ~/.abook 2>/dev/null") }),
  &((Keychord){1, {{WINMOD, XK_e}},                                 spawn,                  {.v = (const char *[]){ "outlook-for-linux", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_w}},                                 spawn,                  {.v = (const char *[]){ BROWSER, NULL } } }),
  &((Keychord){1, {{WINMOD, XK_w}},                                 spawn,                  {.v = (const char *[]){ BROWSER, "--target", "private-window", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_x},{0, XK_d}},                       spawn,                  {.v = (const char *[]){ "pkill", "-f", "discord", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_x},{0, XK_k}},                       spawn,                  {.v = (const char *[]){ "pkill", "-f", "KakaoTalk.exe", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_x},{0, XK_w}},                       spawn,                  {.v = (const char *[]){ "pkill", "-f", BROWSER, NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_a}},                   spawn,                  SHCMD(TERMINAL " -e abook -C ${XDG_CONFIG_HOME:-${HOME}/.config}/abook/abookrc --datafile ${XDG_CONFIG_HOME:-${HOME}/.config}/abook/addressbook") }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_c}},                   spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "calcurse", NULL } } }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_d},{0, XK_b}},         spawn,                  {.v = (const char *[]){ "dbeaver", NULL } } }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_d},{0, XK_o}},         spawn,                  SHCMD(TERMINAL " -c 'lazydocker' -n 'lazydocker' -e sudo lazydocker") }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_d},{0, XK_s}},         spawn,                  {.v = (const char *[]){ "discord", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_g}},                   spawn,                  {.v = (const char *[]){ "gimp", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_h}},                   spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "htop", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_i}},                   spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "nmtui", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_k}},                   spawn,                  {.v = (const char *[]){ "kakaotalk", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_l}},                   spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "lfub", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{ShiftMask, XK_l}},           spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "lfub", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_m}},                   spawn,                  SHCMD(TERMINAL " -n ncmpcpp -e ncmpcpp") }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_n}},                   spawn,                  SHCMD(TERMINAL " -e newsboat; pkill -RTMIN+17 dwmblocks") }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_p}},                   spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "profanity", NULL } } }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_t},{0,XK_r}},          spawn,                  {.v = (const char *[]){ "torwrap", NULL } } }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_t},{0,XK_m}},          spawn,                  {.v = (const char *[]){ "teams-for-linux", NULL } } }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_v},{0, XK_m}},         spawn,                  {.v = (const char *[]){ "dmenuvirt", NULL } } }),
  &((Keychord){3, {{WINKEY, XK_space},{0, XK_v},{0, XK_w}},         spawn,                  {.v = (const char *[]){ TERMINAL, "-e", "nvim", "-c", "VimwikiIndex", "1", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_w}},                   spawn,                  SHCMD(TERMINAL " -e less -Sf ${XDG_CACHE_HOME:-${HOME}/.cache}/weatherreport") }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_z}},                   spawn,                  {.v = (const char *[]){ "zoom", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_Return}},              spawn,                  {.v = (const char *[]){ "cursor", NULL } } }),

  // SCRIPTS
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_b}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-b", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_c}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-c", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_i}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-t", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_o}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-o", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_p}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-p", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_s}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-s", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_b},{0, XK_v}},                       spawn,                  {.v = (const char *[]){ "bookmarks", "-v", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_c}},                                 spawn,                  {.v = (const char *[]){ "clipmenu", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_p}},                                 spawn,                  {.v = (const char *[]){ "passmenu", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_p}},                                 spawn,                  {.v = (const char *[]){ "passmenu2", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_c}},                       spawn,                  {.v = (const char *[]){ "crontog", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_e}},                       spawn,                  SHCMD("ecrypt; pkill -RTMIN+1 dwmblocks") }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_v}},                       spawn,                  {.v = (const char *[]){ "ovpn", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_v},{0, XK_v}},                       spawn,                  {.v = (const char *[]){ "mpvplay", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_b}},                   spawn,                  {.v = (const char *[]){ "dmenubrowse", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_u}},                   spawn,                  {.v = (const char *[]){ "dmenuunicode", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_Insert}},                            spawn,                  SHCMD("xdotool type $(grep -v '^#' ~/.local/share/thesiah/snippets | dmenu -i -l 50 | cut -d' ' -f1)") }),
  &((Keychord){1, {{0, XK_Print}},                                  spawn,                  SHCMD("maim | tee ~/Pictures/screenshots/$(date '+%y%m%d-%H%M-%S').png | xclip -selection clipboard") }),
  &((Keychord){1, {{WINKEY, XK_Print}},                             spawn,                  {.v = (const char *[]){ "maimpick", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_Print}},                             spawn,                  {.v = (const char *[]){ "dmenurecord", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_Print}},                            spawn,                  {.v = (const char *[]){ "dmenurecord", "kill", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_Scroll_Lock}},                       spawn,                  {.v = (const char *[]){ "remaps", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_Scroll_Lock}},                      spawn,                  SHCMD("killall screenkey || screenkey -t 3 -p fixed -s small -g 20%x5%+40%-5% --key-mode keysyms --bak-mode normal --mods-mode normal -f ttf-font-awesome --opacity 0.5 &") }),
  &((Keychord){1, {{WINKEY, XK_F1}},                                spawn,                  SHCMD("groff -mom /usr/local/share/dwm/thesiah.mom -Tpdf | zathura -") }),
  &((Keychord){1, {{WINMOD, XK_F1}},                                spawn,                  SHCMD("nsxiv -ap ${XDG_PICTURES_DIR:-${HOME}/Pictures}/resources") }),
  &((Keychord){1, {{WINMOD2, XK_F1}},                               spawn,                  {.v = (const char *[]){ "dmenuman", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F2}},                                spawn,                  {.v = (const char *[]){ "tutorialvids", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F3}},                                spawn,                  {.v = (const char *[]){ "dmenudisplay", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F3}},                                spawn,                  {.v = (const char *[]){ "dmenudisplay", "-r", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F4}},                                spawn,                  SHCMD(TERMINAL " -n pulsemixer -e pulsemixer; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{WINMOD2, XK_F4}},                               spawn,                  {.v = (const char *[]){ "toggleoutput", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F5}},                                spawn,                  {.v = (const char *[]){ "mailsync", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F5}},                                spawn,                  {.v = (const char *[]){ "newsup", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_F5}},                               spawn,                  {.v = (const char *[]){ "checkup", NULL } } }),
  &((Keychord){1, {{ULTRAMOD, XK_F5}},                              spawn,                  {.v = (const char *[]){ "dmenuupgrade", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F6}},                                spawn,                  {.v = (const char *[]){ "qndl", "-v", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F6}},                                spawn,                  {.v = (const char *[]){ "qndl", "-m", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F7}},                                spawn,                  {.v = (const char *[]){ "transadd", "-l", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_F7}},                               spawn,                  {.v = (const char *[]){ "td-toggle", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F8}},                                spawn,                  {.v = (const char *[]){ "stw", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_F8}},                               spawn,                  {.v = (const char *[]){ "rbackup", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F9}},                                spawn,                  {.v = (const char *[]){ "mounter", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F9}},                                spawn,                  {.v = (const char *[]){ "dmenumountcifs", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_F9}},                               spawn,                  {.v = (const char *[]){ "unmounter", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F10}},                               spawn,                  {.v = (const char *[]){ "dmenuconnections", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F10}},                               spawn,                  {.v = (const char *[]){ "dmenusmbadd", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_F10}},                              spawn,                  {.v = (const char *[]){ "dmenusmbdel", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F11}},                               spawn,                  {.v = (const char *[]){ "webcam", "h", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F11}},                               spawn,                  {.v = (const char *[]){ "dmenurecord", NULL } } }),
  &((Keychord){1, {{WINMOD2, XK_F11}},                              spawn,                  {.v = (const char *[]){ "dmenurecord", "kill", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_F12}},                               spawn,                  {.v = (const char *[]){ "fcitx5-configtool", NULL } } }),
  &((Keychord){1, {{WINMOD, XK_F12}},                               spawn,                  SHCMD("remaps") }),
  &((Keychord){1, {{ShiftMask, XK_F12}},                            spawn,                  SHCMD("remaps") }),

  // SYSTEMS
  &((Keychord){1, {{WINKEY, XK_g}},                                 gesture,                {0} }),
  &((Keychord){1, {{WINKEY, XK_q}},                                 killclient,             {0} }), // kill only current client
  &((Keychord){1, {{WINMOD, XK_q}},                                 killclient,             {.ui = 1 } }), // kill other clients in the same tag
  &((Keychord){1, {{WINMOD2, XK_q}},                                killclient,             {.ui = 2 } }), // kill all clients in the same tag
  &((Keychord){1, {{WINKEY, XK_d}},                                 spawn,                  {.v = dmenucmd } }),
  &((Keychord){1, {{WINKEY, XK_Return}},                            spawn,                  {.v = termcmd } }),
  &((Keychord){1, {{WINMOD, XK_Return}},                            spawn,                  {.v = (const char *[]){ "sd", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_BackSpace}},           spawn,                  {.v = (const char *[]){ "slock", NULL } } }),
  &((Keychord){2, {{WINKEY, XK_space},{ShiftMask, XK_BackSpace}},   spawn,                  {.v = (const char *[]){ "sysact", NULL } } }),
  &((Keychord){1, {{WINKEY, XK_minus}},                             spawn,                  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{WINKEY, XK_equal}},                             spawn,                  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{WINMOD, XK_minus}},                             spawn,                  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 1%-; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{WINMOD, XK_equal}},                             spawn,                  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 1%+; kill -38 $(pidof dwmblocks)") }),
  &((Keychord){1, {{WINMOD2, XK_minus}},                            spawn,                  SHCMD("pkexec brillo -U 5 -q; kill -41 $(pidof dwmblocks)") }),
  &((Keychord){1, {{WINMOD2, XK_equal}},                            spawn,                  SHCMD("pkexec brillo -A 5 -q; kill -41 $(pidof dwmblocks)") }),
  &((Keychord){1, {{ULTRAMOD, XK_minus}},                           spawn,                  SHCMD("monitorbright -dec 5; kill -41 $(pidof dwmblocks)") }),
  &((Keychord){1, {{ULTRAMOD, XK_equal}},                           spawn,                  SHCMD("monitorbright -inc 5; kill -41 $(pidof dwmblocks)") }),
  &((Keychord){1, {{0, XK_Alt_R}},                                  spawn,                  SHCMD("fcitx5-remote -t && kill -42 $(pidof dwmblocks)") }),
  &((Keychord){1, {{EXTRAMOD, XK_q}},                               quit,                   {0} }),
  &((Keychord){1, {{ControlMask, XK_F5}},                           quit,                   {1} }),
  &((Keychord){1, {{EXTRAMOD, XK_F5}},                              spawn,                  SHCMD("killall -9 dwmblocks; while pidof dwmblocks >/dev/null; do sleep 0.1; done; setsid -f dwmblocks") }),

  // TRAVERSALS
  &((Keychord){1, {{WINMOD2, XK_z}},                                zoom,                   {0} }),
  &((Keychord){1, {{WINKEY, XK_grave}},                             view,                   {0} }),
  &((Keychord){1, {{WINKEY, XK_0}},                                 view,                   {.ui = ~0 } }),
  &((Keychord){1, {{WINMOD, XK_0}},                                 tag,                    {.ui = ~0 } }),
  &((Keychord){1, {{WINKEY, XK_Left}},                              focusmon,               {.i = -1 } }),
  &((Keychord){1, {{WINKEY, XK_Right}},                             focusmon,               {.i = +1 } }),
  &((Keychord){1, {{WINMOD2, XK_Left}},                             tagmon,                 {.i = -1 } }),
  &((Keychord){1, {{WINMOD2, XK_Right}},                            tagmon,                 {.i = +1 } }),
  &((Keychord){1, {{WINMOD, XK_Left}},                              shiftswaptags,          {.i = -1 } }),
  &((Keychord){1, {{WINMOD, XK_Right}},                             shiftswaptags,          {.i = +1 } }),
  &((Keychord){1, {{WINMODALL, XK_Left}},                           shiftboth,              {.i = -1 } }),
  &((Keychord){1, {{WINMODALL, XK_Right}},                          shiftboth,              {.i = +1 } }),
  &((Keychord){1, {{ALTKEY, XK_Tab}},                               view,                   {0} }),
  &((Keychord){1, {{ALTMOD, XK_Tab}},                               swapfocus,              {0} }),
  &((Keychord){1, {{ALTMOD2, XK_Tab}},                              swapclient,             {0} }),
  &((Keychord){1, {{ALTMODALL, XK_Tab}},                            alttab,                 {0} }),
  &((Keychord){1, {{WINKEY, XK_bracketleft}},                       shiftview,              {.i = -1 } }),
  &((Keychord){1, {{WINKEY, XK_bracketright}},                      shiftview,              {.i = +1 } }),
  &((Keychord){1, {{WINMOD, XK_bracketleft}},                       shiftviewclients,       {.i = -1 } }),
  &((Keychord){1, {{WINMOD, XK_bracketright}},                      shiftviewclients,       {.i = +1 } }),
  &((Keychord){1, {{WINKEY, XK_z}},                                 winview,                {0} }),
  &((Keychord){2, {{WINKEY, XK_space},{0, XK_space}},               focusmaster,            {0} }),

  // TOGGLES
  &((Keychord){1, {{WINMOD2, XK_f}},                                togglefullscr,          {0} }),
  &((Keychord){1, {{ALTMOD2, XK_f}},                                togglefloating,         {0} }),
  &((Keychord){2, {{WINMOD2, XK_s},{0, XK_x}},                      scratchpad_remove,      {0} }),
  &((Keychord){2, {{WINMOD2, XK_s},{0, XK_1}},                      scratchpad_show,        {.i = 1} }),
  &((Keychord){2, {{WINMOD2, XK_s},{0, XK_2}},                      scratchpad_show,        {.i = 2} }),
  &((Keychord){2, {{WINMOD2, XK_s},{0, XK_3}},                      scratchpad_show,        {.i = 3} }),
  &((Keychord){2, {{WINMOD2, XK_s},{ShiftMask, XK_1}},              scratchpad_hide,        {.i = 1} }),
  &((Keychord){2, {{WINMOD2, XK_s},{ShiftMask, XK_2}},              scratchpad_hide,        {.i = 2} }),
  &((Keychord){2, {{WINMOD2, XK_s},{ShiftMask, XK_3}},              scratchpad_hide,        {.i = 3} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_f}},                       togglefloating,         {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{ControlMask, XK_f}},             togglecanfocusfloating, {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_m}},                       spawn,                  SHCMD("wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle; kill -39 $(pidof dwmblocks)") }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_q}},                       toggleallowkill,        {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_s}},                       togglesticky,           {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_t}},                       togglealttag,           {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_apostrophe}},              togglemark,             {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_space}},                   togglealwaysontop,      {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_Tab}},                     toggleall,              {0} }),
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_1}},                       togglescratch,          {.ui = 0 } }),  // terminal //
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_2}},                       togglescratch,          {.ui = 1 } }),  // lf //
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_3}},                       togglescratch,          {.ui = 2 } }),  // calculator //
  &((Keychord){2, {{WINKEY, XK_t},{0, XK_4}},                       togglescratch,          {.ui = 3 } }),  // todo //
  &((Keychord){1, {{WINMOD2, XK_Return}},                           togglescratch,          {.ui = 0 } }),  // terminal //

  // SUCKLESS CONFIGS
  &((Keychord){3, {{WINKEY, XK_v},{0, XK_s},{0, XK_p}},             spawn,                  SHCMD(TERMINAL " -n suckless -e sc-im ${THESIAH_WWW:-${HOME}/Private/git/THESIAH}/static/progs.csv") }),
  &((Keychord){3, {{WINKEY, XK_v},{0, XK_s},{0, XK_b}},             spawn,                  SHCMD(TERMINAL " -n suckless -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwmblocks/config.def.h") }),
  &((Keychord){3, {{WINKEY, XK_v},{0, XK_s},{0, XK_d}},             spawn,                  SHCMD(TERMINAL " -n suckless -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwm/config.def.h") }),
  &((Keychord){3, {{WINKEY, XK_v},{0, XK_s},{0, XK_m}},             spawn,                  SHCMD(TERMINAL " -n suckless -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dmenu/config.def.h") }),
  &((Keychord){3, {{WINKEY, XK_v},{0, XK_s},{0, XK_l}},             spawn,                  SHCMD(TERMINAL " -n suckless -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/slock/config.def.h") }),
  &((Keychord){3, {{WINKEY, XK_v},{0, XK_s},{0, XK_s}},             spawn,                  SHCMD(TERMINAL " -n suckless -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/st/config.def.h") }),
};

/* Button definitions */
/* Click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
  /* click                event mask      button          function        argument */
  { ClkClientWin,         WINKEY,         Button1,        movemouse,      {0} },
  { ClkClientWin,         WINMOD,         Button1,        killclient,     {0} },
  { ClkClientWin,         WINMOD2,        Button1,        killclient,     {.ui = 2 } },
  { ClkClientWin,         WINKEY,         Button2,        defaultgaps,    {0} },
  { ClkClientWin,         WINMOD,         Button2,        togglefloating, {0} },
  { ClkClientWin,         WINKEY,         Button3,        resizemouse,    {0} },
  { ClkClientWin,         WINMOD,         Button3,        gesture,        {0} },
  { ClkClientWin,         WINKEY,         Button4,        incrgaps,       {.i = -1 } },
  { ClkClientWin,         WINKEY,         Button5,        incrgaps,       {.i = +1 } },
  { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
  { ClkLtSymbol,          0,              Button3,        layoutmenu,     {0} },
  { ClkMonNum,            0,              Button4,        focusmon,       {.i = -1} },
  { ClkMonNum,            0,              Button5,        focusmon,       {.i = +1} },
  { ClkRootWin,           0,              Button2,        togglebar,      {0} },
  { ClkRootWin,           WINKEY,         Button2,        togglebar,      {.i = 1 } },
  { ClkStatusText,        0,              Button1,        sigstatusbar,   {.i = 1} },
  { ClkStatusText,        0,              Button2,        sigstatusbar,   {.i = 2} },
  { ClkStatusText,        0,              Button3,        sigstatusbar,   {.i = 3} },
  { ClkStatusText,        0,              Button4,        sigstatusbar,   {.i = 4} },
  { ClkStatusText,        0,              Button5,        sigstatusbar,   {.i = 5} },
  { ClkStatusText,        ShiftMask,      Button1,        sigstatusbar,   {.i = 6} },
  { ClkStatusText,        ShiftMask,      Button2,        sigstatusbar,   {.i = 7} },
  { ClkStatusText,        ShiftMask,      Button3,        sigstatusbar,   {.i = 8} },
  { ClkStatusText,        ShiftMask,      Button4,        sigstatusbar,   {.i = 9} },
  { ClkStatusText,        ShiftMask,      Button5,        sigstatusbar,   {.i = 10} },
  { ClkStatusText,        ControlMask,    Button1,        sigstatusbar,   {.i = 11} },
  { ClkStatusText,        ControlMask,    Button2,        sigstatusbar,   {.i = 12} },
  { ClkStatusText,        ControlMask,    Button3,        sigstatusbar,   {.i = 13} },
  { ClkStatusText,        ControlMask,    Button4,        sigstatusbar,   {.i = 14} },
  { ClkStatusText,        ControlMask,    Button5,        sigstatusbar,   {.i = 15} },
  { ClkStatusText,        WINKEY,         Button1,        spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwm/config.def.h") },
  { ClkStatusText,        WINMOD,         Button1,        spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dwmblocks/config.def.h") },
  { ClkStatusText,        WINMOD2,        Button1,        spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/dmenu/config.def.h") },
  { ClkStatusText,        WINKEY,         Button3,        spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/st/config.def.h") },
  { ClkStatusText,        WINMOD,         Button3,        spawn,          SHCMD(TERMINAL " -e nvim ${XDG_SOURCES_HOME:-${HOME}/.local/src}/suckless/slock/config.def.h") },
  { ClkTagBar,            0,              Button1,        view,           {0} },
  { ClkTagBar,            WINKEY,         Button1,        tag,            {0} },
  { ClkTagBar,            ULTRAKEY,       Button1,        nview,          {0} },
  { ClkTagBar,            0,              Button2,        spawntag,       {0} },
  { ClkTagBar,            0,              Button3,        toggleview,     {0} },
  { ClkTagBar,            WINKEY,         Button3,        toggletag,      {0} },
  { ClkTagBar,            ULTRAKEY,       Button3,        ntoggleview,    {0} },
  { ClkTagBar,            0,              Button4,        shiftview,      {.i = 1 } },
  { ClkTagBar,            0,              Button5,        shiftview,      {.i = -1 } },
  { ClkWinTitle,          0,              Button2,        zoom,           {0} },
};

