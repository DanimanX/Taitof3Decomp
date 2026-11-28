/* ROM 0x0C25A6: build the HUD strip that belongs to the object at A4 and queue
 * it for drawing. The 0x80-byte block at A4+0x85A gets its payload copied from
 * the ROM record selected by the word at A4+0x4E (pointer table at 0x0C26FA),
 * either verbatim (A4+0x858 == 0) or faded towards 0xFF by the counter left in
 * A4+0x858 after the decrement. The last four bytes are the object's slant,
 * (0xD8 * D7) >> 5 and (0x38 * D7) >> 5, where D7 comes from the alternating
 * field phase A5+0x1C2E; then the block is queued through 0x000C58 as type
 * 0x5000 (0x5080 for player 2, word at A4+4) with a height of 0x20. */
#include "ea_auto.h"

/* 0x000C58 (not translated yet): append one draw block to the 0x30-entry queue
 * at 0x417242/0x417240. The args follow the jsr: 4(SP) = block, 8(SP) = type
 * word, 0xC(SP) = height word. D0 = 0, or 0xFFFFFFFF when the queue is full.
 * D1, D2, A0 and A2 are saved and restored. */
static void Sub_C58(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = rd32(m, r->a[7] + 4u);             /* 0x000C64 movea.l $8(a6),a0 */
    uint32_t d0 = rd32(m, r->a[7] + 8u);             /* 0x000C68 move.l $c(a6),d0 */
    uint32_t d1 = rd32(m, r->a[7] + 12u);            /* 0x000C6C move.l $10(a6),d1 */
    uint16_t d2 = rd16(m, 0x00417240u);              /* 0x000C70 */

    if ((int16_t)d2 >= 0x0030) {                     /* 0x000C76 bge 0x000CAC */
        r->d[0] = 0xFFFFFFFFu;                       /* 0x000CAC moveq #$ff,d0 */
    } else {
        uint32_t a2 = 0x00416000u + (uint32_t)(uint16_t)(d2 << 3); /* 0x000C7C..84 */

        wr16(m, a2, (uint16_t)d1);                   /* 0x000C86 move.w d1,(a2) */
        wr32(m, a2 + 2u, a0);                        /* 0x000C88 */
        wr16(m, a2 + 6u, (uint16_t)d0);              /* 0x000C8C */
        wr16(m, 0x00417242u, 0x0002u);               /* 0x000C90 */
        wr16(m, 0x00417240u, (uint16_t)(d2 + 1u));   /* 0x000C98 */
        r->d[0] = 0u;                                /* 0x000C9E moveq #$0,d0 */
    }
    r->a[7] += 4u;                                   /* 0x000CAA rts */
}

/* 0x0C25F6..0x0C2604 / 0x0C2632..0x0C2648: one faded sample, D1 = value,
 * D0 = 0xFF - value, D1 *= D5, D1 >>= 7 (32-bit), D0 += D1.w. */
static uint16_t fade(uint16_t v, uint16_t d5)
{
    int32_t t = ((int32_t)0x00FF - (int32_t)v) * (int32_t)(int16_t)d5;

    return (uint16_t)(v + (uint16_t)(t >> 7));
}

/* 0x0C2634 / 0x0C2652 / 0x0C26AE / 0x0C26C0: D0 = (D1 * D7) >> 5 with a
 * signed 16x16 product and a 32-bit arithmetic shift. */
static uint16_t slant(uint16_t k, uint32_t d7)
{
    int32_t t = (int32_t)(int16_t)k * (int32_t)(int16_t)(uint16_t)d7;

    return (uint16_t)(t >> 5);
}

