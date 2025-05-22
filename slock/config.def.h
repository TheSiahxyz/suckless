#include <X11/XF86keysym.h>

static const char *colorname[NUMCOLS] = {
  [INIT]      = "#3c3836",  /* after initialization */
  [INPUT]     = "#005577",  /* during input */
  [INPUT_ALT] = "#227799",  /* during input, second color */
  [FAILED]    = "#CC3333",  /* wrong password */
  [CAPS]      = "#FF0000",  /* CapsLock on */
  [CAPS_ALT]  = "#FFA666",  /* hypothetical alternate color for CapsLock on */
  [PAM]       = "#9400D3",  /* waiting for PAM */
};

/* Enable or disable (1 means enable, 0 disable) bell sound when password is incorrect */
static const int xbell = 0;

/* Treat a cleared input like a wrong password (color) */
static const int failonclear = 1;

/* allow control key to trigger fail on clear */
static const int controlkeyclear = 0;

/* Time in seconds before the monitor shuts down */
static const int monitortime = 300;

/* Insert grid pattern with scale 1:1, the size can be changed with logosize */
static const int logosize = 75;
/* Grid width and height for right center alignment */
static const int logow = 12;
static const int logoh = 6;

static XRectangle rectangles[9] = {
	/* x    y       w       h */
	{ 0,    3,      1,      3 },
	{ 1,    3,      2,      1 },
	{ 0,    5,      8,      1 },
	{ 3,    0,      1,      5 },
	{ 5,    3,      1,      2 },
	{ 7,    3,      1,      2 },
	{ 8,    3,      4,      1 },
	{ 9,    4,      1,      2 },
	{ 11,   4,      1,      2 },
};

/* Enable blur */
#define BLUR
/* Set blur radius */
static int blurRadius = 5;
static int privateBlurRadius = 100;
/* Enable Pixelation */
//#define PIXELATION
/* Set pixelation radius */
static const int pixelSize = 0;

/* Background image path, should be available to the user above */
static const char *background_image = "";
static const char *private_image = "Private/photo/19CDB48F-C92D-437D-A65E-F7DD30F0A05F.png";

/* PAM service that's used for authentication */
static const char *pam_service = "system-login";

/* Font settings for the time text */
static const float textsize=64.0;
static const char *textfamily="serif";
static const double textcolorred=255;
static const double textcolorgreen=255;
static const double textcolorblue=255;

/* Default message */
static const char *message = "THESIAH";

/* Text color */
static const char *text_color = "#C6D0F5";

/* Text size (must be a valid size) */
static const char *font_name = "monospace:size=18:bold";

/* Length of entires in scom  */
#define entrylen 3

static const secretpass scom[entrylen] = {
/*	 Password				command */
	{ "reboot",       "loginctl reboot -i" },
	{ "shutdown",     "loginctl poweroff -i" },
	{ "suspend",      "loginctl suspend -i" },
} ;

static const Passthrough passthroughs[] = {
	/* Modifier   Key */
	{ 0,          XF86XK_AudioRaiseVolume },
	{ 0,          XF86XK_AudioLowerVolume },
	{ 0,          XF86XK_AudioMute },
	{ 0,          XF86XK_AudioPause },
	{ 0,          XF86XK_AudioStop },
	{ 0,          XF86XK_AudioNext },
	{ 0,          XF86XK_AudioPrev },
	{ 0,          XF86XK_MonBrightnessUp },
	{ 0,          XF86XK_MonBrightnessDown },
};

