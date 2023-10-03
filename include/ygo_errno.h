#ifndef __ygo_errno_h
#define __ygo_errno_h

typedef enum {
    YGO_OK = 0x00,
    YGO_ERR_BAD_ARGS,
    YGO_ERR_NOT_IMPL,
    YGO_ERR_MEMORY_FAIL,
    YGO_ERR_DEVICE_FAIL,
    YGO_ERR_TAG_NOT_FOUND,
} ygo_errno_t;

#define ERR_CHK(expr) { ygo_errno_t _err = expr; if (_err != YGO_OK) return _err; }
#define ERR_CHK_GOTO(expr) { ygo_errno_t _err = expr; if (_err != YGO_OK) goto cleanup; }
#define ERR_LOG(str...) { fprintf(stderr, str); fprintf(stderr, "\n"); }
#endif
