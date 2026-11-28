#include "fn_auto.h"
#include "fn_call_frames.h"
#include "fn_helpers.h"
#include "../enemy/obj_1194C2.h"

/* ROM 0x0B760C: (capture fnt_: up to the first yield): test-mode selftest panel --
 * a kernel task body (TRAP #5 at 0x0B762E is its yield).
 *
 * Fn_0B760C is the setup before the first yield: it frames a 12x3 box with
 * 0x0B7332 (attribute D1w) around the rect pointed to by A0, and D1w becomes
 * the caller's D0w. Task0B760C_Frame is one pass after the yield:
 *
 *   0B7630  adjust the option index with 0x19FFF0 (counter 0x40CBD0, range
 *           [0..1], step 1, direction byte A5-0x5DDC) -- D1w
 *   0B7662  draw "  OFF  ON " (ROM 0x0B770C) at text row (rect.h+1) with
 *           attribute 7, then the marker string ROM 0x0B7500 at
 *           2*(rect.w+1 + 5*D1w+1), attribute 0xA
 *   0B76CE  A5-0x5DDB bit 5 clear -> D1w = -1 and leave; bit 5 set -> bit 4
 *           set keeps the loop alive (yield again), otherwise leave
 *   0B76EA  unframe with 0x0B73E2 and restore D1/D2/D3/A0
 *
 * The 30 captured pairs for this address all span 4-5 frames (the flow yields
 * to VBlank inside the pair), so tests/test_fn.c skips every one of them as
 * VBlank-interrupted and the capture rejects the routine; it is translated
 * from the bytes and checked only by compilation. */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0B7332: frame a `w` x `h` box at {x.w, y.w} (the rect long the last
 * argument points to) in the text layer at 0x61C000 with attribute `attr`
 * (the third argument). Every register is saved and restored by the ROM, so
 * only the text RAM changes. */
static void sub_0B7332(const f3mem_t *m, uint16_t w, uint16_t h, uint16_t attr, uint32_t rect)
{
    uint16_t x = rd16(m, rect);                                  /* 0x0B7346 */
    uint16_t y = rd16(m, rect + 2u);                             /* 0x0B7348 */
    uint32_t a0 = 0x0061C000u;                                   /* 0x0B734A */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)(x + x);         /* 0x0B7350..0x0B7354 */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)(y << 7);        /* 0x0B7352..0x0B7356 */

    uint32_t d4 = (uint32_t)(uint16_t)(attr << 9);               /* 0x0B7358..0x0B735A */
    uint16_t d0 = (uint16_t)(w - 1u);                            /* 0x0B735C..0x0B7360 */
    uint16_t d1 = (uint16_t)(h - 1u);                            /* 0x0B735E..0x0B7362 */
    d0 = (uint16_t)(d0 + d0);                                    /* 0x0B7364 */
    d1 = (uint16_t)(d1 << 7);                                    /* 0x0B7366 */

    uint32_t a1 = a0 + 2u;                                       /* 0x0B7368 */
    uint16_t d2 = (uint16_t)(w - 3u);                            /* 0x0B736C */
    d4 = (d4 & 0xFFFFFF00u) | 0x02u;                             /* 0x0B736E move.b #2,D4b */
    for (;;) {                                                   /* 0x0B7372 */
        uint16_t d5 = rd16(m, a1);
        if ((int8_t)d5 < 0x20)                                   /* 0x0B7374 cmpi.b #0x20 */
            wr16(m, a1, (uint16_t)d4);
        wr16(m, a1 + (uint32_t)(int32_t)(int16_t)d1, (uint16_t)d4); /* 0x0B737E */
        a1 += 2u;
        if (d2-- == 0u)                                          /* 0x0B7386 dbf */
            break;
    }

    a1 = a0 + 0x80u;                                             /* 0x0B738A */
    uint16_t d3 = (uint16_t)(h - 3u);                            /* 0x0B738E */
    d4 = (d4 & 0xFFFFFF00u) | 0x03u;                             /* 0x0B7390 move.b #3,D4b */
    for (;;) {                                                   /* 0x0B7394 */
        wr16(m, a1, (uint16_t)d4);
        wr16(m, a1 + (uint32_t)(int32_t)(int16_t)d0, (uint16_t)d4);
        d2 = 2u;                                                 /* 0x0B739A */
        for (;;) {                                               /* 0x0B739E */
            wr16(m, a1 + (uint32_t)(int32_t)(int16_t)d2, 0u);
            d2 = (uint16_t)(d2 + 2u);
            if ((int16_t)d2 >= (int16_t)d0)                      /* 0x0B73A4 blt */
                break;
        }
        a1 += 0x80u;                                             /* 0x0B73A8 */
        if (d3-- == 0u)                                          /* 0x0B73AC dbf */
            break;
    }

    d4 = (d4 & 0xFFFFFF00u) | 0x04u;                             /* 0x0B73B0 move.b #4,D4b */
    wr16(m, a0, (uint16_t)d4);                                   /* 0x0B73B4 */
    d4 |= 0x8000u;                                               /* 0x0B73B6 bset #0xf */
    wr16(m, a0 + (uint32_t)(int32_t)(int16_t)d1, (uint16_t)d4);  /* 0x0B73BA */
    d4 &= ~0x8000u;                                              /* 0x0B73BE bclr #0xf */
    a0 += (uint32_t)(int32_t)(int16_t)d0;                        /* 0x0B73C2 */
    d4 |= 0x0100u;                                               /* 0x0B73C4 bset #8 */
    {
        uint16_t d5 = rd16(m, a0);
        if ((int8_t)d5 < 0x20)                                   /* 0x0B73CA */
            wr16(m, a0, (uint16_t)d4);
    }
    d4 |= 0x8000u;                                               /* 0x0B73D4 bset #0xf */
    wr16(m, a0 + (uint32_t)(int32_t)(int16_t)d1, (uint16_t)d4);  /* 0x0B73D8 */
}

