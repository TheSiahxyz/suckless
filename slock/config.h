/* user and group to drop privileges to */
static const char *group = "wheel";
static int personalblur = 100;

static const char *colorname[NUMCOLS] = {
    [INIT] =   "#3c3836",         /* after initialization */
    [INPUT] =  "#005577",       /* during input */
    [INPUT_ALT] = "#227799",    /* during input, second color */
    [FAILED] = "#CC3333",       /* wrong password */
    [CAPS] = "#FF0000",             /* CapsLock on */
    [CAPS_ALT] = "#FFA666",     /* hypothetical alternate color for CapsLock on */
    [PAM] =    "#9400D3",       /* waiting for PAM */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;

/* Background image path, should be available to the user above */
static const char* background_image = "";

/* default message */
static const char *message = "THESIAH";

/* text color */
static const char *text_color = "#131313";

/* text size */
static const char *font_name = "monospace:size=18:bold";

/* time in seconds before the monitor shuts down */
static const int monitortime = 600;

/* PAM service that's used for authentication */
static const char* pam_service = "system-login";

/* insert grid pattern with scale 1:1, the size can be changed with logosize */
static const int logosize = 75;
/* grid width and height for right center alignment */
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

/*Enable blur*/
#define BLUR
/*Set blur radius*/
static int blurRadius=5;
/*Enable Pixelation*/
//#define PIXELATION
/*Set pixelation radius*/
static const int pixelSize=0;

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
    { "color0",    STRING,  &colorname[INIT] },
    { "color4",     STRING,  &colorname[INPUT] },
    { "color12",    STRING,  &colorname[INPUT_ALT] },
    { "color1",     STRING,  &colorname[FAILED] },
    { "color3",     STRING,  &colorname[CAPS] },
    { "color11",    STRING,  &colorname[CAPS_ALT] },
    { "color13",    STRING,  &colorname[PAM] },
    { "color0",     STRING,  &text_color },
};

