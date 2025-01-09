// Modify this file to change what commands output to your statusbar, and recompile using the make command.
static const Block blocks[] = {
    /*Icon*/ /*Command*/ /*Update Interval*/ /*Update Signal (unused from 30)*/
    {"", "sb-music", 0, 11},            // Music (far left)
    {"", "cat /tmp/recordingicon 2>/dev/null", 0, 9},
    {"", "sb-torrent", 20, 7},          // Torrent
    {"", "sb-queues", 10, 26},          // Queues
    {"", "sb-mailbox", 180, 12},        // Mailbox
    {"", "sb-news", 0, 6},              // News
    {"", "sb-git", 60, 21},             // Git
    {"", "sb-tasks", 60, 25},           // Tasks
    {"", "sb-packages", 0, 8},          // Packages
    {"", "sb-forecast", 0, 5},          // Weather
    /* {"", "sb-nettraf", 1, 16}, */
    {"", "sb-cpu", 60, 18},             // CPU
    {"", "sb-memory", 60, 14},          // Memory
    {"", "sb-disk", 10800, 20},         // Disk
    /* {"⌨", "sb-kbselect", 0, 30}, */
    {"", "sb-inputs", 0, 29},           // Inputs
    {"", "sb-bghitness", 0, 23},        // Background Lightness
    {"", "sb-brightness", 0, 22},       // Brightness
    {"", "sb-internet", 5, 4},          // Internet
    {"", "sb-volume", 0, 10},           // Volume
    {"", "sb-battery", 5, 3},           // Battery
    {"", "sb-clock", 60, 1},            // Clock
    /* {"", "sb-moonphase", 18000, 17}, */
    /* {"", "sb-price xmr-btc \"Monero to Bitcoin\" 🔒 34", 9000, 34}, */
    /* {"", "sb-price xmr Monero 🔒 33", 9000, 33}, */
    /* {"", "sb-price eth Ethereum 🍸 32", 9000, 32}, */
    /* {"", "sb-price btc Bitcoin 💰 31", 9000, 31}, */
    /* {"", "sb-doppler", 0, 13}, */
    /* {"", "sb-iplocate", 0, 27}, */
    {"", "sb-ecrypt", 0, 19},           // Ecrypt (rightmost)
    {"", "sb-help-icon", 0, 15},        // Help Icon (rightmost)
};

// Sets delimiter between status commands. NULL character ('\0') means no delimiter.
static char *delim = " ";

// Have dwmblocks automatically recompile and run when you edit this file in
// vim with the following line in your vimrc/init.vim:

// autocmd BufWritePost ~/.local/src/dwmblocks/config.h !cd ~/.local/src/dwmblocks/; sudo make install && { killall -q dwmblocks;setsid dwmblocks & }

// Although, pkill is slightly slower than the command kill, which can
// make a big difference if we are making semi-frequent changes in a script.
// To signal with kill, we must send the value plus 34. Just remember 34.
// So, 10 + 34 = 44, so we use this command: kill -44 $(pidof dwmblocks)

