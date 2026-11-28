#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"

/* ROM 0x1781F6: UNVERIFIED (no usable capture): stage-select panel -- a
 * kernel task body (TRAP #5 at 0x178216 is its yield).
 *
 * Fn_1781F6 is the setup before the first yield: frame a 10x3 box (attribute
 * 4) around the rect at A5+0x4AD4 with 0x0B7332. Task1781F6_Frame is one pass
 * after the yield:
 *
 *   178218  draw the 3-byte label of the selection A5+0x32DC from the table at
 *           ROM 0x17892A at 0x61C000 + 2*10 + (3<<7), attribute 7
 *   17825A  A5-0x5DDB bit 4 set keeps the box open (navigate), clear commits:
 *           copy 0x1000 bytes from the ROM pointer table 0x13F9D4[selection]
 *           to A5+0x42B4 and store the per-selection word of ROM 0x17888A at
 *           A5+0x32DE, then leave
 *   1782A4  navigation: A5-0x5DDB bit 2 / bit 3 step the selection down / up
 *           (A5-0x5DDC is the held state, the byte at A5-0xC90 the repeat
 *           delay, the upper bound is 0x1B); bit 6 of A5-0x5DDB yields again
 *   178308  unframe with 0x0B73E2 and restore D0-D7/A0-A4
 *
 * The 30 captured pairs for this address all span 4-5 frames (the flow yields
 * to VBlank inside the pair), so tests/test_fn.c skips every one of them as
 * VBlank-interrupted and the capture rejects the routine; it is translated
 * from the bytes and checked only by compilation. */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0B7332: frame a `w` x `h` box at {x.w, y.w} (the rect the last
 * argument points to) in the text layer 0x61C000 with attribute `attr`. All
 * registers are saved and restored by the ROM. */
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
 * to. All registers are saved and restored by the ROM. */
static void sub_0B73E2(const f3mem_t *m, uint16_t w, uint16_t h, uint32_t rect)
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

/* ROM 0x0B73E2 tail shared by both exits: unframe the box at A4, then restore
 * D0-D7/A0-A4 from the 13-register save at 0x1781F6. Returns 0. */
static int exit_tail(const f3mem_t *m, f3regs_t *r, uint32_t a4)
{
    push_long(m, r, a4);                                         /* 0x178308 */
    push_word(m, r, 3u);                                         /* 0x17830A */
    push_word(m, r, 0x000Au);                                    /* 0x17830E */
    push_long(m, r, 0x00178318u);
    sub_0B73E2(m, 0x000Au, 3u, a4);                              /* 0x178312 */
    r->a[7] += 8u;                                               /* 0x178318 lea 8(SP) */
    for (int k = 0; k < 8; k++) {                                /* 0x17831C movem (SP)+ */
        r->d[k] = rd32(m, r->a[7]);
        r->a[7] += 4u;
    }
    for (int k = 0; k < 5; k++) {
        r->a[k] = rd32(m, r->a[7]);
        r->a[7] += 4u;
    }
    return 0;                                                    /* 0x178320 rts */
}

/* ROM 0x1781F6: setup before the first TRAP #5 (0x178216). */
int Fn_1781F6(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[4]);                                    /* 0x1781F6 movem 13 regs */
    push_long(m, r, r->a[3]);
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    for (int k = 7; k >= 0; k--)
        push_long(m, r, r->d[k]);
    uint32_t a4 = rd32(m, F3_A5 + 0x4AD4u);                      /* 0x1781FA */
    push_long(m, r, a4);                                         /* 0x1781FE */
    push_word(m, r, 4u);                                         /* 0x178200 */
    push_word(m, r, 3u);                                         /* 0x178204 */
    push_word(m, r, 0x000Au);                                    /* 0x178208 */
    push_long(m, r, 0x00178212u);
    sub_0B7332(m, 0x000Au, 3u, 4u, a4);                          /* 0x17820C */
    r->a[7] += 10u;                                              /* 0x178212 lea 0xa(SP) */
    return 0;                                                    /* 0x178216 TRAP #5 */
}

/* ROM 0x178218: one pass of the loop after the yield. Returns 1 when the pass
 * yields again (0x178304 -> TRAP #5) and 0 when it left through 0x178308. */