int Ea_0C25A6(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a4 = r->a[4];
    const uint32_t a5 = r->a[5];
    uint32_t d0 = r->d[0], d1 = r->d[1];
    uint32_t d5 = r->d[5], d6 = r->d[6], d7 = r->d[7];
    uint32_t a0 = r->a[0], a1 = r->a[1];

    if (rd16(m, a4 + 0x0002u) == 0u)                 /* 0x0C25A6 beq 0x0C25A4 */
        goto done;                                   /* bare rts, nothing changes */
    if (rd32(m, a4 + 0x0052u) == 0u)                 /* 0x0C25AC beq 0x0C25A4 */
        goto done;

    a0 = a4 + 0x085Au;                               /* 0x0C25B2 lea.l $85a(a4),a0 */
    a1 = 0x000C26FAu;                                /* 0x0C25B6 lea.l $c26fa(pc),a1 */
    d0 = set_word(d0, rd16(m, a4 + 0x004Eu));        /* 0x0C25BA move.w $4e(a4),d0 */
    a1 += 4u * (uint32_t)(int32_t)(int16_t)(uint16_t)d0; /* 0x0C25BE */
    /* 0x0C25C2 movea.l (a1)+,a1: with the row taken from and written to A1 the
     * post-increment is lost, so A1 becomes the record pointer itself. */
    a1 = rd32(m, a1);

    d7 = set_word(d7, (uint16_t)(rd16(m, a5 + 0x1C2Eu) & 0x003Fu)); /* 0x0C25C4 */
    if (((uint8_t)d7 & 0x20u) != 0u) {               /* 0x0C25CC btst.b #5,d7 */
        d7 = set_word(d7, (uint16_t)(0u - (uint16_t)((uint16_t)d7 & 0x001Fu)));
        d7 = set_word(d7, (uint16_t)((uint16_t)d7 + 0x0020u)); /* 0x0C25D8 */
    }

    a0 += 4u;                                        /* 0x0C25DC addq.w #4,a0 */
    a1 += 4u;                                        /* 0x0C25DE addq.w #4,a1 */

    if (rd16(m, a4 + 0x0858u) == 0u)                 /* 0x0C25E0 beq 0x0C2670 */
        goto copy;
    {
        uint16_t w = (uint16_t)(rd16(m, a4 + 0x0858u) - 1u);

        wr16(m, a4 + 0x0858u, w);                    /* 0x0C25E8 subq.w #1 */
        if (w == 0u)                                 /* 0x0C25EC beq 0x0C2670 */
            goto copy;

        /* ---- 0x0C25F0: fade the 30 records towards white --------------- */
        d5 = set_word(d5, w);                        /* 0x0C25F0 */
        d6 = 0x001Du;                                /* 0x0C25F4 moveq #$1d,d6 */
        do {
            d0 = set_word(d0, rd16(m, a1));          /* 0x0C25F6 move.w (a1)+,d0 */
            a1 += 2u;
            d0 = set_word(d0, fade((uint16_t)d0, (uint16_t)d5)); /* 0x0C25F8..04 */
            wr16(m, a0, (uint16_t)d0);               /* 0x0C2604 move.w d0,(a0)+ */
            a0 += 2u;
            for (unsigned i = 0; i < 3u; i++) {      /* 0x0C2606..0x0C262C */
                d0 = 0u;                             /* 0x0C2606 moveq #$0,d0 */
                d1 = 0u;                             /* 0x0C2608 moveq #$0,d1 */
                d0 = set_word(d0, rd8(m, a1));       /* 0x0C260A move.b (a1)+,d0 */
                a1 += 1u;
                d0 = set_word(d0, fade((uint16_t)d0, (uint16_t)d5));
                wr8(m, a0, (uint8_t)d0);             /* 0x0C2618 move.b d0,(a0)+ */
                a0 += 1u;
            }
            d6 = (uint16_t)(d6 - 1u);                /* 0x0C262E dbra d6 */
        } while (d6 != 0xFFFFu);
    }
    /* 0x0C2632: the two slant samples, faded as well */
    d0 = set_word(0u, slant(0x00D8u, d7));           /* 0x0C2632..0x0C263C */
    d0 = set_word(d0, fade((uint16_t)d0, (uint16_t)d5));
    wr16(m, a0, (uint16_t)d0);                       /* 0x0C264A */
    a0 += 2u;
    d1 = 0u;                                         /* 0x0C264C..0x0C264E */
    d0 = set_word(0u, slant(0x0038u, d7));           /* 0x0C2650..0x0C265A */
    d0 = set_word(d0, fade((uint16_t)d0, (uint16_t)d5));
    wr8(m, a0, (uint8_t)d0);                         /* 0x0C2668 move.b d0,(a0)+ */
    a0 += 1u;
    wr8(m, a0, 0u);                                  /* 0x0C266A clr.b (a0)+ */
    a0 += 1u;
    goto queue;                                      /* 0x0C266C bra.w 0x0C26CE */

copy:                                                /* 0x0C2670 */
    for (unsigned i = 0; i < 30u; i++) {             /* move.l (a1)+,(a0)+ */
        wr32(m, a0, rd32(m, a1));
        a1 += 4u;
        a0 += 4u;
    }
    /* 0x0C26AC: the slant samples without any fading */
    d0 = set_word(0u, slant(0x00D8u, d7));           /* 0x0C26AC..0x0C26B6 */
    wr16(m, a0, (uint16_t)d0);                       /* 0x0C26B8 */
    a0 += 2u;
    d1 = 0u;                                         /* 0x0C26BA..0x0C26BC */
    d0 = set_word(0u, slant(0x0038u, d7));           /* 0x0C26BE..0x0C26C8 */
    wr8(m, a0, (uint8_t)d0);                         /* 0x0C26CA */
    a0 += 1u;
    wr8(m, a0, 0u);                                  /* 0x0C26CC clr.b (a0)+ */
    a0 += 1u;

queue:                                               /* 0x0C26CE */
    a1 = a4 + 0x085Au;                               /* 0x0C26CE lea.l $85a(a4),a1 */
    d1 = 0x00005000u;                                /* 0x0C26D2 move.l #$5000,d1 */
    if (rd16(m, a4 + 0x0004u) != 0u)                 /* 0x0C26D8 beq 0x0C26E4 */
        d1 = 0x00005080u;                            /* 0x0C26DE move.l #$5080,d1 */
    push_long(m, r, 0x00000020u);                    /* 0x0C26E4 move.l #$20,-(a7) */
    push_long(m, r, d1);                             /* 0x0C26EA move.l d1,-(a7) */
    push_long(m, r, a1);                             /* 0x0C26EC move.l a1,-(a7) */
    push_long(m, r, 0x0C26F4u);                      /* 0x0C26EE jsr $c58 */
    Sub_C58(m, r);
    d0 = r->d[0];
    r->a[7] += 12u;                                  /* 0x0C26F4 lea.l $c(a7),a7 */

done:                                                /* 0x0C25A4 / 0x0C26F8 rts */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[5] = d5;
    r->d[6] = d6;
    r->d[7] = d7;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[7] += 4u;
    return 0;
}
