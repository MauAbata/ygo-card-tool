#include "cargs.h"
#include "strings.h"
#include "ygo_database.h"
#include "ygo_nfc.h"
#include <hd.h>
#include <stdint.h>
#include <stdlib.h>
#include <ygo_errno.h>

static struct cag_option options[] = {
    {
        .identifier = 'h',
        .access_letters = "h",
        .access_name = "help",
        .value_name = NULL,
        .description = HELP_ARG_HELP
    },
    {
        .identifier = 'i',
        .access_letters = "i",
        .access_name = "id",
        .value_name = "ID",
        .description = "Card ID number to search the database for."
    },
    {
        .identifier = 'f',
        .access_letters = "f",
        .access_name = "fetch",
        .value_name = NULL,
        .description = "Fetch card data from the internet, otherwise, a local file will be searched."
    },
    {
        .identifier = 'b',
        .access_letters = "B",
        .access_name = "bin",
        .value_name = "PATH",
        .description = "Specify a binary data file to load data from."
    },
    {
        .identifier = 'j',
        .access_letters = "J",
        .access_name = "json",
        .value_name = "PATH",
        .description = "Specify a JSON data file to load data from."
    },
    {
        .identifier = 'l',
        .access_letters = "L",
        .access_name = "library",
        .value_name = "DIR",
        .description = "Specify a local library directory path to search for card data."
    },
    {
        .identifier = 'n',
        .access_letters = "n",
        .access_name = "dry-run",
        .value_name = NULL,
        .description = "Fetch and display data, but do not write to card."
    },
    {
        .identifier = 'd',
        .access_letters = "d",
        .access_name = "dump",
        .value_name = NULL,
        .description = "Show a hex dump of the data to be written."
    }
};

struct cmd_options {
    uint32_t id;
    int fetch;
    int dry_run;
    int dump;
    const char* bin_file;
    const char* json_file;
    const char* library_dir;
};

static ygo_errno_t usage(const char *invalid);
static ygo_errno_t run(struct cmd_options *opts);

ygo_errno_t cmd_write(int argc, char *argv[]) {
    cag_option_context ctx;
    cag_option_prepare(&ctx, options, CAG_ARRAY_SIZE(options), argc, argv);

    struct cmd_options opts = {
        .id = 0,
        .fetch = 0,
        .dry_run = 0,
        .dump = 0,
        .bin_file = NULL,
        .json_file = NULL,
        .library_dir = NULL,
    };

    while (cag_option_fetch(&ctx)) {
        char id = cag_option_get(&ctx);
        switch (id) {
        default:
        case 'h':
            return usage(cag_option_get_invalid(&ctx));

        case 'i': {
            const char *idstr = cag_option_get_value(&ctx);
            if (idstr == NULL) break;
            opts.id = strtold(idstr, NULL);
            break;
        }

        case 'f': opts.fetch = 1; break;
        case 'b': opts.bin_file = cag_option_get_value(&ctx); break;
        case 'j': opts.json_file = cag_option_get_value(&ctx); break;
        case 'l': opts.library_dir = cag_option_get_value(&ctx); break;
        case 'n': opts.dry_run = 1; break;
        case 'd': opts.dump = 1; break;
        }
    }

    return run(&opts);
}

static ygo_errno_t usage(const char *invalid) {
    if (invalid != NULL) {
        printf("Invalid argument: %s\n\n", invalid);
    }

    printf(
        HELP_USAGE("write") "\n"
        "\n"
        "  Writes card data to a connected NFC tag. The card data may either be loaded from\n"
        "  the online card database, or from a local file or library. If loading from a library\n"
        "  or the online database, --id must be specified. If writing directly from a file,\n"
        "  --id will be ignored. Specifying multiple sources will result in only one being used.\n"
        "\n"
        HELP_ARGS ":\n"
        "\n"
    );

    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);

    printf(
        "\n"
        "Examples:\n"
        "\n"
        "  # Fetch card 06622715 from the online database and write to the card.\n"
        "  ygo write --fetch --id=06622715\n"
        "\n"
        "  # Verify and write the contents of a specific bin file to the card.\n"
        "  ygo write --bin-file=../cards/60279710.bin\n"
        "\n"
        "  # Search the specified library dir for 86066372.json or .bin, verify, and write.\n"
        "  ygo write --library-dir=~/cards --id=86066372\n"
        "\n"
        "  # Shorthand for fetch ID from online database and write to the card.\n"
        "  ygo write -fi 45462149\n"
    );

    return YGO_ERR_BAD_ARGS;
}

ygo_errno_t run(struct cmd_options *opts) {
    ygo_nfc_ctx_t *ctx = NULL;
    ygo_errno_t err = YGO_OK;

    ygo_card_t card = {
        .id = 0x00000000
    };

    if (opts->fetch) {
        if (opts->json_file != NULL || opts->bin_file != NULL) {
            printf("Warning: --fetch specified with data file paths, --json-file and/or --bin-file will be ignored.\n");
        }

        if (opts->id == 0) {
            printf("Error: --fetch specified without --id. This operation is useless.\n");
            return YGO_ERR_BAD_ARGS;
        }

        printf("Fetching card ID: %d...\n", opts->id);
        ygo_db_get_card(opts->id, &card);

        if (card.id != opts->id) {
            printf("Error fetching card: Incorrect or missing data received.\n");
            return YGO_ERR_NO_NETWORK;
        }
    }

    if (card.id == 0) {
        printf("Did not fetch or read binary file...\n");
        return YGO_ERR_BAD_ARGS;
    }

    printf("\n=== Card Data ===\n");
    ygo_card_print(&card);
    printf("=== End Card Data ===\n");

    if (opts->dump) {
        uint8_t raw[144] = {0};
        ygo_card_serialize(raw, &card);
        hd(raw, 144);
    }

    if (opts->dry_run) {
        return YGO_OK;
    }

    printf("Initializing NFC device...\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_init(&ctx));

    printf("Waiting for card read...\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_wait_for_card(ctx));

    printf("Writing out card tag...\n");
    fflush(stdout);
    ERR_CHK_GOTO(ygo_nfc_write_card_tag(ctx, &card));

    printf("Card tag written successfully.\n");

cleanup:
    ygo_nfc_exit(ctx);

    if (err != 0) {
        fprintf(stderr, "ERRNO=%d\n", err);
        fflush(stderr);
    }

    return err;
}
