/* ROM 0x0C966E: (recipe capture fnk_, work/zcap/recipes15.tsv): (no usable capture - the only pair is
 * VBlank-interrupted and the test harness rejects it): turret/gun object tick
 * with a queue push (A0 = object).
 * Queues {0x20, 0x0C982C[+0x22 of parent], (parent +0x0C) << 6 | 0x4000} via
 * 0x001660 when its slot is free, places (+0x14,+0x18) from the parent at
 * +0x08 plus (0x13,0x11), eases them by the sine/cosine pair at +0x10 scaled by
 * ((+0x1E) - 0x100) >> 5 and draws the sprite (0x001282), integrates
 * (+0x2C,+0x30) by (+0x2E,+0x32) while +0x2E decays by 0x60, and adds 0x600 to
 * +0x22: while (byte)+0x22 <= 0x77 it re-arms 0x0C966E, else it clears +0x22
 * and moves to 0x0C96EE. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* 68k memory bitfield {offset:width}, offset counted from the MSB of addr. */
static int32_t bfext_mem_s(const f3mem_t *m, uint32_t addr, unsigned off, unsigned w)
{
    uint32_t v = rd32(m, addr);
    v <<= off;
    v >>= (32u - w);
    if (w < 32u && (v & (1u << (w - 1u))) != 0u)
        v |= ~((1u << w) - 1u);
    return (int32_t)v;
}

/* ROM 0x0C978E: place +0x22 into (+0x1E,+0x1F), add the 10-bit signed
 * (+0x2C,+0x30) velocities to (+0x14,+0x18), add the two 16-bit sine/cosine
 * words at +0x10 scaled by ((+0x1E) - 0x100) >> 5, then draw the sprite. */
static void sub_0C978E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr8(m, obj + 0x1Fu, rd8(m, obj + 0x22u));             /* 0x0C978E */
    wr8(m, obj + 0x1Eu, rd8(m, obj + 0x22u));             /* 0x0C9794 */

    int32_t v14 = bfext_mem_s(m, obj + 0x2Cu, 0u, 10u);   /* 0x0C979A bfexts */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)v14));
    int32_t v18 = bfext_mem_s(m, obj + 0x30u, 0u, 10u);   /* 0x0C97A4 bfexts */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)v18));

    uint32_t d0 = (uint32_t)(uint8_t)rd8(m, obj + 0x1Eu); /* 0x0C97AE..0x0C97B4 */
    d0 = (d0 - 0x100u) & 0xFFFFu;
    uint32_t d1 = d0;                                     /* 0x0C97B8 */
    uint32_t a1 = rd32(m, obj + 0x10u);                   /* 0x0C97BA */

    int32_t s0 = (int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (uint32_t)(s0 >> 5);                             /* 0x0C97BE..0x0C97C0 */
    a1 += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));

    int32_t s1 = (int32_t)(int16_t)(uint16_t)d1 * (int32_t)(int16_t)rd16(m, a1);
    d1 = (uint32_t)(s1 >> 5);                             /* 0x0C97C6..0x0C97C8 */
    a1 += 2u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d1));

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;

    draw_sprite_regs(m, r);                               /* 0x0C97CE -> 0x001282 */
}

/* ROM 0x001660: push {key.w = 0x20, ptr.l, word.l} onto the inter-task queue
 * at A5-0x6750 (8-byte slots, at most 0x20 entries) if it is not full. */
static void queue_push_001660(const f3mem_t *m, uint32_t ptr, uint16_t word)
{
    uint16_t idx = rd16(m, F3_A5 - 0x5DF0u);              /* 0x001678 */
    if ((int16_t)idx >= 0x20)                            /* 0x00167C bge 0x169E */
        return;

    uint32_t a2 = F3_A5 - 0x6750u + (uint32_t)(int16_t)(uint16_t)(idx << 3); /* 0x001682..0x001688 */
    wr16(m, a2, 0x0020u);                                /* 0x00168A move.w D1w,(A2) */
    wr32(m, a2 + 2u, ptr);                               /* 0x00168C move.l A0,(2,A2) */
    wr16(m, a2 + 6u, word);                              /* 0x001690 move.w D0w,(6,A2) */
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);                   /* 0x001694 */
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(idx + 1u));      /* 0x00169A */
}

int Fn_0C966E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t a1 = rd32(m, obj + 0x08u);                      /* 0x0C966E */
    uint32_t d0 = setw(r->d[0], rd16(m, a1 + 0x22u));        /* 0x0C9672 */
    uint32_t d1 = (uint32_t)(uint8_t)rd8(m, a1 + 0x0Cu);     /* 0x0C9676..0x0C9678 */
    d1 = (((d1 << 6) & 0xFFFFu) + 0x4000u) & 0xFFFFu;        /* 0x0C967C..0x0C967E */
    d0 = rd32(m, 0x000C982Cu + 4u * (uint32_t)(int32_t)(int16_t)(uint16_t)d0); /* 0x0C9682 */

    /* 0x0C968A..0x0C969A: args 0x20, D1, D0 -> 0x001660 (A0 = D0, D0 = D1,
     * D1 = 0x20; every register it touches is saved by its movem). */
    queue_push_001660(m, d0, (uint16_t)d1);

    a1 = rd32(m, obj + 0x08u);                               /* 0x0C969E */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0013u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0011u));

    sub_0C978E(m, r);                                        /* 0x0C96BA bsr */

    d0 = setw(r->d[0], rd16(m, obj + 0x2Eu));                /* 0x0C96BE */
    wr16(m, obj + 0x2Cu, (uint16_t)(rd16(m, obj + 0x2Cu) + (uint16_t)d0));
    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) - 0x0060u));
    d0 = setw(d0, rd16(m, obj + 0x32u));                     /* 0x0C96CC */
    wr16(m, obj + 0x30u, (uint16_t)(rd16(m, obj + 0x30u) + (uint16_t)d0));
    r->d[0] = d0;

    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x0600u)); /* 0x0C96D4 */
    if ((uint8_t)rd8(m, obj + 0x22u) <= 0x77u) {             /* 0x0C96DA bls 0x0C9666 */
        wr32(m, obj, 0x000C966Eu);
        return 0;
    }

    wr16(m, obj + 0x22u, 0u);                                /* 0x0C96E2 clr.w */
    wr32(m, obj, 0x000C96EEu);                               /* 0x0C96E6 */
    return 0;
}
