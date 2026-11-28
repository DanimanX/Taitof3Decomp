/* ROM 0x091984: sprite/colour debug overlay tick (A0 = object).
 * 0x091C50 first applies the player nudges (A5-0x5DD8 bits 0..3 add/sub 0x20000
 * to the two 16.16 screen offsets A5-0xC5E / A5-0xC62). Then, depending on the
 * debug flag byte A5-0xC4D:
 *  - bit 0 clear: the word at A5-0xC68 picks record 0x41A260 + 0x0C*(n-1) when
 *    it is in [1, 0x0D], and 0x0919F4 draws it.
 *  - bit 0 set: walk the 0x0C-byte records from A5-0xC58 (frames counted in
 *    A5-0xC54) while their first word runs, drawing each with 0x0919F4.
 * The common tail 0x09194A frees A5-0xC58 and scans 0x41A260..0x41A307 for the
 * first record whose first word is positive (the table ends at 0x41A308), then
 * re-arms the tick at 0x091984.
 * 0x0919F4 places the record's sprite (+0x14/+0x18 = A5-0xC62 / A5-0xC5E plus
 * the record's own offsets), draws it (0x001282), then prints the sprite
 * pointer and record words beside "* (NORMAL)" / "(BOSS n)" / "( DEMO )" at
 * 0x091C32.. and copies the palette cell through 0x0B7B28. */
#include "fn_auto.h"
#include "batch_obj_regs.h"          /* batch_draw_sprite (0x001282) */
#include "../enemy/obj_1194C2.h"     /* Text_DrawString (0x005666) */
#include "../video/sprite.h"         /* Gfx_DrawHex / Gfx_DrawDecimal */

#define REC_BASE   0x0041A260u
#define REC_END    0x0041A308u
#define TMP_SPR    0x0041A308u   /* the one-slot scratch sprite record */
#define TEXT_BASE  0x0061C000u   /* text RAM */
#define PAL_CELLS  0x00444000u
#define PAL_OUT    0x00444D84u

#define OFF_X      (F3_A5 - 0x0C62u)  /* 0x40739E screen X, 16.16 */
#define OFF_Y      (F3_A5 - 0x0C5Eu)  /* 0x4073A2 screen Y, 16.16 */
#define DBG_FLAGS  (F3_A5 - 0x0C4Du)  /* 0x4073B3 debug overlay flags */
#define MODE_BYTE  (F3_A5 - 0x0C4Eu)  /* 0x4073B2 stage mode */
#define DITHER     (F3_A5 - 0x0C52u)  /* 0x4073AE blinking colour */
#define CURSOR     (F3_A5 - 0x0C58u)  /* 0x4073A8 current record */
#define COUNTER    (F3_A5 - 0x0C54u)  /* 0x4073AC frame counter */
#define INDEX_W    (F3_A5 - 0x0C68u)  /* 0x407398 selected record index */
#define PAD_FLAGS  (F3_A5 - 0x5DD8u)  /* 0x402228 player input bits */

#define STR_STAR   0x00091C32u   /* "*" */
#define STR_NORMAL 0x00091C34u   /* "(NORMAL)" */
#define STR_BOSS   0x00091C3Du   /* "(BOSS " */
#define STR_CLOSE  0x00091C44u   /* ")" */
#define STR_DEMO   0x00091C46u   /* "( DEMO )" */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x091C50: nudge the two 16.16 offsets by the player's input bits. */
static void sub_091C50(const f3mem_t *m)
{
    uint8_t f = rd8(m, PAD_FLAGS);
    if ((f & 0x01u) == 0u)                              /* 0x091C50 btst #0 */
        wr32(m, OFF_Y, rd32(m, OFF_Y) - 0x00020000u);
    if ((f & 0x02u) == 0u)                              /* 0x091C62 btst #1 */
        wr32(m, OFF_Y, rd32(m, OFF_Y) + 0x00020000u);
    if ((f & 0x04u) == 0u)                              /* 0x091C74 btst #2 */
        wr32(m, OFF_X, rd32(m, OFF_X) - 0x00020000u);
    if ((f & 0x08u) == 0u)                              /* 0x091C86 btst #3 */
        wr32(m, OFF_X, rd32(m, OFF_X) + 0x00020000u);
}

