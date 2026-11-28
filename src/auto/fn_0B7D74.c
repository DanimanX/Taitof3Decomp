/* ROM 0x0B7D74: UNVERIFIED (no capture): the sound/option editor.
 *
 * The routine frames a 8x6 box at the record A5-0xC18 points to and prints the
 * single control glyph 0x06 at (x+2, y), then yields at the TRAP #5 at
 * 0x0B7DD0. Fn_0B7D74 covers exactly that setup and hands the live A0 (the
 * record) and A1 (0x440000, the cell array) on in the register file;
 * Fn_0B7D74_Tick is one pass of the editor loop the ROM resumes with at
 * 0x0B7DD2 and returns 1 where the ROM yields, 0 when it leaves through the
 * 0x0B7FE0 exit (blank the 0x11x6 box and restore the saved registers).
 *
 * One pass: clear the cursor cell of the current row, move the row cursor
 * A5-0xC3A with pad bits 0/1 (clamped to 0..2), paint the cursor cell 0x1405,
 * pick the 4-byte cell at 0x440000 + (A5-0xC3E)*0x40 + (A5-0xC3C)*4 and rotate
 * byte (2 - row) into the low end with `ror.l` (the byte is at bit (2-row)*8),
 * edit it by +/- 8 through 0x19FFF0 (bounds 0..0xFF, buttons from the
 * sign-extended byte A5-0x5DDC), rotate it back and store the long, mirror it
 * into the 0x504-offset table, redraw the three volume bars of the cell with
 * 0x1A0086 (colours 4, 5 and 8, four slots each) and print the packed value as
 * four hex digits with 0x005690. The pass repeats while pad bits 5 and 4 of
 * 0x402225 are both set.
 *
 * The X-flag chain at 0x0B7EF2 (lsr.w #3 then two roxr.w) is modelled
 * explicitly: after the shift the extend flag is bit 2 of the byte, the high
 * bit of each roxr is that flag and the flag becomes the bit rotated out. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (0x005666) */
#include "../video/sprite.h"       /* Gfx_DrawHex (0x005690) */

/* ROM 0x0B7332: frame a `w` x `h` box at {x.w, y.w} (the rect long the last
 * argument points to) in the text layer at 0x61C000 with attribute `attr`
 * (the third argument). Every register is saved and restored by the ROM, so
 * only the text RAM changes. */
static void box_frame_0B7332(const f3mem_t *m, uint16_t w, uint16_t h, uint16_t attr, uint32_t rect)
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
static void box_blank_0B73E2(const f3mem_t *m, uint16_t w, uint16_t h, uint32_t rect)
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


/* 0x0B7EE6..0x0B7EFA and the two repeats: D3 = byte >> 3 (the bar value), D0
 * and D1 take the bit through the rotate-through-extend chain. */
static uint32_t fold_byte(uint32_t *d0, uint32_t *d1, uint8_t byte)
{
    unsigned x = (unsigned)(((unsigned)byte >> 2) & 1u);   /* lsr.w #3 extend */
    uint32_t d3 = (uint32_t)byte >> 3;                     /* 0x0B7EF2 lsr.w #3 */
    uint32_t lo = ((*d0 & 0xFFFFu) | d3) & 0xFFFFu;        /* 0x0B7EF4 or.w D3,D0 */

    unsigned nx = (unsigned)(lo & 1u);
    lo = ((uint32_t)x << 15 | (lo >> 1)) & 0xFFFFu;        /* 0x0B7EF6 roxr.w #1,D0 */
    x = nx;
    uint16_t h1 = (uint16_t)(*d1 & 0xFFFFu);
    nx = (unsigned)(h1 & 1u);
    h1 = (uint16_t)(((uint32_t)x << 15 | (h1 >> 1)) & 0xFFFFu); /* 0x0B7EF8 roxr.w #1,D1 */
    (void)nx;

    *d0 = (*d0 & 0xFFFF0000u) | lo;
    *d1 = (*d1 & 0xFFFF0000u) | h1;
    *d0 = (*d0 >> 4) | (*d0 << 28);                        /* 0x0B7EFA ror.l #4,D0 */
    return d3;
}

/* The 0x1A0086 call of 0x0B7F00/0x0B7F38/0x0B7F70: push dst, value, slots,
 * colour (in that argument order) and the return PC, then drop the four. */
static void bar_1A0086(const f3mem_t *m, f3regs_t *r, uint32_t dst, uint32_t value,
                       uint32_t slots, uint16_t colour)
{
    push_word(m, r, colour);
    push_long(m, r, slots);
    push_long(m, r, value);
    push_long(m, r, dst);
    push_long(m, r, 0x000B7F0Eu);
    Fn_1A0086(m, r);
    r->a[7] += 0x0Eu;
}

