/**
 * main.c - Primary function is to parse base command line and forward that to the relevant command
 * handlers. This is an entrypoint, not an implementation. Keep it simple.
 */

#include "hd.h"
#include "ygo_card.h"
#include <cJSON.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "ygo_errno.h"
#include "curl/curl.h"
#include "nfc/nfc.h"
#include "mifare.h"

const char *CMD_VIEW = "view";
const char *CMD_WRITE = "write";
const char *CMD_READ = "read";
const char *CMD_FETCH = "fetch";

ygo_errno_t usage(const char* cmd);

void fetch(void);

#define MAX_DEVICE_COUNT 16

static nfc_device *pnd = NULL;
static nfc_context *context;

#define ERR(str...) fprintf(stderr, str);

static void stop_polling(int sig)
{
    (void) sig;
    if (pnd != NULL)
        nfc_abort_command(pnd);
    else {
        nfc_exit(context);
    }
}

void
print_nfc_target(const nfc_target *pnt, bool verbose)
{
    char *s;
    str_nfc_target(&s, pnt, verbose);
    printf("%s", s);
    nfc_free(s);
}

void nfc_poll(void) {
    const uint8_t uiPollNr = 20;
    const uint8_t uiPeriod = 2;
    const nfc_modulation nmModulations[6] = {
        { .nmt = NMT_ISO14443A, .nbr = NBR_106 },
        { .nmt = NMT_ISO14443B, .nbr = NBR_106 },
        { .nmt = NMT_FELICA, .nbr = NBR_212 },
        { .nmt = NMT_FELICA, .nbr = NBR_424 },
        { .nmt = NMT_JEWEL, .nbr = NBR_106 },
        { .nmt = NMT_ISO14443BICLASS, .nbr = NBR_106 },
    };
    const size_t szModulations = 6;

    nfc_target nt;
    int res = 0;

    nfc_init(&context);
    if (context == NULL) {
        ERR("Unable to init libnfc (malloc)");
        return;
    }

    pnd = nfc_open(context, NULL);

    if (pnd == NULL) {
        ERR("%s", "Unable to open NFC device.");
        nfc_exit(context);
        return;
    }

    if (nfc_initiator_init(pnd) < 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        nfc_close(pnd);
        nfc_exit(context);
        return;
    }

    printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));
//    printf("NFC device will poll during %ld ms (%u pollings of %lu ms for %" PRIdPTR " modulations)\n", (unsigned long) uiPollNr * szModulations * uiPeriod * 150, uiPollNr, (unsigned long) uiPeriod * 150, szModulations);
    if ((res = nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt))  < 0) {
        nfc_perror(pnd, "nfc_initiator_poll_target");
        nfc_close(pnd);
        nfc_exit(context);
        return;
    }

    if (res > 0) {
        print_nfc_target(&nt, 1);
        printf("Got target.\n");

        {
            // Read out data struct:
            mifare_param mp;
            nfc_initiator_mifare_cmd(pnd, MC_READ, 0x00, &mp);
            printf("Read system data:\n");
            hd(mp.mpd.abtData, 16);
        }

        {
            // Write out a sample card pack
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

            printf("Loaded card: %s\n", card.name);

            uint8_t raw[144] = {0};
            ygo_card_serialize(raw, &card);

            for (int i = 0; i < 144; i += 16) {
                mifare_param mp;
                memcpy(mp.mpd.abtData, raw + i, 16);
                nfc_initiator_mifare_cmd(pnd, MC_WRITE, (0x04 + (i / 16)), &mp);
            }

            printf("Card data in theory loaded.");
        }

            printf("Waiting for card removing...");
        fflush(stdout);
        while (0 == nfc_initiator_target_is_present(pnd, NULL)) {}
        nfc_perror(pnd, "nfc_initiator_target_is_present");
        printf("done.\n");
    } else {
        printf("No target found.\n");
    }

    nfc_close(pnd);
    nfc_exit(context);
}

int main(int argc, char *argv[]) {
    char *command = NULL;
    if (argc > 1) command = argv[1];

    if (command == NULL) return usage(command);

    if (!strcmp(CMD_VIEW, command)) {
        printf("View Card from Binary File\n");
        return YGO_OK;
    }

    if (!strcmp(CMD_READ, command)) {
        nfc_poll();
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