/* 0x091ABC..0x091AC4 / 0x091AEE..0x091AF6 / 0x091B4E..0x091B56:
 *   lsl.w #7,D2w ; add.w D1w,D1w ; add.w D2w,D1w ; ext.l D1 ; addi.l #0x61C000 */
static uint32_t text_addr(uint16_t col, uint16_t row)
{
    uint16_t w = (uint16_t)((uint16_t)(col << 1) + (uint16_t)(row << 7));
    return (uint32_t)(int32_t)(int16_t)w + TEXT_BASE;
}

/* One 0x005666 string at a 0x61C000 text address; returns the new cursor. */
static uint32_t put_text(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t attr)
{
    return Text_DrawString(m, src, dst, attr);
}

/* ROM 0x0919F4: place + draw the record A1 for object A0 and its readout.
 * D0/D1/A1 leave as the ROM leaves them; A2/D2/D3 are internal. */
static void sub_0919F4(const f3mem_t *m, f3regs_t *r, uint32_t *a1_io)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = *a1_io;
    uint32_t d0 = r->d[0];
    uint32_t a2 = rd32(m, a1 + 0x02u);                  /* 0x0919F4 (long at +2) */
    a2 = rd32(m, a2 + 0x08u);                           /* 0x0919F8 */

    wr8(m, obj + 0x1Cu, rd8(m, a1 + 0x0Au));            /* 0x0919FC */
    wr16(m, obj + 0x14u, rd16(m, OFF_X));               /* 0x091A02 */
    d0 = setw(d0, rd16(m, a1 + 0x06u));                 /* 0x091A08 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x091A0C */
    wr16(m, obj + 0x18u, rd16(m, OFF_Y));               /* 0x091A10 */
    d0 = setw(d0, rd16(m, a1 + 0x08u));                 /* 0x091A16 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x091A1A */

    if ((rd8(m, DBG_FLAGS) & 0x04u) != 0u) {            /* 0x091A1E btst #2 */
        uint32_t d1;
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0048u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0048u));
        d0 = setw(d0, 0x00FFu);                         /* 0x091A34 */
        d0 = setw(d0, (uint16_t)((uint8_t)d0 - rd8(m, obj + 0x1Fu))); /* 0x091A38 */
        d1 = setw(r->d[1], rd16(m, a2 + 0x02u));        /* 0x091A3C */
        d1 = setw(d1, (uint16_t)(d1 << 3));             /* 0x091A40 */
        d1 = (uint32_t)((int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)(uint16_t)d1);
        d1 >>= 8;                                       /* 0x091A44 lsr.l #8 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)d1));
        d0 = setw(d0, 0x00FFu);                         /* 0x091A4A */
        d0 = setw(d0, (uint16_t)((uint8_t)d0 - rd8(m, obj + 0x1Eu))); /* 0x091A4E */
        d1 = setw(d1, rd16(m, a2));                     /* 0x091A52 */
        d1 = setw(d1, (uint16_t)(d1 << 3));             /* 0x091A54 */
        d1 = (uint32_t)((int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)(uint16_t)d1);
        d1 >>= 8;                                       /* 0x091A58 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)d1));
        r->d[1] = d1;
    }

    if ((rd8(m, DBG_FLAGS) & 0x02u) != 0u) {            /* 0x091A5E btst #1 */
        wr32(m, TMP_SPR, rd32(m, a2));                  /* 0x091A68 */
        wr8(m, obj + 0x1Du, 0u);                        /* 0x091A6E */
        wr32(m, obj + 0x10u, TMP_SPR);                  /* 0x091A72 */
        r->d[0] = d0;
        r->a[0] = obj;
        batch_draw_sprite(m, r, 0x00091A84u);           /* 0x091A7A movem {A2,A1} */
        r->a[1] = a1;
        r->a[2] = a2;
    }

    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x0Bu));            /* 0x091A88 */
    wr32(m, obj + 0x10u, a2);                           /* 0x091A8E */
    r->d[0] = d0;
    r->a[0] = obj;
    batch_draw_sprite(m, r, 0x00091A9Cu);               /* 0x091A92 movem {A1} */

    /* 0x091AA0..0x091AAE: text column/row from 0x407390, plus the margins. */
    uint32_t t = 0x00407390u;
    uint32_t d1 = setw(r->d[1], rd16(m, t));
    t += 2u;
    uint32_t d2 = setw(r->d[2], rd16(m, t));
    t += 2u;
    d1 = setw(d1, (uint16_t)(d1 + 0x0007u));
    d2 = setw(d2, (uint16_t)(d2 + 0x0001u));
    uint32_t d3 = 0;

    a2 = rd32(m, obj + 0x10u);                          /* 0x091AB0 */

    /* 0x091AB4..0x091AD8: value A2 (the sprite pointer), 6 hex digits. */
    d0 = Gfx_DrawHex(m, a2, 6u, text_addr((uint16_t)d1, (uint16_t)d2), 7u);
    d2 = setw(d2, (uint16_t)(d2 + 1u));                 /* 0x091AE0 */
    d3 = setw(d3, rd16(m, a2 + 0x02u));                 /* 0x091AE2 */
    /* 0x091AE6..0x091B0A: the record's +0x02 word, 2 decimal digits. */
    d0 = Gfx_DrawDecimal(m, d3, 2u, text_addr((uint16_t)d1, (uint16_t)d2), 7u);

    d0 = put_text(m, STR_STAR, d0, 0x000Au);            /* 0x091B12 */
    d3 = setw(d3, rd16(m, a2));                         /* 0x091B28 */
    d0 = Gfx_DrawDecimal(m, d3, 2u, d0, 7u);            /* 0x091B2A */

    d2 = setw(d2, (uint16_t)(d2 + 1u));                 /* 0x091B40 */
    d3 = setw(d3, rd16(m, a2 + 0x06u));                 /* 0x091B42 */
    d0 = Gfx_DrawHex(m, d3, 2u, text_addr((uint16_t)d1, (uint16_t)d2), 7u); /* 0x091B46 */

    d1 = setw(d1, rd8(m, MODE_BYTE));                   /* 0x091B72 move.b */
    if ((uint8_t)d1 == 0u) {
        d0 = put_text(m, STR_NORMAL, d0, 5u);           /* 0x091B7A */
    } else if ((int8_t)d1 > 3) {                        /* 0x091B94 cmpi.b #3 */
        d0 = put_text(m, STR_DEMO, d0, 4u);             /* 0x091BE2 */
    } else {
        d0 = put_text(m, STR_BOSS, d0, 8u);             /* 0x091B9C */
        d0 = Gfx_DrawHex(m, d1, 1u, d0, 8u);            /* 0x091BB2 */
        d0 = put_text(m, STR_CLOSE, d0, 8u);            /* 0x091BC8 */
    }

    /* 0x091BF8..0x091C16: copy the palette cell of the sprite's colour byte. */
    {
        uint32_t save_a1 = a1;
        uint32_t pal_a2 = rd32(m, a1 + 0x02u);
        uint32_t src = PAL_CELLS;
        pal_a2 = rd32(m, pal_a2 + 0x08u);
        d0 = setw(d0, (uint16_t)(rd16(m, pal_a2 + 0x06u) & 0x00FFu));
        d0 = setw(d0, (uint16_t)(d0 << 6));
        src += (uint32_t)(uint16_t)d0;                  /* 0x091C14 adda.w */
        {
            f3regs_t t2 = *r;
            t2.a[1] = src;
            (void)Fn_0B7B28(m, &t2);                    /* 0x091C16 */
        }
        a1 = save_a1;                                   /* 0x091C1C movem (SP)+,{A1} */
        wr32(m, PAL_OUT, rd32(m, DITHER));              /* 0x091C20 */
        wr32(m, DITHER, rd32(m, DITHER) ^ 0x006F6F6Fu); /* 0x091C28 eori.l */
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    *a1_io = a1;
}

