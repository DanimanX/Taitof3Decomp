/* ROM 0x0B7B52: (capture fnt_: injected call up to its first yield, F3_INJECT_ABORT): the sound/option editor task body.
 *
 * Pre-yield part: frame a 0x12 x 9 box (attribute 6) at the record
 * A5-0xC18 points to (0x4073E8), draw the hex legend and its two attribute
 * rows through 0x0B7A54 at (x+1, y+2), then yield at the TRAP #5 at 0x0B7B7E.
 * Task0B7B52_Frame below runs one pass of the loop the ROM resumes with at
 * 0x0B7B80; the loop re-enters the pre-yield code at 0x0B7B56.
 *
 * One pass: clear and repaint the cursor cell of the 0x20 x ... text grid, move
 * the column cursor A5-0xC3C with pad bits 3/2 (wrapping 0..0x1F) and the cell
 * cursor A5-0xC3E by +/-2 through 0x19FFF0 (bounds 0..0x202, buttons from the
 * byte A5-0x5DDC shifted left by 2), mask it to 9 bits, print "CODE:" 0x0B7D3E,
 * the two-digit hex cell number (0x5690), then " (SCR)" 0x0B7D44 or " (OBJ)"
 * 0x0B7D4B depending on the high byte of the cell number, and copy the 0x20
 * longs of the selected 0x40-byte group into the visible ring at 0x440500
 * (0x0B7B28).
 *
 * Then, with pad bit 5 of 0x402225:
 *   - bit 4 set: the loop goes straight back to the 0x0B7B7E yield (return 1);
 *   - bit 4 clear: the record at A5-0xC18 is advanced by one long, its x is
 *     moved by (nibble - 1) and its y by 3*bit4 + 4 of the cell number, the
 *     nested editor 0x0B7D74 runs and the loop restarts at 0x0B7B56.
 * With bit 5 clear the pass ends at 0x0B7FE0-style exit: blank the 0x11 x 6 box
 * and pop the six saved registers, RTS.
 *
 * The nested editor yields internally (0x0B7DD0); Fn_0B7D74 covers its setup
 * and Fn_0B7D74_Tick one loop pass, so this frame returns 1 at that yield too
 * and the ROM's resume address is then 0x0B7DD2. The saved-register frames of
 * 0x0B7B52 and 0x0B7D74 stay on the task stack across those yields, exactly as
 * in the ROM. */
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


/* 0x0B7B56..0x0B7B7E: the pre-yield code, also the loop's restart point. */
static void body_0B7B56(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a0 = rd32(m, F3_A5 - 0xC18u);    /* 0x0B7B56 movea.l -0xC18(A5),A0 */

    /* 0x0B7B5A..0x0B7B6C: w = 0x12, h = 9, attribute 6. */
    box_frame_0B7332(m, 0x12u, 9u, 6u, a0);

    /* 0x0B7B70..0x0B7B7A: legend at (x+1, y+2). */
    r->d[1] = (r->d[1] & 0xFFFF0000u) |
              (uint16_t)(rd16(m, a0) + 1u);          /* 0x0B7B70 move.w (A0),D1 ; addq.w #1 */
    r->d[2] = (r->d[2] & 0xFFFF0000u) |
              (uint16_t)(rd16(m, a0 + 2u) + 2u);     /* 0x0B7B74 move.w (2,A0),D2 ; addq.w #2 */
    r->a[0] = a0;
    Fn_0B7A54(m, r);                                 /* 0x0B7B7A bsr 0x0B7A54 */
}

int Fn_0B7B52(const f3mem_t *m, f3regs_t *r)
{
    /* 0x0B7B52 movem.l d0-d2/a0-a2,-(SP): memory order D0 D1 D2 A0 A1 A2. */
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);

    body_0B7B56(m, r);
    return 0;                                        /* 0x0B7B7E trap #5 */
}

