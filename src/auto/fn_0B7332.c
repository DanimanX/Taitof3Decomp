/* ROM 0x0B7332: UNVERIFIED (capture spans frames): draw a framed text window
 * in text RAM (all arguments on the stack, every register restored).
 * work/ram/fn_0B7332 has every post snapshot several frames after its pre
 * (0x6DD -> 0x6E6, ...), so the harness skips all 30 pairs. Checked by hand
 * instead: every one of the 1259 gfx bytes this translation writes agrees with
 * the capture's post image across all 30 pairs (the capture has extra writes
 * from other code in the frames in between).
 *   (0x24,SP).w = width, (0x26,SP).w = height, (0x28,SP).w = attribute base,
 *   (0x2A,SP).l = {x.w, y.w} text position.
 * The corner/edge glyphs go in at 0x61C000 + 2*x + 0x80*y: the top and bottom
 * rows get the attribute+2 (top) / +3 (bottom) with the far corner ORed with
 * 0x8000, the left/right columns the same, and the interior is cleared to 0.
 * Cells whose low byte is already >= 0x20 (a drawn character) are left alone on
 * the top-left run and at the bottom-left corner. */
#include "fn_auto.h"

#define TEXT_BASE 0x0061C000u

static uint16_t set_byte(uint16_t reg, uint8_t b)
{
    return (uint16_t)((reg & 0xFF00u) | b);
}

int Fn_0B7332(const f3mem_t *m, f3regs_t *r)
{
    /* The movem.l saves 32 bytes, so the arguments sit at SP+0x24.. of the
     * post-movem SP, i.e. SP+4.. of the entry SP (a[7]). */
    uint32_t ap = r->a[7];
    uint16_t d2 = rd16(m, ap + 4u);                     /* width */
    uint16_t d3 = rd16(m, ap + 6u);                     /* height */
    uint16_t d4 = rd16(m, ap + 8u);                     /* attribute base */
    uint32_t a0 = rd32(m, ap + 0x0Au);                  /* {x.w, y.w} */

    uint16_t t0 = rd16(m, a0);                          /* 0x0B7346 move.w (A0)+,D0w */
    a0 += 2u;
    uint16_t t1 = rd16(m, a0);                          /* 0x0B7348 */
    a0 += 2u;

    uint32_t p = TEXT_BASE;                             /* 0x0B734A */
    p += (uint32_t)(int32_t)(int16_t)(uint16_t)(t0 + t0);   /* 0x0B7350..0x0B7354 */
    p += (uint32_t)(int32_t)(int16_t)(uint16_t)(t1 << 7);   /* 0x0B7352..0x0B7356 */

    uint16_t d4w = (uint16_t)(d4 << 8);                 /* 0x0B7358 lsl.w #8 */
    d4w = (uint16_t)(d4w << 1);                         /* 0x0B735A lsl.w #1 */

    uint16_t off0 = (uint16_t)((uint16_t)(d2 - 1u) + (uint16_t)(d2 - 1u)); /* D0 */
    uint16_t off1 = (uint16_t)((uint16_t)(d3 - 1u) << 7);                 /* D1 */
    uint32_t a1 = p + 2u;                               /* 0x0B7368 */
    int16_t cnt2 = (int16_t)(d2 - 3u);
    d4w = set_byte(d4w, 0x02u);                         /* 0x0B736E move.b #2 */

    for (;;) {                                          /* 0x0B7372 */
        uint16_t d5 = rd16(m, a1);
        if ((int8_t)(uint8_t)d5 < 0x20)                 /* 0x0B7374 cmpi.b/bge */
            wr16(m, a1, d4w);
        wr16(m, a1 + off1, d4w);                        /* 0x0B737E */
        a1 += 2u;
        if (--cnt2 < 0)                                 /* 0x0B7386 dbf */
            break;
    }

    a1 = p + 0x80u;                                     /* 0x0B738A lea (0x80,A0),A1 */
    int16_t cnt3 = (int16_t)(d3 - 3u);
    d4w = set_byte(d4w, 0x03u);                         /* 0x0B7390 */

    for (;;) {                                          /* 0x0B7394 */
        wr16(m, a1, d4w);
        wr16(m, a1 + off0, d4w);                        /* 0x0B7396 */
        uint16_t d2x = 2u;                              /* 0x0B739A */
        do {
            wr16(m, a1 + d2x, 0u);                      /* 0x0B739E clr.w */
            d2x = (uint16_t)(d2x + 2u);
        } while ((int16_t)d2x < (int16_t)off0);         /* 0x0B73A4 cmp/blt */
        a1 += 0x80u;
        if (--cnt3 < 0)                                 /* 0x0B73AC dbf */
            break;
    }

    d4w = set_byte(d4w, 0x04u);                         /* 0x0B73B0 move.b #4 */
    wr16(m, p, d4w);                                    /* 0x0B73B4 */
    d4w |= 0x8000u;                                     /* 0x0B73B6 bset.l #0xf */
    wr16(m, p + off1, d4w);                             /* 0x0B73BA */
    d4w = (uint16_t)(d4w & ~0x8000u);                   /* 0x0B73BE bclr.l #0xf */
    p += (uint32_t)(int32_t)(int16_t)off0;              /* 0x0B73C2 adda.w */
    d4w |= 0x0100u;                                     /* 0x0B73C4 bset.l #8 */
    {
        uint16_t d5 = rd16(m, p);                       /* 0x0B73C8 */
        if ((int8_t)(uint8_t)d5 < 0x20)                 /* 0x0B73CA */
            wr16(m, p, d4w);                            /* 0x0B73D2 */
    }
    d4w |= 0x8000u;                                     /* 0x0B73D4 */
    wr16(m, p + off1, d4w);                             /* 0x0B73D8 */
    return 0;                                           /* 0x0B73DC movem restore */
}
