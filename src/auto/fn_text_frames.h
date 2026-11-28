#ifndef FN_TEXT_FRAMES_H
#define FN_TEXT_FRAMES_H

#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"
#include "../video/sprite.h"

/* 0x005690: mirror the argument and LINK/MOVEM writes around Gfx_DrawHex. */
static inline void draw_hex(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                            uint32_t value, uint16_t digits, uint32_t dst,
                            uint16_t attr)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_word(m, r, digits);
    push_long(m, r, value);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[0] = Gfx_DrawHex(m, value, digits, dst, attr);
    r->a[7] = sp;
}

/* 0x00571A: mirror the argument and LINK/MOVEM writes around Gfx_DrawDecimal. */
static inline void draw_decimal(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                                uint32_t value, uint16_t digits, uint32_t dst,
                                uint16_t attr)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_word(m, r, digits);
    push_long(m, r, value);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[0]);
    for (unsigned reg = 6; reg > 1;)
        push_long(m, r, r->d[--reg]);
    r->d[0] = Gfx_DrawDecimal(m, value, digits, dst, attr);
    r->a[7] = sp;
}

/* 0x005666: mirror the argument and LINK/MOVEM writes around Text_DrawString. */
static inline void draw_text(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                             uint32_t src, uint32_t dst, uint16_t attr)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_long(m, r, src);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    r->d[0] = Text_DrawString(m, src, dst, attr);
    r->a[7] = sp;
}

#endif
