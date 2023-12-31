#ifndef __ygo_errno_h
#define __ygo_errno_h

typedef enum {
    YGO_OK = 0x00,
    YGO_ERR_BAD_ARGS,
    YGO_ERR_NOT_IMPL,
    YGO_ERR_MEMORY_FAIL,
    YGO_ERR_DEVICE_FAIL,
    YGO_ERR_TAG_NOT_FOUND,
    YGO_ERR_NO_NETWORK,
    YGO_ERR_BAD_FETCH,
} ygo_errno_t;

#define ERR_CHK(expr) { ygo_errno_t _err = expr; if (_err != YGO_OK) return _err; }
#define ERR_CHK_GOTO(expr) { err = expr; if (err != YGO_OK) goto cleanup; }
#define ERR_LOG(...) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr); }
#endif
