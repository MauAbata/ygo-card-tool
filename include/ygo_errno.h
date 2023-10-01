#ifndef __ygo_errno_h
#define __ygo_errno_h

typedef enum {
    YGO_OK = 0x00,
    YGO_ERR_BAD_ARGS,
} ygo_errno_t;

#define ERR_CHK(expr) { ygo_errno_t _err = expr; if (_err != YGO_OK) return _err; }
#endif
