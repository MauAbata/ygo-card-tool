/**
 * main.c - Primary function is to parse base command line and forward that to the relevant command
 * handlers. This is an entrypoint, not an implementation. Keep it simple.
 */

#include "hd.h"
#include "ygo_card.h"
#include <cJSON.h>
//#include <getopt.h>
#include "curl/curl.h"
#include "ygo_errno.h"
#include "ygo_nfc.h"
#include <stdio.h>
#include <string.h>
#include <ygo_database.h>
#include <cargs.h>
#include "strings.h"

const char *CMD_VIEW = "view";
ygo_errno_t cmd_view(int argc, char *argv[]);

const char *CMD_WRITE = "write";
ygo_errno_t cmd_write(int argc, char *argv[]);

const char *CMD_READ = "read";
ygo_errno_t cmd_read(int argc, char *argv[]);

const char *CMD_FETCH = "fetch";
ygo_errno_t cmd_fetch(int argc, char *argv[]);

static struct cag_option options[] = {
    {
        .identifier = 'h',
        .access_letters = "h",
        .access_name = "help",
        .description = HELP_ARG_HELP
    }
};

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
    if (argc < 2) return usage("");
    char *command = argv[1];

    // Offset args until the command name becomes argv[0]
    int cmd_argc = argc - 1;
    char** cmd_argv = argv + 1;

    if (!strcmp(CMD_WRITE, command)) {
        return cmd_write(cmd_argc, cmd_argv);
    }

    if (!strcmp(CMD_VIEW, command)) {
        printf("View Card from Binary File\n");
        return YGO_OK;
    }

    if (!strcmp(CMD_READ, command)) {
        return nfc_read();
    }

    if (!strcmp(CMD_FETCH, command)) {
        return YGO_OK;
    }

    return usage(command);
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


ygo_errno_t usage(const char *cmd) {
    if (cmd != NULL) {
        if (cmd[0] == '\0')
            printf(HELP_SPECIFY_CMD "\n");
        else
            printf(HELP_UNKNOWN_CMD ": %s\n", cmd);
    }

    printf(
        HELP_USAGE("<command>") "\n"
        "\n"
        HELP_COMMANDS ":\n"
        "  view                 " HELP_CMD_VIEW "\n"
        "  write                " HELP_CMD_WRITE "\n"
        "  read                 " HELP_CMD_READ "\n"
        "  encode               " HELP_CMD_ENCODE "\n"
        "  decode               " HELP_CMD_DECODE "\n"
    );

    return YGO_ERR_BAD_ARGS;
}
