/**
 * main.c - Primary function is to parse base command line and forward that to the relevant command
 * handlers. This is an entrypoint, not an implementation. Keep it simple.
 */

#include "ygo_card.h"
#include "ygo_errno.h"
#include <stdio.h>
#include <string.h>
#include "strings.h"

const char *CMD_VIEW = "view";
ygo_errno_t cmd_view(int argc, char *argv[]);

const char *CMD_WRITE = "write";
ygo_errno_t cmd_write(int argc, char *argv[]);

const char *CMD_READ = "read";
ygo_errno_t cmd_read(int argc, char *argv[]);

const char *CMD_FETCH = "fetch";
ygo_errno_t cmd_fetch(int argc, char *argv[]);

ygo_errno_t usage(const char* cmd);

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
        return YGO_OK;
    }

    if (!strcmp(CMD_FETCH, command)) {
        return YGO_OK;
    }

    return usage(command);
}

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
