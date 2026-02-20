#ifndef CONFIG_H
#define CONFIG_H

// String used to delimit block outputs in the status.
#define DELIMITER " "

// Maximum number of Unicode characters that a block can output.
#define MAX_BLOCK_OUTPUT_LENGTH 50

// Control whether blocks are clickable.
#define CLICKABLE_BLOCKS 1

// Control whether a leading delimiter should be prepended to the status.
#define LEADING_DELIMITER 0

// Control whether a trailing delimiter should be appended to the status.
#define TRAILING_DELIMITER 0

// Define blocks for the status feed as X(icon, cmd, interval, signal).
#define BLOCKS(X)\
  X("", "sb-ticker",                                    0,      23)   \
  X("", "cat /tmp/recordingicon 2>/dev/null",           0,      22)   \
  X("", "sb-music",                                     0,      21)   \
  X("", "sb-torrent",                                   0,      20)   \
  X("", "sb-queues",                                    10,     19)   \
  X("", "sb-mailbox",                                   300,    18)   \
  X("", "sb-news",                                      0,      17)   \
  X("", "sb-repos",                                     60,     16)   \
  X("", "sb-tasks",                                     60,     15)   \
  X("", "sb-packages",                                  0,      14)   \
  X("", "sb-forecast",                                  3600,   13)   \
  X("", "sb-cpu",                                       60,     11)   \
  X("", "sb-memory",                                    60,     10)   \
  X("", "sb-disk /home",                                10800,  9)    \
  X("", "sb-keyboard",                                  0,      8)    \
  X("", "sb-brightness",                                0,      7)    \
  X("", "sb-internet",                                  5,      6)    \
  X("", "sb-mic",                                       0,      5)    \
  X("", "sb-volume",                                    0,      4)    \
  X("", "sb-battery",                                   5,      3)    \
  X("", "sb-clock",                                     0,      2)    \
  X("", "sb-ecrypt",                                    0,      1)    \
  X("", "sb-help-icon",                                 0,      0)
#endif  // CONFIG_H

  /* X("", "sb-price xmr-btc \"Monero to Bitcoin\" 🔒 30", 900,    30)   \
  X("", "sb-price xmr Monero 🔒 29",                    900,    29)   \
  X("", "sb-price bnb Binance 🫧 28",                   900,    28)   \
  X("", "sb-price xrp XRP 🪓 27",                       900,    27)   \
  X("", "sb-price usdt Tether ⛺ 26",                   900,    26)   \
  X("", "sb-price eth Ethereum 🍸 25",                  900,    25)   \
  X("", "sb-price btc Bitcoin 💰 24",                   900,    24)   \
  X("", "sb-cpubars",                                   15,     12)   \
  X("", "sb-iplocate",                                  0,      6)    \ */

// Have dwmblocks automatically recompile and run when you edit this file in
// vim with the following line in your vimrc/init.vim:

// autocmd BufWritePost ~/.local/src/suckless/dwmblocks/config.def.h !cd ~/.local/src/suckless/dwmblocks/; sudo make install && { killall -q dwmblocks;setsid -f dwmblocks }

// Although, pkill is slightly slower than the command kill, which can
// make a big difference if we are making semi-frequent changes in a script.
// To signal with kill, we must send the value plus 34. Just remember 34.
// So, 10 + 34 = 44, so we use this command: kill -44 $(pidof dwmblocks)

