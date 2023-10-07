/**
 * main.c - Primary function is to parse base command line and forward that to the relevant command
 * handlers. This is an entrypoint, not an implementation. Keep it simple.
 */

#include "hd.h"
#include "ygo_card.h"
#include <cJSON.h>
//#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "ygo_errno.h"
#include "curl/curl.h"
#include "ygo_nfc.h"

const char *CMD_VIEW = "view";
const char *CMD_WRITE = "write";
const char *CMD_READ = "read";
const char *CMD_FETCH = "fetch";

ygo_errno_t usage(const char* cmd);

void fetch(void);

ygo_errno_t nfc_read(void) {
    ygo_errno_t err = YGO_OK;
    ygo_nfc_ctx_t *ctx = NULL;

    ygo_card_t card = {
        .id = 40737112,
        .name = "Dark Magician of Chaos",
        .type = YGO_CARD_TYPE_EFFECT_MONSTER,
        .frame = YGO_CARD_FRAME_EFFECT,
        .subtype = YGO_CARD_STYPE_MONSTER_SPELLCASTER,
        .attribute = YGO_CARD_ATTR_DARK,
        .atk = 2800,
        .def = 2600,
        .rank = 8,
        .scale = 0,
        .link_value = 0,
        .link_markers = 0,
    };

    printf("Initializing NFC device...\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_init(&ctx));

    printf("Waiting for card read...\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_wait_for_card(ctx));

    printf("Writing out card tag...\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_write_card_tag(ctx, &card));

    printf("Complete!\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_wait_for_card_removed(ctx));

cleanup:
    ygo_nfc_exit(ctx);
    if (err != 0)
        printf("ERRNO=%d\n", err);
    return err;
}

int main(int argc, char *argv[]) {
    char *command = NULL;
    if (argc > 1) command = argv[1];
    printf("Executing ...\n");
    fflush(stdout);

    if (command == NULL) return usage(command);

    if (!strcmp(CMD_VIEW, command)) {
        printf("View Card from Binary File\n");
        return YGO_OK;
    }

    if (!strcmp(CMD_READ, command)) {
        return nfc_read();
    }

    if (!strcmp(CMD_FETCH, command)) {
        fetch();
        return YGO_OK;
    }

    return usage(command);
}

void fetch(void) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://db.ygoprodeck.com/api/v7/cardinfo.php?id=65330383");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        printf("HTTP res=%d\n", res);
        curl_easy_cleanup(curl);
    }
}

//void demo(void) {
//    cJSON *root = cJSON_CreateObject();
//    cJSON_AddBoolToObject(root, "test", 1);
//    printf("cJSON Link Test:\n%s\n\n", cJSON_Print(root));
//    cJSON_Delete(root);
//
//    ygo_card_t card = {.id = 40737112,
//        .name = "Dark Magician of Chaos",
//        .type = YGO_CARD_TYPE_EFFECT_MONSTER,
//        .frame = YGO_CARD_FRAME_EFFECT,
//        .subtype = YGO_CARD_STYPE_MONSTER_SPELLCASTER,
//        .attribute = YGO_CARD_ATTR_DARK,
//        .atk = 2800,
//        .def = 2600,
//        .rank = 8,
//        .scale = 0,
//        .link_value = 0,
//        .link_markers = 0};
//
//    printf("Loaded card: %s\n", card.name);
//
//    uint8_t raw[144] = {0};
//    ygo_card_serialize(raw, &card);
//
//    printf("Serializing as raw:\n");
//    hd(raw, sizeof(raw));
//}
//
//ygo_errno_t getopts(options_t *opts, int argc, char *argv[]) {
//    int option_index = 0;
//    int c;
//
//    struct option long_options[] = {
//        { .name = "dump", .has_arg = no_argument, .flag = NULL, .val = 'f'},
//        { 0, 0, 0, 0 }
//    };
//
//    char *optstr = "d";
//
//    while(1) {
//        c = getopt_long(argc, argv, optstr, long_options, &option_index);
//        if (c == -1) break;
//
//        switch (c) { // NOLINT(hicpp-multiway-paths-covered)
//        case 'd': opts->dump = 1; break;
//        default:
//            return usage(argv[0]);
//        }
//    }
//
//    return YGO_OK;
//}

#ifdef LANG_JP
#define HELP_SPECIFY_CMD "コマンドを指定する必要があります。"
#define HELP_UNKNOWN_CMD "不明なコマンド"
#define HELP_USAGE "使用法: ygo <コマンド> [オプション...]"
#define HELP_COMMANDS "コマンド"
#define HELP_CMD_VIEW "バイナリパックからカードデータを表示する"
#define HELP_CMD_WRITE "バイナリパックをカードタグに送信"
#define HELP_CMD_READ "カードタグからバイナリパックを受け取る"
#define HELP_CMD_ENCODE "JSON カードデータファイルをバイナリパックにエンコードする"
#define HELP_CMD_DECODE "JSON カードデータファイルからバイナリパックをデコードする"
#else
#define HELP_SPECIFY_CMD "You must specify a command."
#define HELP_UNKNOWN_CMD "Unknown command"
#define HELP_USAGE "Usage: ygo <command> [options...]"
#define HELP_COMMANDS "Commands"
#define HELP_CMD_VIEW "View the card data encoded in a binary pack"
#define HELP_CMD_WRITE "Write a binary pack to a card tag"
#define HELP_CMD_READ "Read a binary pack from a card tag"
#define HELP_CMD_ENCODE "Encode a JSON card data file to a binary pack"
#define HELP_CMD_DECODE "Decode a binary pack into a JSON card data file"
#endif

ygo_errno_t usage(const char *cmd) {
    if (cmd == NULL) printf(HELP_SPECIFY_CMD "\n");
    else printf(HELP_UNKNOWN_CMD ": %s\n", cmd);

    printf(
        HELP_USAGE "\n"
        "\n"
        HELP_COMMANDS ":\n"
        "    view     - " HELP_CMD_VIEW "\n"
        "    write    - " HELP_CMD_WRITE "\n"
        "    read     - " HELP_CMD_READ "\n"
        "    encode   - " HELP_CMD_ENCODE "\n"
        "    decode   - " HELP_CMD_DECODE "\n"
    );

    return YGO_ERR_BAD_ARGS;
}
