#ifndef __strings_h
#define __strings_h

#ifdef LANG_JP

#define HELP_SPECIFY_CMD "コマンドを指定する必要があります。"
#define HELP_UNKNOWN_CMD "不明なコマンド"
#define HELP_USAGE "使用法: ygo <コマンド> [オプション...]"
#define HELP_COMMANDS "コマンド"
#define HELP_GLOBAL_ARGS "[[global args]]"
#define HELP_CMD_VIEW "バイナリパックからカードデータを表示する"
#define HELP_CMD_WRITE "バイナリパックをカードタグに送信"
#define HELP_CMD_READ "カードタグからバイナリパックを受け取る"
#define HELP_CMD_ENCODE "JSON カードデータファイルをバイナリパックにエンコードする"
#define HELP_CMD_DECODE "JSON カードデータファイルからバイナリパックをデコードする"
#define HELP_ARG_HELP "Shows the command help"
#define HELP_ARGS "Arguments"

#else

#define HELP_SPECIFY_CMD "You must specify a command."
#define HELP_UNKNOWN_CMD "Unknown command"
#define HELP_USAGE(cmd) "Usage:\n  ygo "cmd" [options...]"
#define HELP_COMMANDS "Commands"
#define HELP_GLOBAL_ARGS "Global Arguments"
#define HELP_CMD_VIEW "View the card data encoded in a binary pack"
#define HELP_CMD_WRITE "Write a binary pack to a card tag"
#define HELP_CMD_READ "Read a binary pack from a card tag"
#define HELP_CMD_ENCODE "Encode a JSON card data file to a binary pack"
#define HELP_CMD_DECODE "Decode a binary pack into a JSON card data file"
#define HELP_ARG_HELP "Shows the command help"
#define HELP_ARGS "Arguments"

#endif

#endif
