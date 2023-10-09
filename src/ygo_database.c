#include "ygo_database.h"
#include "cJSON.h"
#include "curl/curl.h"
#include <ygo_card.h>

#define YGO_API_URL "https://db.ygoprodeck.com/api/v7/cardinfo.php"
#define YGO_URL_LEN 200

struct curl_buffer_state {
    char *buffer;
    size_t buffsz;
};

// TODO: There's some serious heap corrpution going on here.
static size_t _write_curl_buffer(void *buffer, size_t size, size_t nmemb, void *userp) {
    struct curl_buffer_state *state = (struct curl_buffer_state*) userp;

    // Allocate or reallocate a memory buffer to hold incoming data:
    if (state->buffer == NULL) {
        state->buffer = (char*) malloc(size * nmemb + 1);
    } else {
        state->buffer = (char *)realloc(state->buffer, (size * nmemb) + state->buffsz + 1);
    }

    if (state->buffer == NULL) return CURL_WRITEFUNC_ERROR;

    memcpy(state->buffer + state->buffsz, buffer, size * nmemb);
    state->buffsz += (size * nmemb);
    state->buffer[state->buffsz] = '\0';
    return (size * nmemb);
}

ygo_errno_t ygo_db_get_card_json(uint32_t card_id, cJSON **root) {
    if (root == NULL) return YGO_ERR_BAD_ARGS;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        char url[YGO_URL_LEN];
        snprintf(url, YGO_URL_LEN, "%s?id=%d", YGO_API_URL, card_id);

        struct curl_buffer_state state = {
            .buffer = NULL,
            .buffsz = 0,
        };

        printf("Fetching %s\n", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_curl_buffer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &state);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(state.buffer);
            return YGO_ERR_BAD_FETCH;
        }

        curl_easy_cleanup(curl);

        // Parse JSON on Buffer
        cJSON *response = cJSON_Parse(state.buffer);
        free(state.buffer);

        if (response == NULL) {
            printf("Failed to decode JSON response.\n");
            return YGO_ERR_BAD_FETCH;
        }

        cJSON *data = cJSON_GetObjectItem(response, "data");
        cJSON *item = data != NULL ? cJSON_GetArrayItem(data, 0) : NULL;

        if (item == NULL) {
            printf("Expected data was not found in network response.\n");
            return YGO_ERR_BAD_FETCH;
        }

        cJSON_DetachItemViaPointer(data, item);
        cJSON_Delete(response);
        *root = item;

        return YGO_OK;
    }

    return YGO_ERR_NO_NETWORK;
}

ygo_errno_t ygo_db_get_card(uint32_t card_id, ygo_card_t *card) {
    if (card == NULL) return YGO_ERR_BAD_ARGS;
    memset(card, 0x00, sizeof(ygo_card_t));

    cJSON *root = NULL;
    ygo_db_get_card_json(card_id, &root);

    if (root == NULL) {
        printf("Attempt to fetch and decode card data failed.\n");
        return YGO_ERR_BAD_FETCH;
    }

    ygo_errno_t err = ygo_db_json_to_card(root, card);
    cJSON_Delete(root);
    return err;
}

void _assign_card_type(ygo_card_t *card, char *valuestring) {
    if (valuestring == NULL) return;
    char *type = NULL;
    char *ptr = NULL;

    card->type = 0;
    card->ability = 0;
    card->summon = 0;
    card->flags = 0x00;

    LOGD("Checking \'%s\' for tokens to give to \'%s\'\n", valuestring, card->name);
    while ((type = strtok_s(ptr == NULL ? valuestring : NULL, " ", &ptr)) != NULL) {
        LOGD("Checking type token: %s (%p)\n", type, ptr);

        if (card->type == 0) {
            ygo_card_type_t ct = ygo_card_type_from_str(type);
            if (ct != -1) {
                card->type = ct;
                continue;
            }
        }

        if (card->ability == 0) {
            ygo_monster_ability_t tb = ygo_monster_ability_from_str(type);
            if (tb != -1) {
                card->ability = tb;
                continue;
            }
        }

        if (card->summon == 0) {
            ygo_summon_type_t st = ygo_summon_type_from_str(type);
            if (st != -1) {
                card->summon = st;
                continue;
            }
        }

        ygo_monster_flag_t fl = ygo_monster_flag_from_str(type);
        if (fl != -1) {
            LOGD("Type %s was a flag.\n", type);
            card->flags = (unsigned)card->flags | (unsigned)fl;
            continue;
        }
    }
}

void _assign_link_markers(ygo_card_t *card, cJSON *root) {
    card->link_markers = 0x00;

    cJSON *node = NULL;
    cJSON_ArrayForEach(node, root) {
        LOGD("Checking link marker token: %s\n", node->valuestring);
        ygo_card_link_markers_t lm = ygo_card_link_markers_from_str(node->valuestring);
        if (lm != -1) {
            card->link_markers = (unsigned)card->link_markers | (unsigned)lm;
        }
    }
}

#define get(key) data = cJSON_GetObjectItem(root, key); if (data != NULL)

ygo_errno_t ygo_db_json_to_card(cJSON *root, ygo_card_t *card) {
    memset(card, 0, sizeof(ygo_card_t));
    cJSON *data = NULL;

    get("id") card->id = data->valueint;
    get("name") strcpy_s(card->name, YGO_CARD_NAME_MAX_LEN, data->valuestring);
    get("type") _assign_card_type(card, data->valuestring);
    get("race") card->monster_type = ygo_monster_type_from_str(data->valuestring);
    get("attribute") card->attribute = ygo_attribute_from_str(data->valuestring);
    get("atk") card->atk = data->valueint;
    get("def") card->def = data->valueint;
    get("level") card->level = data->valueint;
    get("scale") card->scale = data->valueint;
    get("linkval") card->link_value = data->valueint;
    get("linkmarkers") _assign_link_markers(card, data);

    return YGO_OK;
}
