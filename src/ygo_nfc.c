#include "ygo_nfc.h"
#include "mifare.h"
#include "hd.h"
#include <stdlib.h>
#include <string.h>

ygo_errno_t ygo_nfc_init(ygo_nfc_ctx_t *ctx) {
    if (ctx == NULL) return YGO_ERR_BAD_ARGS;

    nfc_init(&ctx->context);
    if (ctx->context == NULL) {
        ERR_LOG("Unable to initialize NFC library.");
        return YGO_ERR_MEMORY_FAIL;
    }

    ctx->device = nfc_open(ctx->context, NULL);
    if (ctx->device == NULL) {
        ERR_LOG("Unable to open NFC device.");
        ygo_nfc_exit(ctx);
        return YGO_ERR_DEVICE_FAIL;
    }

    if (nfc_initiator_init(ctx->device) < 0) {
        nfc_perror(ctx->device, "nfc_initiator_init");
        ERR_LOG("Unable to initiate communication with NFC device.");
        ygo_nfc_exit(ctx);
        return YGO_ERR_DEVICE_FAIL;
    }

    return YGO_OK;
}

void ygo_nfc_exit(ygo_nfc_ctx_t *ctx) {
    if (ctx->device != NULL) {
        nfc_close(ctx->device);
        ctx->device = NULL;
    }

    if (ctx->context != NULL) {
        nfc_exit(ctx->context);
        ctx->context = NULL;
    }

    if (ctx->target != NULL) {
        free(ctx->target);
        ctx->target = NULL;
    }
}

ygo_errno_t ygo_nfc_wait_for_card(ygo_nfc_ctx_t *ctx) {
    if (ctx->context == NULL || ctx->device == NULL) return YGO_ERR_DEVICE_FAIL;

    const uint8_t nfc_poll_count = 20;
    const uint8_t nfc_poll_period = 2;
    const nfc_modulation nfc_modulations[] = {
        { .nmt = NMT_ISO14443A, .nbr = NBR_106 },
        { .nmt = NMT_ISO14443B, .nbr = NBR_106 },
        { .nmt = NMT_FELICA, .nbr = NBR_212 },
        { .nmt = NMT_FELICA, .nbr = NBR_424 },
        { .nmt = NMT_JEWEL, .nbr = NBR_106 },
        { .nmt = NMT_ISO14443BICLASS, .nbr = NBR_106 },
    };
    const size_t nfc_modulation_count = sizeof(nfc_modulations) / sizeof(nfc_modulation);

    // Allocate a target.
    ctx->target = (nfc_target*) malloc(sizeof(nfc_target));

    int res = nfc_initiator_poll_target(
        ctx->device,
        nfc_modulations,
        nfc_modulation_count,
        nfc_poll_count,
        nfc_poll_period,
        ctx->target
    );

    if (res < 0) {
        nfc_perror(ctx->device, "nfc_initiator_poll_target");
        if (ctx->target != NULL) {
            free(ctx->target);
            ctx->target = NULL;
        }
        return YGO_ERR_TAG_NOT_FOUND;
    }

    return YGO_OK;
}

ygo_errno_t ygo_nfc_wait_for_card_removed(ygo_nfc_ctx_t *ctx) {
    if (ctx->context == NULL || ctx->device == NULL) return YGO_ERR_DEVICE_FAIL;
    // TODO - This might be locking up the device. Also, just don't call it. There's no break. Bad.
    while (0 == nfc_initiator_target_is_present(ctx->device, NULL)) {}
    nfc_perror(ctx->device, "nfc_initiator_target_is_present");
    return YGO_OK;
}

void ygo_nfc_cancel_wait(ygo_nfc_ctx_t *ctx) {
    if (ctx->device != NULL) {
        nfc_abort_command(ctx->device);
    }
}

// Card Data manipulation

ygo_errno_t ygo_nfc_validate_card_tag(ygo_nfc_ctx_t *ctx) {
    return YGO_ERR_NOT_IMPL;
}

typedef uint8_t nfc_tag_sys_data_t[16];
static ygo_errno_t _read_system_data(ygo_nfc_ctx_t *ctx, nfc_tag_sys_data_t data) {
    if (ctx->context == NULL) return YGO_ERR_DEVICE_FAIL;
    if (ctx->device == NULL) return YGO_ERR_DEVICE_FAIL;
    if (ctx->target == NULL) return YGO_ERR_TAG_NOT_FOUND;

    mifare_param mp;
    nfc_initiator_mifare_cmd(ctx->device, MC_READ, 0x00, &mp);

    // This can later be moved to a debug macro?
    printf("Read system data:\n");
    hd(mp.mpd.abtData, 16);
    memcpy(data, mp.mpd.abtData, 16);

    return YGO_OK;
}

ygo_errno_t ygo_nfc_write_card_tag(ygo_nfc_ctx_t *ctx, const ygo_card_t *card) {
    nfc_tag_sys_data_t sys;
    ygo_errno_t err = YGO_OK;

    ERR_CHK(_read_system_data(ctx, sys));
    printf("Writing out card: %s\n", card->name);

    uint8_t raw[144] = {0};
    ygo_card_serialize(raw, card);

    printf("Dumping card data:\n");
    hd(raw, 144);

    for (int i = 0; i < 144; i += 4) {
        mifare_param mp;
        memcpy(mp.mpd.abtData, raw + i, 4);
        nfc_initiator_mifare_cmd(ctx->device, MC_WRITE, (0x04 + (i / 4)), &mp);
    }

    printf("Card data in theory loaded.\n");
    return YGO_OK;
}

ygo_errno_t ygo_nfc_read_card_tag(ygo_nfc_ctx_t *ctx, ygo_card_t *card) {
    return YGO_ERR_NOT_IMPL;
}

// Debugging Helpers

void ygo_nfc_dump_card_data(ygo_nfc_ctx_t *ctx);