/* ROM 0x0B73E2: blank the `w` x `h` box at the rect the third argument points
 * to (top-left {x.w, y.w} of the text layer 0x61C000). Registers are saved. */
void sub_0B73E2(const f3mem_t *m, uint16_t w, uint16_t h, uint32_t rect)
{
    uint16_t x = rd16(m, rect);                                  /* 0x0B73F2 */
    uint16_t y = rd16(m, rect + 2u);                             /* 0x0B73F4 */
    uint32_t a0 = 0x0061C000u;                                   /* 0x0B73F6 */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)(x + x);         /* 0x0B73FC..0x0B7400 */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)(y << 7);        /* 0x0B73FE..0x0B7402 */

    uint16_t d2 = (uint16_t)(w - 1u);                            /* 0x0B7404 */
    uint16_t d3 = (uint16_t)(h - 1u);                            /* 0x0B7406 */
    for (;;) {                                                   /* 0x0B7408 */
        uint32_t a1 = a0;
        uint16_t d0 = d2;
        for (;;) {                                               /* 0x0B740C */
            wr16(m, a0, 0u);
            a0 += 2u;
            if (d0-- == 0u)                                      /* 0x0B740E dbf */
                break;
        }
        a0 = a1 + 0x80u;
        if (d3-- == 0u)                                          /* 0x0B7416 dbf */
            break;
    }
}

/* ROM 0x0B760C: setup before the first TRAP #5 (0x0B762E). */
int Fn_0B760C(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[0]);                                    /* 0x0B760C movem {D1 D2 D3 A0} */
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);                                    /* 0x0B7610 movem {D0} */
    push_long(m, r, r->a[0]);                                    /* 0x0B7614 */
    push_word(m, r, (uint16_t)r->d[1]);                          /* 0x0B7616 */
    push_word(m, r, 3u);                                         /* 0x0B7618 */
    push_word(m, r, 0x000Cu);                                    /* 0x0B761C */
    push_long(m, r, 0x000B7624u);
    sub_0B7332(m, 0x000Cu, 3u, (uint16_t)r->d[1], r->a[0]);      /* 0x0B7620 */
    r->a[7] += 14u;                        /* 0x0B7624 lea 0xa(SP) + the rts' return PC */
    r->d[0] = rd32(m, r->a[7]);                                  /* 0x0B7628 movem (SP)+,{D0} */
    r->a[7] += 4u;
    r->d[1] = setw(r->d[1], (uint16_t)r->d[0]);                  /* 0x0B762C move.w D0w,D1w */
    return 0;                                                    /* 0x0B762E TRAP #5 */
}

/* ROM 0x0B7630: one pass of the loop after the yield. Returns 1 when the pass
 * chose to yield again (0x0B76E6 -> TRAP #5) and 0 when it ran the exit tail
 * (0x0B76EA..0x0B770A) and unwound the frame. */
