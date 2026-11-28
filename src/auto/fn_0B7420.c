/* ROM 0x0B7420: multi-line text block printer (stack arguments).
 * Args (at SP+4 of the entry frame): word count, word attr, word attr2,
 * long source-long-list, long table. It reads two words (D0,D1) from the
 * source list and a long pointer (A2) from the table, steps A2 either back
 * into [0, count) or forward, stores {1+D0, (A2)+D1+1} back into the source
 * list, then prints `count` rows: for each row a text string from the list is
 * drawn at 0x61C000 + (D0*2 + D1*128) with attr, plus -- when D1 == (A2)+D1 --
 * the fixed string 0x0B7500 with attr2. Finally, unless A5-0x5DDB bit 4 is
 * set, D0 = table[4 + (A2)*8]. All other registers are saved/restored. */
#include "fn_auto.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (0x005666) */

/* The hardware drops writes to ROM and to unmapped space (the capture keeps
 * the old RAM bytes there); only Work RAM writes are observable. */
static int is_ram(uint32_t a)
{
    a &= 0xFFFFFFu;
    if (a >= 0x420000u && a < 0x440000u)   /* Work RAM mirror */
        a -= 0x20000u;
    return a >= F3_WRAM_BASE && a < F3_WRAM_BASE + F3_WRAM_SIZE;
}

static void hw_wr16(const f3mem_t *m, uint32_t a, uint16_t v)
{
    if (is_ram(a))
        wr16(m, a, v);
}

int Fn_0B7420(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d2w = rd16(m, sp + 4u);            /* 0x0B7424 */
    uint16_t d3w = rd16(m, sp + 6u);            /* 0x0B7428 */
    uint16_t d4w = rd16(m, sp + 8u);            /* 0x0B742C */
    uint32_t a0 = rd32(m, sp + 10u);            /* 0x0B7430 */
    uint32_t a1 = rd32(m, sp + 14u);            /* 0x0B7434 */

    uint16_t d0w = rd16(m, a0);                 /* 0x0B7438 (A0)+ */
    a0 += 2u;
    uint16_t d1w = rd16(m, a0);                 /* 0x0B743A (A0)+ */
    a0 += 2u;
    uint32_t a2 = rd32(m, a1);                  /* 0x0B743C (A1)+ */
    a1 += 4u;

    uint8_t flags = rd8(m, F3_A5 - 0x5DDBu);    /* 0x0B743E..0x0B745A */

    if ((flags & 0x01u) == 0u) {                /* 0x0B7444 bne */
        uint16_t v = (uint16_t)(rd16(m, a2) - 1u); /* 0x0B7448 subq.w */
        hw_wr16(m, a2, v);
        if ((int16_t)v < 0)                     /* 0x0B744A cmpi / 0x0B744E bge */
            hw_wr16(m, a2, (uint16_t)(v + d2w)); /* 0x0B7452 */
    }
    if ((flags & 0x02u) == 0u) {                /* 0x0B7454 / 0x0B745A bne */
        uint16_t v = (uint16_t)(rd16(m, a2) + 1u); /* 0x0B745E addq.w */
        hw_wr16(m, a2, v);
        if (!((int16_t)d2w > (int16_t)v))       /* 0x0B7460 cmp.w (A2),D2w ; bgt */
            hw_wr16(m, a2, (uint16_t)(v - d2w)); /* 0x0B7466 sub.w */
    }

    hw_wr16(m, a0, 0x0001u);              /* 0x0B7468 move.w #1,(A0) */
    hw_wr16(m, a0, (uint16_t)(rd16(m, a0) + d0w)); /* 0x0B746C */
    hw_wr16(m, a0 + 2u, rd16(m, a2));     /* 0x0B746E */
    hw_wr16(m, a0 + 2u, (uint16_t)(rd16(m, a0 + 2u) + d1w)); /* 0x0B7472 */
    hw_wr16(m, a0 + 2u, (uint16_t)(rd16(m, a0 + 2u) + 1u));  /* 0x0B7476 */

    a0 = a1;                                    /* 0x0B747A movea.l A1,A0 */
    d0w = (uint16_t)(d0w + 1u);                 /* 0x0B747C */
    d1w = (uint16_t)(d1w + 1u);                 /* 0x0B747E */
    uint16_t d5w = (uint16_t)(d1w + rd16(m, a2)); /* 0x0B7482/0x0B7484 */

    /* 0x0B7486 loop; D2w guards the iteration count (1 = print one row). */
    for (unsigned iter = 0; iter <= 0x8000u; iter++) {
        uint32_t src = rd32(m, a0);             /* 0x0B749C move.l (A0),-(SP) */
        uint32_t dst = 0x0061C000u
                     + (uint32_t)(int32_t)(int16_t)(uint16_t)(
                           (uint16_t)(d0w + d0w) + (uint16_t)(d1w << 7)); /* 0x0B748C..0x0B7494 */
        (void)Text_DrawString(m, src, dst, d3w); /* 0x0B749E */

        if (d5w == d1w)                          /* 0x0B74AC cmp.w D1w,D5w */
            (void)Text_DrawString(m, 0x000B7500u, dst, d4w); /* 0x0B74CE */

        a0 += 8u;                                /* 0x0B74DC */
        d1w = (uint16_t)(d1w + 1u);              /* 0x0B74E0 */
        d2w = (uint16_t)(d2w - 1u);              /* 0x0B74E2 */
        if (!((int16_t)d2w > 0))                 /* 0x0B74E4 bgt */
            break;
    }

    uint32_t d0 = 0u;                            /* 0x0B74E6 moveq #0 */
    if ((flags & 0x10u) == 0u) {                 /* 0x0B74E8 btst #4 / bne */
        uint16_t idx = (uint16_t)(rd16(m, a2) << 3); /* 0x0B74F2/0x0B74F4 */
        d0 = rd32(m, a1 + 4u + (uint32_t)idx);   /* 0x0B74F6 */
    }
    r->d[0] = d0;                                /* 0x0B74FA restores D1-D5/A0-A2 */
    return 0;
}