int Fn_0B7D74(const f3mem_t *m, f3regs_t *r)
{
    /* 0x0B7D74 movem.l d0-d3/a0-a2,-(SP): memory order D0 D1 D2 D3 A0 A1 A2. */
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);

    const uint32_t a0 = rd32(m, F3_A5 - 0xC18u);    /* 0x0B7D78 */
    const uint32_t a1 = 0x00440000u;                /* 0x0B7D7C */

    /* 0x0B7D82..0x0B7D94: w = 8, h = 6, attribute 6. */
    box_frame_0B7332(m, 8u, 6u, 6u, a0);

    /* 0x0B7D98..0x0B7DCC: the glyph at 0x0B7502 ("\\x06") at (x+2, y+0), attr 0xA. */
    {
        uint32_t dx = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, a0) + 2u);
        uint32_t dy = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, a0 + 2u) << 7);
        (void)Text_DrawString(m, 0x000B7502u, 0x0061C000u + dx + dx + dy, 0x0Au);
    }

    r->a[0] = a0;                                   /* live across the yield */
    r->a[1] = a1;
    return 1;                                       /* 0x0B7DD0 trap #5 */
}

int Fn_0B7D74_Tick(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a0 = r->a[0];
    const uint32_t a1 = r->a[1];
    uint32_t a2, d0, d1, d2, cell;
    uint32_t d3;
    uint32_t saved0, saved1, saved2, saveda0, saveda1;
    unsigned rot;

    /* 0x0B7DD2..0x0B7DE8: text cursor of the cell grid. */
    {
        uint32_t dx = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, a0) + 1u);
        uint32_t dy = (uint32_t)(int32_t)(int16_t)(uint16_t)
            ((uint16_t)((uint16_t)(rd16(m, a0 + 2u) + 1u) << 7));
        a2 = 0x0061C000u + dx + dx + dy;
    }

    /* 0x0B7DEA: clear the cell of the row the cursor sat on. */
    wr16(m, a2 + (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, F3_A5 - 0xC3Au) << 7), 0u);

    /* 0x0B7DF6..0x0B7E0E: pad bits 0 and 1 move the row cursor. */
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x01u))
        wr16(m, F3_A5 - 0xC3Au, (uint16_t)(rd16(m, F3_A5 - 0xC3Au) - 1u));
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x02u))
        wr16(m, F3_A5 - 0xC3Au, (uint16_t)(rd16(m, F3_A5 - 0xC3Au) + 1u));

    /* 0x0B7E12..0x0B7E2C: clamp to 0..2 (the ROM stores the bound directly). */
    if ((int16_t)rd16(m, F3_A5 - 0xC3Au) < 0)
        wr16(m, F3_A5 - 0xC3Au, 0u);
    if ((int16_t)rd16(m, F3_A5 - 0xC3Au) > 2)
        wr16(m, F3_A5 - 0xC3Au, 2u);

    /* 0x0B7E32..0x0B7E44: paint the cursor cell with 0x1405. */
    wr16(m, a2 + (uint32_t)(int32_t)(int16_t)(uint16_t)
             (rd16(m, F3_A5 - 0xC3Au) << 7), 0x1405u);

    /* 0x0B7E48..0x0B7E6E: the cell long, byte (2-row) rotated to the low end. */
    d0 = (uint32_t)(uint16_t)(rd16(m, F3_A5 - 0xC3Eu) << 6);   /* 0x0B7E48..0x0B7E4C */
    d1 = (uint32_t)(uint16_t)(rd16(m, F3_A5 - 0xC3Cu) << 2);   /* 0x0B7E4E..0x0B7E52 */
    d1 = (uint32_t)(uint16_t)(d0 + d1);                         /* 0x0B7E54 */
    cell = a1 + (uint32_t)(int32_t)(int16_t)(uint16_t)d1;       /* 0x0B7E60 */
    rot = (unsigned)((uint16_t)(2u - rd16(m, F3_A5 - 0xC3Au)) << 3) & 63u; /* 0x0B7E56..0x0B7E5E */
    d2 = rd32(m, cell);
    if (rot != 0u)
        d2 = (d2 >> rot) | (d2 << (32u - rot));                 /* 0x0B7E64 ror.l D1,D2 */
    d3 = d2 & 0xFFu;                                            /* 0x0B7E6E */
    d2 &= 0xFFFFFF00u;                                          /* 0x0B7E68 */

    /* 0x0B7E74..0x0B7EB2: 0x19FFF0 edits the byte by 8 within 0..0xFF. */
    saved0 = r->d[0]; saved1 = r->d[1]; saved2 = r->d[2];
    saveda0 = r->a[0]; saveda1 = r->a[1];
    {
        uint16_t buttons = (uint16_t)(int16_t)(int8_t)rd8(m, F3_A5 - 0x5DDCu);
        push_word(m, r, 8u);                                    /* 0x0B7E80 step */
        push_word(m, r, 0xFFu);                                 /* 0x0B7E84 upper */
        push_word(m, r, 0u);                                    /* 0x0B7E88 lower */
        push_word(m, r, (uint16_t)d3);                          /* 0x0B7E8C value */
        push_word(m, r, buttons);                               /* 0x0B7E94 buttons */
        push_long(m, r, 0x000B7E9Cu);
        Fn_19FFF0(m, r);                                        /* 0x0B7E96 jsr 0x19FFF0 */
        r->a[7] += 0x0Au;                                       /* 0x0B7E9C lea (0xa,SP),SP */
    }
    d3 = r->d[0] & 0xFFFFu;                                     /* 0x0B7EA4 move.w D0,D3 */
    r->d[0] = saved0; r->d[1] = saved1; r->d[2] = saved2;
    r->a[0] = saveda0; r->a[1] = saveda1;

    d2 |= d3;                                                   /* 0x0B7EAE or.l D3,D2 */
    if (rot != 0u)
        d2 = (d2 << rot) | (d2 >> (32u - rot));                 /* 0x0B7EB0 rol.l D1,D2 */
    wr32(m, cell, d2);                                          /* 0x0B7EB2 */

    /* 0x0B7EB6..0x0B7EC8: mirror the long into the 0x504-offset table. */
    d0 = (uint32_t)rd16(m, F3_A5 - 0xC3Cu);
    d1 = d0;
    d0 &= 0x18u;                                                /* 0x0B7EBC andi.w #0x18 */
    d1 &= 0x07u;                                                /* 0x0B7EC0 andi.w #7 */
    d0 = (uint32_t)(uint16_t)(d0 + d0);                         /* 0x0B7EC4 add.w D0,D0 */
    d0 = (uint32_t)(uint16_t)(d0 + d1);                         /* 0x0B7EC6 add.w D1,D0 */
    wr32(m, a1 + 0x504u + 4u * (uint32_t)(int32_t)(int16_t)(uint16_t)d0, d2);

    /* 0x0B7ECE..0x0B7EE4: the bar cursor is two rows below / one right. */
    {
        uint32_t bx = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, a0) + 2u);
        uint32_t by = (uint32_t)(int32_t)(int16_t)(uint16_t)
            ((uint16_t)((uint16_t)(rd16(m, a0 + 2u) + 3u) << 7));
        a2 = 0x0061C000u + bx + bx + by;
    }

    /* 0x0B7EE6..0x0B7F88: three 4-slot volume bars, colours 4, 5 and 8. */
    d0 = 0u;                                                    /* 0x0B7EE6 moveq #0,D0 */
    d1 = 0u;                                                    /* 0x0B7EE8 move.l D0,D1 */
    {
        uint32_t value = fold_byte(&d0, &d1, (uint8_t)(d2 & 0xFFu));
        bar_1A0086(m, r, a2, value, 4u, 4u);                    /* 0x0B7F00..0x0B7F18 */
    }
    a2 -= 0x80u;                                                /* 0x0B7F1C */
    d2 >>= 8;                                                   /* 0x0B7F20 lsr.l #8,D2 */
    {
        uint32_t value = fold_byte(&d0, &d1, (uint8_t)(d2 & 0xFFu));
        bar_1A0086(m, r, a2, value, 4u, 5u);                    /* 0x0B7F38..0x0B7F50 */
    }
    a2 -= 0x80u;                                                /* 0x0B7F54 */
    d2 >>= 8;                                                   /* 0x0B7F58 */
    {
        uint32_t value = fold_byte(&d0, &d1, (uint8_t)(d2 & 0xFFu));
        bar_1A0086(m, r, a2, value, 4u, 8u);                    /* 0x0B7F70..0x0B7F88 */
    }

    /* 0x0B7F8C..0x0B7F90: rol.w #4,D1 ; swap D0 ; or.w D1,D0. */
    {
        uint32_t lo1 = (d1 & 0xFFFFu);
        lo1 = ((lo1 << 4) | (lo1 >> 12)) & 0xFFFFu;
        d0 = (d0 << 16) | (d0 >> 16);                            /* swap D0 */
        d0 = (d0 & 0xFFFF0000u) | ((d0 & 0xFFFFu) | lo1);
    }

    /* 0x0B7F92..0x0B7FC4: the packed value as four hex digits, attribute 7. */
    {
        uint32_t hx = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, a0) + 2u);
        uint32_t hy = (uint32_t)(int32_t)(int16_t)(uint16_t)
            ((uint16_t)((uint16_t)(rd16(m, a0 + 2u) + 4u) << 7));
        (void)Gfx_DrawHex(m, d0, 4u, 0x0061C000u + hx + hx + hy, 7u);
    }

    /* 0x0B7FC8..0x0B7FDC: keep looping while pad bits 5 and 4 are both set. */
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x20u))
        goto leave;
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x10u))
        goto leave;
    return 1;                                                   /* 0x0B7DD0 trap #5 */

leave:
    /* 0x0B7FE0..0x0B7FF6: blank the 0x11 x 6 box, restore D0-D3/A0-A2, RTS. */
    box_blank_0B73E2(m, 0x11u, 6u, a0);
    r->d[0] = rd32(m, r->a[7]);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->d[2] = rd32(m, r->a[7] + 8u);
    r->d[3] = rd32(m, r->a[7] + 12u);
    r->a[0] = rd32(m, r->a[7] + 16u);
    r->a[1] = rd32(m, r->a[7] + 20u);
    r->a[2] = rd32(m, r->a[7] + 24u);
    r->a[7] += 28u;                                             /* movem (SP)+ */
    r->a[7] += 4u;                                              /* rts */
    return 0;
}
