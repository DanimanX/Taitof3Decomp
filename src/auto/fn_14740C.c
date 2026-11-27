#include "fn_auto.h"
/* ROM 0x14740C: stream three playfield strips when forward scroll crosses a block boundary. */
#include "fn_call_frames.h"
#include "../video/sprite.h"

/* 0x14744C, 0x147486 and 0x1474C0 use identical call frames. */
static void draw_strip(const f3mem_t *m, f3regs_t *r, uint32_t src,
                       uint32_t base, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);
    r->a[0] = src;
    push_long(m, r, r->a[1]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);

    push_word(m, r, 0u);
    uint32_t flags = rd32(m, r->a[7]);
    r->d[1] = (uint32_t)((int32_t)(int16_t)r->d[1] * 0x80);
    r->d[0] = set_word(r->d[0], (uint16_t)((r->d[0] << 2) + r->d[1]));
    r->a[1] = base + (uint32_t)(int32_t)(int16_t)r->d[0];
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, ret);
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);
    push_long(m, r, 0x0053DEu);                      /* 0x0053DC direction call */
    Gfx_DrawCompressedTilemap(m, src, r->a[1], flags);

    r->a[7] = sp - 44u;                              /* 0x14747E MOVEM */
    for (unsigned i = 0; i < 8; ++i, r->a[7] += 4u)
        r->d[i] = rd32(m, r->a[7]);
    r->a[1] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    r->d[0] = rd32(m, r->a[7]);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->a[7] = sp;
}

static uint16_t scroll_block(uint16_t scroll)
{
    return (uint16_t)((int16_t)(uint16_t)(scroll - 0x20u) >> 7);
}

int Fn_14740C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t scroll = rd16(m, a5 - 0x5DA8u);
    uint16_t previous = rd16(m, a5 + 0x41E4u);
    r->d[0] = set_word(r->d[0], scroll);
    r->d[1] = set_word(r->d[1], previous);

    if (scroll > previous) {                        /* 0x147416 BLS */
        uint16_t block = scroll_block(scroll);
        uint16_t old_block = scroll_block(previous);
        r->d[0] = set_word(r->d[0], block);
        r->d[1] = set_word(r->d[1], old_block);
        if (block != old_block) {                   /* 0x147430 BEQ */
            uint16_t next = (uint16_t)(block + 3u);
            r->d[6] = set_word(r->d[6], next);
            r->d[0] = set_word(r->d[0], (uint16_t)((next & 3u) << 3));
            r->d[1] = set_word(r->d[1], 8u);
            draw_strip(m, r, 0x06AC64u, 0x610000u, 0x14747Au);
            draw_strip(m, r, 0x06ACFEu, 0x611000u, 0x1474B4u);
            draw_strip(m, r, 0x06AD9Eu, 0x612000u, 0x1474EEu);
        }
    }

    r->a[7] += 4u;                                  /* 0x1474FA RTS */
    return 0;
}