int Fn_091984(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    sub_091C50(m);                                      /* 0x091984 bsr */

    if ((rd8(m, DBG_FLAGS) & 0x01u) == 0u) {            /* 0x091988 btst #0 */
        a1 = REC_BASE;                                  /* 0x091992 */
        d0 = setw(d0, (uint16_t)(rd16(m, INDEX_W) - 1u)); /* 0x091998 subq.w */
        int16_t idx = (int16_t)d0;
        if (idx < 0)                                    /* 0x09199E blt */
            goto lab_09194A;
        if (idx >= 0x000E)                              /* 0x0919A0 cmpi/bge */
            goto lab_09194A;
        d0 = (uint32_t)((int32_t)idx * 0x0C);           /* 0x0919A6 muls.w #0xC */
        a1 += (uint32_t)(int32_t)(int16_t)(uint16_t)d0;  /* 0x0919AA adda.w */
        r->d[0] = d0;
        r->d[1] = d1;
        sub_0919F4(m, r, &a1);                          /* 0x0919AC bsr */
        d0 = r->d[0];
        d1 = r->d[1];
        goto lab_09194A;                                /* 0x0919B0 bra */
    }

    if ((int32_t)rd32(m, CURSOR) <= 0)                  /* 0x0919B2 tst.l/ble */
        goto lab_09194A;
    a1 = rd32(m, CURSOR);                               /* 0x0919B8 */
    r->d[0] = d0;
    r->d[1] = d1;
    sub_0919F4(m, r, &a1);                              /* 0x0919BC bsr */
    d0 = r->d[0];
    d1 = r->d[1];
    a1 = rd32(m, CURSOR);                               /* 0x0919C0 */
    wr16(m, COUNTER, (uint16_t)(rd16(m, COUNTER) + 1u)); /* 0x0919C4 addq.w */
    d0 = setw(d0, rd16(m, COUNTER));                    /* 0x0919C8 */
    if ((int16_t)d0 < (int16_t)rd16(m, a1))             /* 0x0919CC cmp.w/blt */
        goto lab_09197C;
    wr16(m, COUNTER, 0u);                               /* 0x0919D0 */
    for (;;) {                                          /* 0x0919D6 */
        a1 += 0x0Cu;
        if ((int32_t)a1 >= (int32_t)REC_END)            /* 0x0919DA cmpa.l/bge */
            goto lab_09194A;
        int16_t w = (int16_t)rd16(m, a1);               /* 0x0919E4 tst.w */
        if (w == 0)
            continue;                                   /* 0x0919E6 beq */
        if (w < 0)                                      /* 0x0919E8 blt */
            goto lab_09194A;
        wr32(m, CURSOR, a1);                            /* 0x0919EC */
        goto lab_09197C;                                /* 0x0919F0 bra */
    }

lab_09194A:
    a1 = REC_BASE;                                      /* 0x09194A */
    d0 = setw(d0, 0x000Du);                             /* 0x091950 */
    wr32(m, CURSOR, 0xFFFFFFFFu);                       /* 0x091954 */
    wr16(m, COUNTER, 0u);                               /* 0x09195C */
    for (;;) {                                          /* 0x091962 */
        int16_t w = (int16_t)rd16(m, a1);
        if (w > 0) {                                    /* 0x091964 bgt */
            wr32(m, CURSOR, a1);                        /* 0x091978 */
            break;
        }
        if (w < 0)                                      /* 0x091968 blt */
            break;
        a1 += 0x0Cu;                                    /* 0x09196C lea */
        d0 = setw(d0, (uint16_t)((uint16_t)d0 - 1u));   /* 0x091970 dbf */
        if ((uint16_t)d0 == 0xFFFFu)
            break;                                      /* 0x091974 bra */
    }

lab_09197C:
    wr32(m, obj + 0x00u, 0x00091984u);                  /* 0x09197C */

    r->a[0] = obj;
    r->a[1] = a1;
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
