#ifndef __ygo_database_h
#define __ygo_database_h

#include <stdint.h>
#include "ygo_errno.h"
#include "ygo_card.h"
#include "cJSON.h"

ygo_errno_t ygo_db_get_card_json(uint32_t card_id, cJSON **root);
ygo_errno_t ygo_db_get_card(uint32_t card_id, ygo_card_t *card);
ygo_errno_t ygo_db_json_to_card(cJSON *root, ygo_card_t *card);

#endif
