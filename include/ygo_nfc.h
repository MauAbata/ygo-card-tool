#ifndef __nfc_h
#define __nfc_h

#include "ygo_errno.h"
#include "nfc/nfc.h"
#include "ygo_card.h"

typedef struct {
    nfc_device *device;
    nfc_context *context;
    nfc_target *target;
} ygo_nfc_ctx_t;

/**
 * Initializes an NFC connection for reading card tags.
 * @param ctx
 * @return
 */
ygo_errno_t ygo_nfc_init(ygo_nfc_ctx_t **pctx);

/**
 * Call this to cleanup after yourself.
 * @param ctx
 */
void ygo_nfc_exit(ygo_nfc_ctx_t *ctx);

/**
 * Polls the card reader until a card is detected. Check the error code for timeout
 * or abort. At the end of this, context will be updated with a new target if the return
 * is YGO_OK. If the return is an error, please cleanup your context and exit.
 * @param ctx
 * @return
 */
ygo_errno_t ygo_nfc_wait_for_card(ygo_nfc_ctx_t *ctx);

ygo_errno_t ygo_nfc_wait_for_card_removed(ygo_nfc_ctx_t *ctx);

/**
 * Cancels a pending wait operation. This should be done in an interrupt handler.
 */
void ygo_nfc_cancel_wait(ygo_nfc_ctx_t *ctx);

// Card Data manipulation

/**
 * Validates that the current target tag can in fact store YGO card data.
 * @param ctx
 * @return
 */
ygo_errno_t ygo_nfc_validate_card_tag(ygo_nfc_ctx_t *ctx);

/**
 * Writes Card Data to the target tag.
 * @param ctx
 * @param card
 * @return
 */
ygo_errno_t ygo_nfc_write_card_tag(ygo_nfc_ctx_t *ctx, const ygo_card_t *card);

/**
 * Reads Card Data from the current tag.
 * @param ctx
 * @param card
 * @return
 */
ygo_errno_t ygo_nfc_read_card_tag(ygo_nfc_ctx_t *ctx, ygo_card_t *card);

// Debugging Helpers

void ygo_nfc_dump_card_data(ygo_nfc_ctx_t *ctx);

#endif