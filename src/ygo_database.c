﻿#include "ygo_database.h"
#include "curl/curl.h"
#include "cJSON.h"

#define YGO_API_URL "https://db.ygoprodeck.com/api/v7/cardinfo.php"
#define YGO_URL_LEN 200

struct curl_buffer_state {
    char *buffer;
    size_t buffsz;
};

static size_t _write_curl_buffer(void *buffer, size_t size, size_t nmemb, void *userp) {
    struct curl_buffer_state *state = (struct curl_buffer_state*) userp;

    // Allocate or reallocate a memory buffer to hold incoming data:
    if (state->buffer == NULL) {
        state->buffer = (char*) malloc(size * nmemb + 1);
    } else {
        state->buffer = (char *)realloc(state->buffer, (size * nmemb) + state->buffsz);
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

ygo_errno_t ygo_db_json_to_card(cJSON *root, ygo_card_t *card) {
    memset(card, 0, sizeof(ygo_card_t));

    cJSON *id = cJSON_GetObjectItem(root, "id");
    if (id != NULL) card->id = id->valueint;

    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (name != NULL) strcpy_s(card->name, YGO_CARD_NAME_MAX_LEN, name->valuestring);

    return YGO_OK;
}