int Task0B760C_Frame(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->d[0]);                                    /* 0x0B7630 movem {D0} */
    push_long(m, r, r->a[0]);                                    /* 0x0B7634 movem {A0} */
    push_word(m, r, 1u);                                         /* 0x0B7638 */
    push_word(m, r, 1u);                                         /* 0x0B763C */
    push_word(m, r, 0u);                                         /* 0x0B7640 */
    push_word(m, r, (uint16_t)r->d[1]);                          /* 0x0B7644 */
    r->d[0] = setw(r->d[0], (uint16_t)(int16_t)(int8_t)rd8(m, F3_A5 - 0x5DDCu));
    push_word(m, r, (uint16_t)r->d[0]);                          /* 0x0B764C */
    push_long(m, r, 0x000B7654u);
    Fn_19FFF0(m, r);                                             /* 0x0B764E */
    r->a[7] += 10u;                                              /* 0x0B7654 lea 0xa(SP) */
    r->a[0] = rd32(m, r->a[7]);                                  /* 0x0B7658 movem (SP)+,{A0} */
    r->a[7] += 4u;
    r->d[1] = setw(r->d[1], (uint16_t)r->d[0]);                  /* 0x0B765C */
    r->d[0] = rd32(m, r->a[7]);                                  /* 0x0B765E movem (SP)+,{D0} */
    r->a[7] += 4u;

    uint16_t d2 = (uint16_t)(rd16(m, r->a[0]) + 1u);             /* 0x0B7662 */
    uint16_t d3 = (uint16_t)(rd16(m, r->a[0] + 2u) + 1u);        /* 0x0B7666 */

    {
        push_long(m, r, d3);                                     /* 0x0B766C movem {D3 D2} */
        push_long(m, r, d2);
        push_word(m, r, 7u);                                     /* 0x0B7670 */
        uint16_t idx = (uint16_t)((uint16_t)(d2 + d2) + (uint16_t)(d3 << 7));
        uint32_t dst = 0x0061C000u + (uint32_t)(int32_t)(int16_t)idx; /* 0x0B7674..0x0B767C */
        push_long(m, r, dst);                                    /* 0x0B7682 */
        push_long(m, r, 0x000B770Cu);                            /* 0x0B7684 */
        Text_DrawString(m, 0x000B770Cu, dst, 7u);                /* 0x0B768A */
        r->a[7] += 10u;                                          /* 0x0B7690 */
        r->d[2] = rd32(m, r->a[7]);                              /* 0x0B7694 movem (SP)+,{D2 D3} */
        r->a[7] += 4u;
        r->d[3] = rd32(m, r->a[7]);
        r->a[7] += 4u;
    }

    {
        uint32_t d0 = (uint32_t)((int32_t)(int16_t)(uint16_t)r->d[1] * 5); /* 0x0B7698..0x0B769A */
        d0 = setw(d0, (uint16_t)((uint16_t)d0 + 1u));            /* 0x0B769E */
        d2 = (uint16_t)(d2 + (uint16_t)d0);                      /* 0x0B76A0 */
    }

    {
        push_long(m, r, d3);                                     /* 0x0B76A2 movem {D3 D2} */
        push_long(m, r, d2);
        push_word(m, r, 0x0Au);                                  /* 0x0B76A6 */
        uint16_t idx = (uint16_t)((uint16_t)(d2 + d2) + (uint16_t)(d3 << 7));
        uint32_t dst = 0x0061C000u + (uint32_t)(int32_t)(int16_t)idx; /* 0x0B76AA..0x0B76B2 */
        push_long(m, r, dst);                                    /* 0x0B76B8 */
        push_long(m, r, 0x000B7500u);                            /* 0x0B76BA */
        r->d[0] = Text_DrawString(m, 0x000B7500u, dst, 0x0Au);   /* 0x0B76C0 */
        r->a[7] += 10u;                                          /* 0x0B76C6 */
        r->d[2] = rd32(m, r->a[7]);                              /* 0x0B76CA movem (SP)+,{D2 D3} */
        r->a[7] += 4u;
        r->d[3] = rd32(m, r->a[7]);
        r->a[7] += 4u;
    }

    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x20u) == 0u) {               /* 0x0B76CE btst #5 */
        r->d[1] = setw(r->d[1], 0xFFFFu);                        /* 0x0B76D8 move.w #-1 */
    } else if ((rd8(m, F3_A5 - 0x5DDBu) & 0x10u) != 0u) {        /* 0x0B76E0 btst #4 */
        return 1;                                                /* 0x0B76E6 -> TRAP #5 */
    }

    push_long(m, r, r->d[1]);                                    /* 0x0B76EA movem {D1} */
    push_long(m, r, r->a[0]);                                    /* 0x0B76EE */
    push_word(m, r, 3u);                                         /* 0x0B76F0 */
    push_word(m, r, 0x000Cu);                                    /* 0x0B76F4 */
    push_long(m, r, 0x000B76FCu);
    sub_0B73E2(m, 0x000Cu, 3u, r->a[0]);                         /* 0x0B76F8 */
    r->a[7] += 8u;                                               /* 0x0B76FC lea 8(SP) */
    r->d[1] = rd32(m, r->a[7]);                                  /* 0x0B7700 movem (SP)+,{D1} */
    r->a[7] += 4u;
    r->d[0] = setw(r->d[0], (uint16_t)r->d[1]);                  /* 0x0B7704 move.w D1w,D0w */
    r->d[1] = rd32(m, r->a[7]);                                  /* 0x0B7706 movem (SP)+,{D1 D2 D3 A0} */
    r->a[7] += 4u;
    r->d[2] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    r->d[3] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    r->a[0] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return 0;                                                    /* 0x0B770A rts */
}