int Task1781F6_Frame(const f3mem_t *m, f3regs_t *r)
{
    {
        uint32_t a0 = 0x0017892Au;                               /* 0x17821C */
        uint32_t d0 = (uint32_t)((int32_t)(int16_t)rd16(m, F3_A5 + 0x32DCu) * 3); /* 0x178218..0x178222 */
        a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)d0;          /* 0x178226 */
        d0 = setw(d0, 0x0005u);                                  /* 0x17822A move.w #5,D0w */
        uint32_t d1 = setw(r->d[1], 0x0003u);                    /* 0x17822E move.w #3,D1w */
        push_long(m, r, d1);                                     /* 0x178232 movem {D1 D0}: D1 */
        push_long(m, r, d0);                                     /* (D0 at the lower address) */
        push_word(m, r, 7u);                                     /* 0x178236 */
        uint32_t dst = 0x0061C000u
                     + (uint32_t)(int32_t)(int16_t)(uint16_t)
                       ((uint16_t)((uint16_t)(5u + 5u) + (uint16_t)(d1 << 7)));
        /* 0x17823A..0x178242: 2*5 + (3<<7) */
        push_long(m, r, dst);                                    /* 0x178248 */
        push_long(m, r, a0);                                     /* 0x17824A */
        Text_DrawString(m, a0, dst, 7u);                         /* 0x17824C */
        r->a[7] += 10u;                                          /* 0x178252 */
        r->d[0] = rd32(m, r->a[7]);                              /* 0x178256 movem (SP)+,{D0 D1} */
        r->a[7] += 4u;
        r->d[1] = rd32(m, r->a[7]);
        r->a[7] += 4u;
    }

    uint32_t a4 = r->a[4];

    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x10u) == 0u) {               /* 0x17825A btst #4 */
        /* 0x178264: the selection is confirmed -- load its map into the HUD
         * area and store the per-stage word. */
        push_long(m, r, r->a[1]);                                /* 0x178264 movem {A1 A0}: A0 */
        push_long(m, r, r->a[0]);                                /* at the higher address */
        uint32_t a0 = 0x0013F9D4u;                               /* 0x178268 */
        uint16_t d7w = (uint16_t)(rd16(m, F3_A5 + 0x32DCu) * 4u); /* 0x17826E..0x178274 */
        a0 += (uint32_t)(int32_t)(int16_t)d7w;                   /* 0x178276 */
        a0 = rd32(m, a0);                                        /* 0x17827A */
        uint32_t a1 = F3_A5 + 0x42B4u;                           /* 0x17827C */
        uint16_t d7 = 0x00FFu;                                   /* 0x178280 */
        for (;;) {                                               /* 0x178284 */
            wr32(m, a1, rd32(m, a0)); a0 += 4u; a1 += 4u;
            wr32(m, a1, rd32(m, a0)); a0 += 4u; a1 += 4u;
            wr32(m, a1, rd32(m, a0)); a0 += 4u; a1 += 4u;
            wr32(m, a1, rd32(m, a0)); a0 += 4u; a1 += 4u;
            if (d7-- == 0u)                                      /* 0x178288 dbf */
                break;
        }
        {
            uint32_t tbl = 0x0017888Au;                          /* 0x17828C */
            uint16_t d0 = rd16(m, F3_A5 + 0x32DCu);              /* 0x178292 */
            wr16(m, F3_A5 + 0x32DEu,
                 rd16(m, tbl + (uint32_t)(int32_t)(int16_t)
                               (uint16_t)(d0 + d0)));            /* 0x178296 */
        }
        r->a[0] = rd32(m, r->a[7]);                              /* 0x17829C movem (SP)+,{A0 A1} */
        r->a[7] += 4u;
        r->a[1] = rd32(m, r->a[7]);
        r->a[7] += 4u;
        return exit_tail(m, r, a4);                              /* 0x1782A0 bra 0x178308 */
    }

    /* 0x1782A4: navigation */
    {
        uint16_t d0 = rd16(m, F3_A5 + 0x32DCu);                  /* 0x1782A4 */
        int down = 0;
        if ((rd8(m, F3_A5 - 0x5DDBu) & 0x04u) == 0u) {           /* 0x1782A8 */
            down = 1;
        } else if ((rd8(m, F3_A5 - 0x5DDCu) & 0x04u) == 0u        /* 0x1782B2 */
                   && rd8(m, F3_A5 - 0x0C90u) > 0x0Cu) {         /* 0x1782BC */
            down = 1;
        }
        if (down && d0 != 0u)                                    /* 0x1782C6 */
            d0 = (uint16_t)(d0 - 1u);

        int up = 0;
        if ((rd8(m, F3_A5 - 0x5DDBu) & 0x08u) == 0u) {           /* 0x1782CE */
            up = 1;
        } else if ((rd8(m, F3_A5 - 0x5DDCu) & 0x08u) == 0u        /* 0x1782D8 */
                   && rd8(m, F3_A5 - 0x0C90u) > 0x0Cu) {         /* 0x1782E2 */
            up = 1;
        }
        if (up && d0 != 0x001Bu)                                 /* 0x1782EC */
            d0 = (uint16_t)(d0 + 1u);

        wr16(m, F3_A5 + 0x32DCu, d0);                            /* 0x1782F6 */
    }

    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x40u) != 0u)                 /* 0x1782FA btst #6 */
        return 1;                                                /* 0x178304 -> TRAP #5 */
    return exit_tail(m, r, a4);                                  /* 0x178308 */
}