int Task0B7B52_Frame(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a0 = rd32(m, F3_A5 - 0xC18u);
    uint32_t a1;

    /* 0x0B7B80..0x0B7B9C: clear the cursor cell of the edit grid. */
    a1 = 0x0061C000u +
         (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)(rd16(m, a0) + 1u) << 1) +
         (uint32_t)(int32_t)(int16_t)
             ((uint16_t)((uint16_t)(rd16(m, a0 + 2u) + 3u) << 7));
    r->a[1] = a1;
    r->d[1] = (r->d[1] & 0xFFFF0000u);                /* 0x0B7B98 move.w #0,D1 */
    Fn_0B7D52(m, r);                                  /* 0x0B7B9C bsr 0x0B7D52 */

    /* 0x0B7BA0..0x0B7BBA: pad bits 2 and 3 move the column cursor. */
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x04u))
        wr16(m, F3_A5 - 0xC3Cu, (uint16_t)(rd16(m, F3_A5 - 0xC3Cu) - 1u));
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x08u))
        wr16(m, F3_A5 - 0xC3Cu, (uint16_t)(rd16(m, F3_A5 - 0xC3Cu) + 1u));

    /* 0x0B7BBC..0x0B7BEC: wrap -1 -> 0x1F and 0x20 -> 0 (the addi/subi #0 are
     * the ROM's own no-ops). */
    if ((int16_t)rd16(m, F3_A5 - 0xC3Cu) < 0) {
        uint16_t v = (uint16_t)(rd16(m, F3_A5 - 0xC3Cu) + 0x1Fu);   /* 0x0B7BC6 */
        v = (uint16_t)(v + 1u);                                     /* 0x0B7BD2 */
        wr16(m, F3_A5 - 0xC3Cu, v);
    }
    if ((int16_t)rd16(m, F3_A5 - 0xC3Cu) > 0x1F) {
        uint16_t v = (uint16_t)(rd16(m, F3_A5 - 0xC3Cu) - 0x1Fu);   /* 0x0B7BE0 */
        v = (uint16_t)(v - 1u);                                     /* 0x0B7BEC */
        wr16(m, F3_A5 - 0xC3Cu, v);
    }

    /* 0x0B7BF0..0x0B7BFC: mark the column (0xA << 8 << 1 = 0x1400, | 6). */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0x0A06u;
    Fn_0B7D52(m, r);

    /* 0x0B7C00..0x0B7C42: cell cursor +/- 2 within 0..0x202, then -2 and mask. */
    {
        uint32_t saved0;
        uint16_t buttons = (uint16_t)(uint8_t)((uint8_t)rd8(m, F3_A5 - 0x5DDCu) << 2);
        saved0 = r->d[0];
        wr16(m, F3_A5 - 0xC3Eu, (uint16_t)(rd16(m, F3_A5 - 0xC3Eu) + 2u)); /* 0x0B7C06 */
        push_word(m, r, 2u);                        /* 0x0B7C12 step */
        push_word(m, r, 0x0202u);                   /* 0x0B7C16 upper */
        push_word(m, r, 0u);                        /* 0x0B7C1A lower */
        push_word(m, r, rd16(m, F3_A5 - 0xC3Eu));   /* 0x0B7C1E value */
        push_word(m, r, (uint16_t)(int16_t)(int8_t)(uint8_t)buttons); /* 0x0B7C22..0x0B7C26 */
        push_long(m, r, 0x000B7C2Eu);
        Fn_19FFF0(m, r);                            /* 0x0B7C28 jsr 0x19FFF0 */
        r->a[7] += 0x0Au;                           /* 0x0B7C2E lea (0xa,SP),SP */
        wr16(m, F3_A5 - 0xC3Eu, (uint16_t)r->d[0]);  /* 0x0B7C36 move.w D0,-0xC3E(A5) */
        r->d[0] = saved0;
    }
    wr16(m, F3_A5 - 0xC3Eu, (uint16_t)(rd16(m, F3_A5 - 0xC3Eu) - 2u)); /* 0x0B7C3E */
    wr16(m, F3_A5 - 0xC3Eu, (uint16_t)(rd16(m, F3_A5 - 0xC3Eu) & 0x01FFu)); /* 0x0B7C42 */

    /* 0x0B7C48..0x0B7C7A: "CODE:" at (x+2, y+1), attribute 0xA. */
    {
        uint32_t dst = 0x0061C000u +
            (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)(rd16(m, a0) + 2u) << 1) +
            (uint32_t)(int32_t)(int16_t)
                ((uint16_t)((uint16_t)(rd16(m, a0 + 2u) + 1u) << 7));
        r->d[0] = Text_DrawString(m, 0x000B7D3Eu, dst, 0x0Au);
    }

    /* 0x0B7C7E..0x0B7C94: the cell number, two hex digits, attribute 7. */
    {
        uint16_t cellno = rd16(m, F3_A5 - 0xC3Eu);
        r->d[3] = (r->d[3] & 0xFFFF0000u) | cellno;
        r->d[0] = Gfx_DrawHex(m, cellno, 2u, r->d[0], 7u);
    }

    /* 0x0B7C98..0x0B7CCC: " (SCR)" with attribute 5, or " (OBJ)" with 8. */
    if (((uint16_t)r->d[3] & 0xFF00u) == 0u)
        r->d[0] = Text_DrawString(m, 0x000B7D44u, r->d[0], 5u);
    else
        r->d[0] = Text_DrawString(m, 0x000B7D4Bu, r->d[0], 8u);

    /* 0x0B7CD0..0x0B7CDE: copy the group into the visible ring. */
    r->a[1] = 0x00440000u +
        (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, F3_A5 - 0xC3Eu) << 6);
    Fn_0B7B28(m, r);

    /* 0x0B7CE2 btst.b #5: without it the editor leaves. */
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x20u))
        goto leave;

    /* 0x0B7CEC btst.b #4: with it the loop restarts at the yield. */
    if (rd8(m, F3_A5 - 0x5DDBu) & 0x10u)
        return 1;                                /* 0x0B7B7E trap #5 */

    /* 0x0B7CF6..0x0B7D22: duplicate the record, move it, run the editor. */
    {
        uint32_t rec = a0;
        uint16_t sel = rd16(m, F3_A5 - 0xC3Cu);
        uint16_t dx;
        wr32(m, rec + 4u, rd32(m, rec));         /* 0x0B7CF6 move.l (A0),4(A0) */
        r->a[0] = rec + 4u;                      /* 0x0B7CFA addq.l #4,A0 */
        rec += 4u;
        dx = (uint16_t)(uint16_t)((sel >> 4) & 1u);            /* 0x0B7D00 bfextu D0{27:1} */
        wr16(m, rec, (uint16_t)(rd16(m, rec) + (uint16_t)((sel & 0x0Fu) - 1u))); /* 0x0B7D04..0x0B7D0A */
        wr16(m, rec + 2u, (uint16_t)(rd16(m, rec + 2u) + (uint16_t)(dx * 3u + 4u))); /* 0x0B7D0C..0x0B7D12 */
        wr32(m, F3_A5 - 0xC18u, rd32(m, F3_A5 - 0xC18u) + 4u); /* 0x0B7D16 */
        /* 0x0B7D1A bsr 0x0B7D74: the editor's setup pass always ends at its
         * 0x0B7DD0 trap, so the task yields here; Fn_0B7D74_Tick covers the
         * passes the ROM resumes with at 0x0B7DD2. */
        (void)Fn_0B7D74(m, r);
        wr32(m, F3_A5 - 0xC18u, rd32(m, F3_A5 - 0xC18u) - 4u); /* 0x0B7D1E */
    }
    return 1;

leave:
    /* 0x0B7D26..0x0B7D3C: blank the 0x11 x 6 box, restore, RTS. */
    box_blank_0B73E2(m, 0x11u, 6u, a0);
    r->d[0] = rd32(m, r->a[7]);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->d[2] = rd32(m, r->a[7] + 8u);
    r->a[0] = rd32(m, r->a[7] + 12u);
    r->a[1] = rd32(m, r->a[7] + 16u);
    r->a[2] = rd32(m, r->a[7] + 20u);
    r->a[7] += 24u;                              /* movem.l (SP)+,d0-d2/a0-a2 */
    r->a[7] += 4u;                               /* 0x0B7D3C rts */
    return 0;
}
