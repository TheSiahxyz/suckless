// Modify this file to change what commands output to your statusbar, and recompile using the make command.
static const Block blocks[] = {
    /*Icon*/ /*Command*/ /*Update Interval*/ /*Update Signal*/
    {"", "sb-music", 1, 11},
    /* {"⌨", "sb-kbselect", 0, 30}, */
    {"", "cat /tmp/recordingicon 2>/dev/null", 0, 9},
    {"", "sb-tasks", 10, 26},
    {"", "sb-torrent", 20, 7},
    {"", "sb-packages", 0, 8},
    {"", "sb-news", 0, 6},
    {"", "sb-mailbox", 180, 12},
    /* {"",	"sb-price xmr-btc \"Monero to Bitcoin\" 🔒 34",	9000,	34}, */
    /* {"",	"sb-price xmr Monero 🔒 33",			9000,	33}, */
    /* {"",	"sb-price eth Ethereum 🍸 32",			9000,	32}, */
    /* {"",	"sb-price btc Bitcoin 💰 31",			9000,	31}, */
    {"", "sb-doppler", 0, 13},
    {"", "sb-forecast", 0, 5},
    {"", "sb-cpu", 60, 18},
    {"", "sb-memory", 60, 14},
    {"", "sb-disk", 10800, 20},
    /* {"", "sb-nettraf", 1, 16}, */
    {"", "sb-volume", 0, 10},
    {"", "sb-bghitness", 0, 23},
    {"", "sb-brightness", 0, 22},
    {"", "sb-battery", 5, 3},
    {"", "sb-inputs", 0, 29},
    /* {"",	"sb-moonphase",	18000,	17}, */
    {"", "sb-clock", 60, 1},
    {"", "sb-internet", 5, 4},
    /* {"",	"sb-iplocate", 0,	27}, */
    {"", "sb-ecrypt", 0, 19},
    {"", "sb-help-icon", 0, 15},
};

// Sets delimiter between status commands. NULL character ('\0') means no delimiter.
static char *delim = " ";

// Have dwmblocks automatically recompile and run when you edit this file in
// vim with the following line in your vimrc/init.vim:

// autocmd BufWritePost ~/.local/src/dwmblocks/config.h !cd ~/.local/src/dwmblocks/; sudo make install && { killall -q dwmblocks;setsid dwmblocks & }
