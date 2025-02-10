// Modify this file to change what commands output to your statusbar, and recompile using the make command.
static const Block blocks[] = {
  /*Icon*/ /*Command*/ /*Update Interval*/ /*Update Signal (1-31)*/
  {"", "sb-music", 0, 23},            // Music (far left)
  {"", "cat /tmp/recordingicon 2>/dev/null", 0, 24}, // Recording
  {"", "sb-torrent", 20, 22},         // Torrent
  {"", "sb-queues", 10, 21},          // Queues
  {"", "sb-mailbox", 300, 20},        // Mailbox
  {"", "sb-news", 0, 19},             // News
  {"", "sb-repos", 60, 18},           // Git repositories
  {"", "sb-tasks", 60, 17},           // Tasks
  {"", "sb-packages", 0, 16},         // Packages
  {"", "sb-forecast", 10800, 15},     // Weather
  /* {"", "sb-price xmr-btc \"Monero to Bitcoin\" 🔒 30", 9000, 31}, */
  /* {"", "sb-price xmr Monero 🔒 29", 9000, 30}, */
  /* {"", "sb-price bnb Binance 🫧 28", 9000, 29}, */
  /* {"", "sb-price xrp XRP 🪓 27", 9000, 28}, */
  /* {"", "sb-price usdt Tether ⛺ 26", 9000, 27}, */
  /* {"", "sb-price eth Ethereum 🍸 25", 9000, 26}, */
  /* {"", "sb-price btc Bitcoin 💰 24", 9000, 25}, */
  /* {"", "sb-nettraf", 1, 14},          // Network  */
  {"", "sb-cpu", 60, 13},             // CPU
  {"", "sb-memory", 60, 12},          // Memory
  {"", "sb-disk", 10800, 11},         // Disk
  {"", "sb-keyboard", 0, 10},         // Inputs
  {"", "sb-bghitness", 0, 9},         // Background Lightness
  {"", "sb-brightness", 0, 8},        // Brightness
  {"", "sb-internet", 5, 7},          // Internet
  /* {"", "sb-iplocate", 0, 6},          // ip */
  {"", "sb-volume", 0, 5},            // Volume
  {"", "sb-battery", 5, 4},           // Battery
  {"", "sb-clock", 60, 3},            // Clock
  {"", "sb-ecrypt", 0, 2},            // Ecrypt (rightmost)
  {"", "sb-help-icon", 0, 1},         // Help Icon (rightmost)
};

// Sets delimiter between status commands. NULL character ('\0') means no delimiter.
static char *delim = " ";

// Have dwmblocks automatically recompile and run when you edit this file in
// vim with the following line in your vimrc/init.vim:

// autocmd BufWritePost ~/.local/src/suckless/dwmblocks/config.def.h !cd ~/.local/src/suckless/dwmblocks/; sudo make install && { killall -q dwmblocks;setsid -f dwmblocks }

// Although, pkill is slightly slower than the command kill, which can
// make a big difference if we are making semi-frequent changes in a script.
// To signal with kill, we must send the value plus 34. Just remember 34.
// So, 10 + 34 = 44, so we use this command: kill -44 $(pidof dwmblocks)

