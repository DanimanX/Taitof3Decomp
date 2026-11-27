/* ROM 0x13D69A high-score name entry (called from the player code at
 * 0x0C73EC), translated in segments (src/kernel/seg.h).
 *
 *   13D69A  movem.l {D0-D3/A0-A2},-(SP)
 *   13D69E  A0 = 0x41B190 (two 0x8A player structs) / D2 = 1
 *   13D6A8  per player: cursor fields (6,3A,3E) = 0, bsr 0x13D638 (insert
 *           score (0x48,A0) into the 5 records at 0x406CD4); a qualifying
 *           player's new record name = player id long (0x22,A0), low word
 *           cleared
 *   13D6CE  per player: find the record whose name word is the player id,
 *           blank it to "..." and keep its index (or -1) at (0x48,A0)
 *   13D712  D7 = 0x708 (frames left)
 *   13D718  frame: bsr 0x13D94C (blank panel + "BEST 5" + the five records);
 *           A0 = 0x41B190, D0w = 1, sf.b D1; for each player with a record
 *           index >= 0: A1 = record, bsr 0x13D7A0 (row, cursor, input),
 *           addq.b #1,D1; dbf D0w
 *   13D74A  trap #5                          <- the routine's only yield
 *   13D74C  tst.b D1 / beq 13D790 (no row drawn: leave)
 *   13D752  "TIME:" at 0x61CCA0 + D7/60 as 2 digits at 7, dbf D7 -> 13D718
 *   13D790  bsr 0x13D94C, restore, rts       (back into untranslated player
 *           code: SEG_LEAVE, no capture covers that path)
 *
 * Callees are inlined register-exactly (memory and registers):
 *   0x005666 Text_DrawString(m, src, dst, col) -> D0 = end, keeps D7/A0/A1
 *   0x005690 Gfx_DrawHex / 0x00571A Gfx_DrawDecimal -> D0 = dst + 2*digits
 *   0x13D638 Hiscore_Insert -> D0 = score, D1w = slot/-1, A1 = record,
 *            A2 = record (the "-1" path returns before 13D65E's
 *            `lea 0x406D04,A2`, so A2 keeps its caller value there)
 *   0x13D7A0 row draw: movem keeps D0/D1/A0-A3, leaks D2 (row offset, then
 *            the next player's record index) and D3 (the row's attribute)
 *   0x13D94C panel draw: movem keeps D0-D3/A0/D7, leaks D4/D5 (last record's
 *            digit count and zone bit) and A1/A2
 * Every 68k push (movem blocks, jsr/bsr return addresses, arguments) is
 * reproduced: the stack lives in Work RAM and the capture compares it. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"    /* Text_DrawString  0x005666 */
#include "../player/player_task.h"  /* Hiscore_Insert   0x13D638 */
#include "../video/sprite.h"        /* Gfx_DrawHex 0x005690, Gfx_DrawDecimal 0x00571A */

#define ROM_TBL_PANEL_LABEL 0x0013D916u   /* {attr,"1P",0}, {attr,"2P",0} pointers */
#define ROM_TBL_ALPHABET    0x0013D926u   /* "ABC...Z0123456789." */
#define ROM_TBL_ZONE_CHARS  0x0013D5C8u   /* {char,0} pairs */
#define ROM_TXT_COUNTDOWN   0x0013D79Au   /* "TIME:" */
#define ROM_TXT_BEST5       0x0013DAAAu   /* "  BEST 5   " */
#define ROM_TXT_COLUMNS     0x0013DAB6u   /* "RANK    SCORE    ZONE NAME" */
#define ROM_TXT_QUOTE       0x0013DAD1u   /* "'" */
#define ROM_PANEL_TILES     0x00626200u   /* + (A5-0x5D56).w, 0xE8 words of 0x0CFC */
#define ROM_TXT_COUNTDOWN_DST 0x0061CCA0u

/* ---- 68k stack helpers -------------------------------------------------- */

static void push32(const f3mem_t *m, f3regs_t *r, uint32_t v) { seg_push32(m, r, v); }

/* push a word: move.w #imm,-(SP) writes two bytes */
static void push16(const f3mem_t *m, f3regs_t *r, uint16_t v)
{
    r->a[7] -= 2u;
    wr16(m, r->a[7], v);
}

/* movem.l <list>,-(SP) / movem.l (SP)+,<list>. `mask` uses the canonical
 * numbering (bit n = Dn for n < 8, A(n-8) above); both directions touch the
 * registers at ascending addresses in that order. */
static void movem_push(const f3mem_t *m, f3regs_t *r, uint16_t mask)
{
    unsigned n = 0;
    for (unsigned i = 0; i < 16u; i++)
        if (mask & (1u << i)) n++;
    r->a[7] -= 4u * n;
    uint32_t a = r->a[7];
    for (unsigned i = 0; i < 16u; i++)
        if (mask & (1u << i)) {
            wr32(m, a, i < 8u ? r->d[i] : r->a[i - 8u]);
            a += 4u;
        }
}

static void movem_pop(const f3mem_t *m, f3regs_t *r, uint16_t mask)
{
    uint32_t a = r->a[7];
    for (unsigned i = 0; i < 16u; i++)
        if (mask & (1u << i)) {
            uint32_t v = rd32(m, a);
            if (i < 8u) r->d[i] = v; else r->a[i - 8u] = v;
            a += 4u;
        }
    r->a[7] = a;
}

/* ---- callees ------------------------------------------------------------ */

/* ROM 0x13D638, called with A0 = player: A1 = record, D1w = slot or -1,
 * D0 = the score long. Memory comes from Hiscore_Insert (0x13D638). */
static void fx_13D638(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t rec = 0;
    int16_t slot = Hiscore_Insert(m, a0, &rec);
    r->d[0] = rd32(m, a0 + 0x48u);                   /* 13D638 / 13D696 */
    r->a[1] = rec;
    if (slot < 0) {
        r->d[1] = 0x0000FFFFu;                       /* 13D656 move.w #-1 (after moveq #0) */
    } else {
        r->d[1] = (uint32_t)slot;                    /* 13D642 moveq #0 + addq.w */
        r->a[2] = rec;                               /* 13D65E..13D68A exit */
    }
}

/* ROM 0x13D7A0: draw one player's row (label, cursor cell, the name character
 * under the cursor, rude-name fixup) and apply the left/right/confirm bits of
 * the input pointer at (0x44,A0). Called with A0 = player, A1 = record, D7 =
 * frames left; saves D0/D1/A0-A3, so it leaks only D2 and D3. */
static void fx_13D7A0(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a0 = r->a[0];
    const uint32_t a1 = r->a[1];
    movem_push(m, r, 0x0F03u);                       /* 13D7A0 {D0 D1 A0-A3} */
    /* 13D7A4 lea (0x170,PC),A2 ; 13D7AA/13D7AE A2 = label[(0x22,A0).w] */
    uint32_t a2 = rd32(m, ROM_TBL_PANEL_LABEL +
                       (uint32_t)(int32_t)(int16_t)rd16(m, a0 + 0x22u) * 4u);
    seg_setw(&r->d[1], 8u);                          /* 13D7B2 D1w = 8 (column) */
    /* 13D7B6..13D7BC: D2 = (0x48,A0).w * 2 + 0xD (row) */
    r->d[2] = rd32(m, a0 + 0x48u);
    seg_setw(&r->d[2], (uint16_t)((uint16_t)r->d[2] + (uint16_t)r->d[2]));
    seg_setw(&r->d[2], (uint16_t)((uint16_t)r->d[2] + 0x000Du));
    seg_setw(&r->d[3],                               /* 13D7C0 move.b (A2)+,D3b */
             (uint16_t)((r->d[3] & 0xFF00u) | rd8(m, a2)));
    a2 += 1u;
    movem_push(m, r, 0x0006u);                       /* 13D7C2 {D1 D2} */
    push16(m, r, (uint16_t)r->d[3]);                 /* 13D7C6 attr */
    seg_setw(&r->d[2], (uint16_t)((uint16_t)r->d[2] << 7));                 /* 13D7C8 */
    seg_setw(&r->d[1], (uint16_t)((uint16_t)r->d[1] + (uint16_t)r->d[1]));  /* 13D7CA */
    seg_setw(&r->d[1], (uint16_t)((uint16_t)r->d[1] + (uint16_t)r->d[2]));  /* 13D7CC */
    r->d[1] = (uint32_t)(int32_t)(int16_t)r->d[1];   /* 13D7CE ext.l */
    r->d[1] += 0x0061C000u;                          /* 13D7D0 dst */
    push32(m, r, r->d[1]);                           /* 13D7D6 */
    push32(m, r, a2);                                /* 13D7D8 src */
    push32(m, r, 0x0013D7E0u);
    r->d[0] = Text_DrawString(m, a2, r->d[1], (uint16_t)r->d[3]);   /* 13D7DA */
    r->a[7] += 10u;                                  /* 13D7E0 lea 0xa(SP),SP */
    (void)seg_pop32(m, r);
    movem_pop(m, r, 0x0006u);                        /* 13D7E4 */
    a2 = r->d[0];                                    /* 13D7E8 movea.l D0,A2 */
    /* 13D7EA/13D7EE A2 = end of the label + 0x2A + 2*(0x3E,A0).w (cursor cell) */
    a2 += 0x2Au + (uint32_t)(int32_t)(int16_t)(uint16_t)(2u * rd16(m, a0 + 0x3Eu));
    /* 13D7F2 btst.l #0,D7: odd frames take the cell from the cursor word at
     * 0x106B6, even frames keep the RAM word. The ROM's bfins D1,D0{0x11:0x6}
     * writes 6 bits from bit 14 down (bit 31 - 17), i.e. word bits 14..9:
     * 0x0E60 with value 8 becomes 0x1060 in the capture. */
    if ((r->d[7] & 1u) != 0u) {
        uint16_t w = rd16(m, 0x000106B6u);           /* 13D808 */
        seg_setw(&r->d[0], (uint16_t)((w & 0x81FFu) | (8u << 9)));   /* bfins #8 */
    } else {
        uint16_t w = rd16(m, a2);                    /* 13D7FA */
        seg_setw(&r->d[0], (uint16_t)((w & 0x81FFu) | (5u << 9)));   /* bfins #5 */
    }
    wr16(m, a2, (uint16_t)r->d[0]);                  /* 13D816 */
    /* 13D818..13D8A6: character stepping from the input byte at (0x44,A0). */
    uint32_t inp = rd32(m, a0 + 0x44u);
    int skip = 0;
    if ((rd8(m, inp) & 0x08u) == 0u) {               /* 13D81C btst.b #3 */
        if ((int16_t)rd16(m, a0 + 0x06u) < 0x14) {   /* 13D824 */
            wr16(m, a0 + 0x06u, (uint16_t)(rd16(m, a0 + 0x06u) + 1u));   /* 13D82E */
            if ((int16_t)rd16(m, a0 + 0x06u) > 1)    /* 13D832 bgt 13D8AC */
                skip = 1;
        }
        if (!skip)                                                    /* 13D83C */
            wr16(m, a0 + 0x3Au, (uint16_t)(rd16(m, a0 + 0x3Au) + 1u));
    } else if ((rd8(m, inp) & 0x04u) == 0u) {        /* 13D844 btst.b #2 */
        if ((int16_t)rd16(m, a0 + 0x06u) < 0x14) {   /* 13D84C */
            wr16(m, a0 + 0x06u, (uint16_t)(rd16(m, a0 + 0x06u) + 1u));   /* 13D856 */
            if ((int16_t)rd16(m, a0 + 0x06u) > 1)    /* 13D85A bgt 13D8AC */
                skip = 1;
        }
        if (!skip)                                                    /* 13D864 */
            wr16(m, a0 + 0x3Au, (uint16_t)(rd16(m, a0 + 0x3Au) - 1u));
    } else {
        wr16(m, a0 + 0x06u, 0u);                                      /* 13D86C */
    }
    if (!skip) {                                     /* 13D870: wrap 0..0x24 */
        uint16_t ch = rd16(m, a0 + 0x3Au);
        if ((int16_t)ch < 0) ch = (uint16_t)(ch + 0x25u);
        if ((int16_t)ch > 0x24) ch = (uint16_t)(ch - 0x25u);
        wr16(m, a0 + 0x3Au, ch);
        /* 13D89C lea (0x88,PC),A3 ; 13D8A2/13D8A6 store the alphabet char */
        wr8(m, a1 + 0x06u + (uint32_t)(int32_t)(int16_t)rd16(m, a0 + 0x3Eu),
            rd8(m, ROM_TBL_ALPHABET + (uint32_t)(int32_t)(int16_t)ch));
    }
    /* 13D8AC: the cursor keys only act while the countdown runs (D7.w > 0). */
    if ((int16_t)r->d[7] > 0) {
        if ((rd8(m, inp + 1u) & 0x20u) == 0u) {      /* 13D8B2 btst.b #5 */
            uint16_t t = (uint16_t)(rd16(m, a0 + 0x3Eu) - 1u);
            wr16(m, a0 + 0x3Eu, t);                  /* 13D8BC */
            if ((int16_t)t < 0)
                wr16(m, a0 + 0x3Eu, 0u);             /* 13D8C4 */
        }
        if ((rd8(m, inp + 1u) & 0x10u) == 0u) {      /* 13D8C8 btst.b #4 */
            uint16_t t = (uint16_t)(rd16(m, a0 + 0x3Eu) + 1u);
            wr16(m, a0 + 0x3Eu, t);                  /* 13D8D2 */
            if ((int16_t)t >= 3)
                wr32(m, a0 + 0x48u, 0xFFFFFFFFu);    /* 13D8E0: entry done */
        }
    }
    /* 13D8E8: do not leave a rude name in the table. */
    if (rd32(m, a1 + 0x06u) == 0x4B4B4B00u)
        wr32(m, a1 + 0x06u, 0x41414100u);
    if (rd32(m, a1 + 0x06u) == 0x53455800u)
        wr32(m, a1 + 0x06u, 0x41414100u);
    movem_pop(m, r, 0x0F03u);                        /* 13D910 */
}

/* ROM 0x13D94C: blank the panel rows, redraw "BEST 5" and the RANK/SCORE/
 * ZONE NAME columns of the five records. 13D94C saves D0-D3/A0 (and D7/A0
 * again around the tile fill), so it leaks D4/D5, A1 and A2. */
static void fx_13D94C(const f3mem_t *m, f3regs_t *r)
{
    movem_push(m, r, 0x010Fu);                       /* 13D94C {D0-D3 A0} */
    movem_push(m, r, 0x0180u);                       /* 13D950 {D7 A0} */
    uint32_t a0 = ROM_PANEL_TILES +
                  (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    seg_setw(&r->d[7], 0x00E7u);                     /* 13D95E */
    do {
        wr16(m, a0, 0x0CFCu);                        /* 13D962: 0xE8 words */
        a0 += 2u;
        seg_setw(&r->d[7], (uint16_t)((uint16_t)r->d[7] - 1u));
    } while ((uint16_t)r->d[7] != 0xFFFFu);          /* 13D966 dbf */
    movem_pop(m, r, 0x0180u);                        /* 13D96A */

    push16(m, r, 5u);                                /* 13D96E */
    push32(m, r, 0x0061C41Cu);                       /* 13D972 */
    push32(m, r, ROM_TXT_BEST5);                     /* 13D978 */
    push32(m, r, 0x0013D984u);
    (void)Text_DrawString(m, ROM_TXT_BEST5, 0x0061C41Cu, 5u);   /* 13D97E */
    r->a[7] += 10u;                                  /* 13D984 lea 0xa(SP),SP */
    (void)seg_pop32(m, r);

    push16(m, r, 8u);                                /* 13D988 */
    push32(m, r, 0x0061C590u);                       /* 13D98C */
    push32(m, r, ROM_TXT_COLUMNS);                   /* 13D992 */
    push32(m, r, 0x0013D99Eu);
    (void)Text_DrawString(m, ROM_TXT_COLUMNS, 0x0061C590u, 8u); /* 13D998 */
    r->a[7] += 10u;
    (void)seg_pop32(m, r);

    a0 = 0x00406CD4u;                                /* 13D9A2 */
    uint32_t a1 = ROM_TBL_ZONE_CHARS;                /* 13DA00 */
    uint32_t a2 = r->a[2];                           /* 13DA1C (only leaks) */
    seg_setw(&r->d[1], 1u);                          /* 13D9A8 rank */
    seg_setw(&r->d[2], 0x000Du);                     /* 13D9AC row */
    for (;;) {                                       /* 13D9B0, five records */
        seg_setw(&r->d[3], 0x000Bu);                 /* rank column */
        movem_push(m, r, 0x000Cu);                   /* 13D9B4 {D2 D3} */
        push16(m, r, 0x000Au);                       /* 13D9B8 attr */
        seg_setw(&r->d[2], (uint16_t)((uint16_t)r->d[2] << 7));                /* 13D9BC */
        seg_setw(&r->d[3], (uint16_t)((uint16_t)r->d[3] + (uint16_t)r->d[3])); /* 13D9BE */
        seg_setw(&r->d[3], (uint16_t)((uint16_t)r->d[3] + (uint16_t)r->d[2])); /* 13D9C0 */
        r->d[3] = (uint32_t)(int32_t)(int16_t)r->d[3];                         /* 13D9C2 ext.l */
        r->d[3] += 0x0061C000u;                                                /* 13D9C4 */
        push32(m, r, r->d[3]);                       /* 13D9CA dst */
        push16(m, r, 1u);                            /* 13D9CC digits */
        push32(m, r, r->d[1]);                       /* 13D9D0 rank value */
        push32(m, r, 0x0013D9D8u);
        r->d[0] = Gfx_DrawHex(m, r->d[1], 1u, r->d[3], 0x000Au);   /* 13D9D2 */
        r->a[7] += 12u;                              /* 13D9D8 lea 0xc(SP),SP */
        (void)seg_pop32(m, r);
        movem_pop(m, r, 0x000Cu);                    /* 13D9DC */
        r->d[0] += 2u;                               /* 13D9E0 addq.l */
        r->d[3] = rd32(m, a0);                       /* 13D9E2 score */
        push16(m, r, 7u);                            /* 13D9E4 attr */
        push32(m, r, r->d[0]);                       /* 13D9E8 dst */
        push16(m, r, 8u);                            /* 13D9EA digits */
        push32(m, r, r->d[3]);                       /* 13D9EE score */
        push32(m, r, 0x0013D9F6u);
        r->d[0] = Gfx_DrawDecimal(m, r->d[3], 8u, r->d[0], 7u);    /* 13D9F0 */
        r->a[7] += 12u;                              /* 13D9F6 */
        (void)seg_pop32(m, r);
        r->d[0] += 2u;                               /* 13D9FA */
        /* 13D9FC: the zone letters, D4 of them, with a "'" cursor at 0x15/0x1B. */
        r->d[3] = 0u;                                /* moveq #0,D3 */
        r->d[4] = 0u;                                /* moveq #0,D4 */
        a1 = ROM_TBL_ZONE_CHARS;                     /* 13DA00 */
        for (;;) {                                   /* 13DA06 */
            seg_setw(&r->d[4], (uint16_t)((uint16_t)r->d[4] + 1u));
            seg_setw(&r->d[5],
                     ((int8_t)(uint8_t)r->d[4] < (int8_t)rd8(m, a0 + 0x04u))
                     ? 6u : 4u);                     /* 13DA08 cmp.b / bge */
            /* 13DA1C lea (0,A1,D3w*2),A2 */
            a2 = a1 + (uint32_t)(int32_t)(int16_t)(uint16_t)(2u * (uint16_t)r->d[3]);
            push16(m, r, (uint16_t)r->d[5]);         /* 13DA20 */
            push32(m, r, r->d[0]);                   /* 13DA22 */
            push32(m, r, a2);                        /* 13DA24 */
            push32(m, r, 0x0013DA2Cu);
            r->d[0] = Text_DrawString(m, a2, r->d[0], (uint16_t)r->d[5]);  /* 13DA26 */
            r->a[7] += 10u;                          /* 13DA2C */
            (void)seg_pop32(m, r);
            if ((uint16_t)r->d[3] == 0x15u || (uint16_t)r->d[3] == 0x1Bu) {  /* 13DA30/13DA38 */
                push16(m, r, (uint16_t)r->d[5]);     /* 13DA44 */
                push32(m, r, r->d[0]);               /* 13DA46 */
                push32(m, r, ROM_TXT_QUOTE);         /* 13DA48 */
                push32(m, r, 0x0013DA54u);
                r->d[0] = Text_DrawString(m, ROM_TXT_QUOTE, r->d[0],
                                          (uint16_t)r->d[5]);              /* 13DA4E */
                r->a[7] += 10u;                      /* 13DA54 */
                (void)seg_pop32(m, r);
                r->d[0] -= 2u;                       /* 13DA58 */
            }
            /* 13DA5A: bfextu (0x5,A0){D5:1},D5 with D5 = D4-1 -- bit D5 of the
             * zone word, MSB first; D4 <= 7 here so it is always the byte at 0x5. */
            r->d[5] = r->d[4];
            seg_setw(&r->d[5], (uint16_t)((uint16_t)r->d[5] - 1u));
            {
                uint32_t bit = (uint16_t)r->d[5];
                uint8_t b = rd8(m, a0 + 0x05u + (bit >> 3));
                r->d[5] = (uint32_t)((b >> (7u - (bit & 7u))) & 1u);
            }
            seg_setw(&r->d[3], (uint16_t)((uint16_t)r->d[3] + (uint16_t)r->d[5])); /* 13DA64 */
            seg_setw(&r->d[3], (uint16_t)((uint16_t)r->d[3] + (uint16_t)r->d[4])); /* 13DA66 */
            if ((int16_t)r->d[4] >= 7) break;                                     /* 13DA68 */
            if ((int8_t)(uint8_t)r->d[4] >= (int8_t)rd8(m, a0 + 0x04u)) break;    /* 13DA70 */
        }
        /* 13DA76: the name starts D4-9 cells before the cursor. */
        seg_setw(&r->d[4], (uint16_t)((uint16_t)r->d[4] - 9u));
        seg_setw(&r->d[4], (uint16_t)((uint16_t)r->d[4] + (uint16_t)r->d[4]));
        r->d[4] = (uint32_t)(int32_t)(int16_t)r->d[4];        /* 13DA7C ext.l */
        r->d[0] -= r->d[4];                                   /* 13DA7E */
        push16(m, r, 7u);                                     /* 13DA80 */
        push32(m, r, r->d[0]);                                /* 13DA84 */
        push32(m, r, a0 + 0x06u);                             /* 13DA86 pea (6,A0) */
        push32(m, r, 0x0013DA90u);
        (void)Text_DrawString(m, a0 + 0x06u, r->d[0], 7u);    /* 13DA8A name */
        r->a[7] += 10u;                                       /* 13DA90 */
        (void)seg_pop32(m, r);
        a0 += 0x0Cu;                                          /* 13DA94 */
        seg_setw(&r->d[2], (uint16_t)((uint16_t)r->d[2] + 2u));   /* 13DA98 */
        seg_setw(&r->d[1], (uint16_t)((uint16_t)r->d[1] + 1u));   /* 13DA9A */
        if ((int16_t)r->d[1] > 5) break;                          /* 13DA9C */
    }
    r->a[1] = a1;                                    /* A1 leaks 0x13D5C8 */
    r->a[2] = a2;                                    /* A2 leaks the last cell */
    movem_pop(m, r, 0x010Fu);                        /* 13DAA4 */
}

/* 13D718..13D746: one frame of the entry screen -- the panel and every
 * qualifying player's row. Leaves D1.b = rows drawn (13D726 sf.b / 13D740
 * addq.b), D2 = the last player's record index, A0 past the second player. */
static void fx_frame_13D718(const f3mem_t *m, f3regs_t *r)
{
    push32(m, r, 0x0013D71Cu);                       /* 13D718 bsr 0x13D94C */
    fx_13D94C(m, r);
    (void)seg_pop32(m, r);
    r->a[0] = 0x0041B190u;                           /* 13D71C */
    seg_setw(&r->d[0], 1u);                          /* 13D722 */
    r->d[1] &= 0xFFFFFF00u;                          /* 13D726 sf.b D1 */
    for (;;) {                                       /* 13D728 */
        r->a[1] = 0x00406CD4u;
        r->d[2] = rd32(m, r->a[0] + 0x48u);          /* 13D72E */
        if ((int32_t)r->d[2] < 0)                    /* 13D732 bmi */
            goto next_player;
        r->d[2] = (uint32_t)(uint16_t)r->d[2] * 0x0Cu;                        /* 13D736 mulu.w */
        r->a[1] = 0x00406CD4u +
                  (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[2];              /* 13D73A adda.w */
        push32(m, r, 0x0013D740u);                   /* 13D73C bsr 0x13D7A0 */
        fx_13D7A0(m, r);
        (void)seg_pop32(m, r);
        r->d[1] = (r->d[1] & 0xFFFFFF00u) |
                  (uint8_t)((uint8_t)r->d[1] + 1u);  /* 13D740 addq.b */
next_player:
        r->a[0] += 0x8Au;                            /* 13D742 */
        seg_setw(&r->d[0], (uint16_t)((uint16_t)r->d[0] - 1u));   /* 13D746 dbf */
        if ((uint16_t)r->d[0] == 0xFFFFu)
            break;
    }
}

/* ---- segments ----------------------------------------------------------- */

uint32_t Seg_13D69A(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x0013D69Au:
        movem_push(m, r, 0x070Fu);                   /* 13D69A {D0-D3 A0-A2} */
        r->a[0] = 0x0041B190u;                       /* 13D69E */
        seg_setw(&r->d[2], 1u);                      /* 13D6A4 */
        for (;;) {                                   /* 13D6A8: insert both scores */
            wr16(m, r->a[0] + 0x06u, 0u);
            wr16(m, r->a[0] + 0x3Au, 0u);
            wr16(m, r->a[0] + 0x3Eu, 0u);
            push32(m, r, 0x0013D6B6u);               /* 13D6B4 bsr 0x13D638 */
            fx_13D638(m, r);
            (void)seg_pop32(m, r);
            if ((int16_t)r->d[1] >= 0) {             /* 13D6B6 tst.w / blt */
                r->d[0] = rd32(m, r->a[0] + 0x22u);  /* 13D6BC */
                seg_setw(&r->d[0], 0u);              /* 13D6C0 clr.w */
                wr32(m, r->a[1] + 0x06u, r->d[0]);   /* 13D6C2 */
            }
            r->a[0] += 0x8Au;                        /* 13D6C6 */
            seg_setw(&r->d[2], (uint16_t)((uint16_t)r->d[2] - 1u));   /* 13D6CA dbf */
            if ((uint16_t)r->d[2] == 0xFFFFu)
                break;
        }
        r->a[0] = 0x0041B190u;                       /* 13D6CE */
        seg_setw(&r->d[0], 1u);                      /* 13D6D4 */
        for (;;) {                                   /* 13D6D8: find each record */
            r->a[1] = 0x00406CD4u;
            r->d[1] = 0u;                            /* 13D6DE moveq #0,D1 */
            seg_setw(&r->d[2], rd16(m, r->a[0] + 0x22u));    /* 13D6E0 */
            for (;;) {                               /* 13D6E4 */
                if ((uint16_t)rd16(m, r->a[1] + 0x06u) == (uint16_t)r->d[2])
                    break;                           /* 13D6E8 beq 13D6FE */
                r->a[1] += 0x0Cu;                    /* 13D6EC */
                seg_setw(&r->d[1], (uint16_t)((uint16_t)r->d[1] + 1u));   /* 13D6F0 */
                if ((int16_t)r->d[1] < 5)            /* 13D6F2 blt */
                    continue;
                r->d[1] = 0xFFFFFFFFu;               /* 13D6F8 moveq #-1 */
                goto store_index;
            }
            wr32(m, r->a[1] + 0x06u, 0x2E2E2E00u);   /* 13D6FE "..." */
store_index:
            wr32(m, r->a[0] + 0x48u, r->d[1]);       /* 13D706 */
            r->a[0] += 0x8Au;                        /* 13D70A */
            seg_setw(&r->d[0], (uint16_t)((uint16_t)r->d[0] - 1u));   /* 13D70E dbf */
            if ((uint16_t)r->d[0] == 0xFFFFu)
                break;
        }
        r->d[7] = 0x00000708u;                       /* 13D712 */
        fx_frame_13D718(m, r);                       /* 13D718 */
        return 0x0013D74Au;                          /* 13D74A trap #5 */

    case 0x0013D74Cu:
        if ((r->d[1] & 0xFFu) == 0u)                 /* 13D74C tst.b / beq 13D790 */
            return SEG_LEAVE;                        /* no row drawn: rts at 13D798 */
        push16(m, r, 6u);                            /* 13D752 */
        push32(m, r, ROM_TXT_COUNTDOWN_DST);         /* 13D756 */
        push32(m, r, ROM_TXT_COUNTDOWN);             /* 13D75C */
        push32(m, r, 0x0013D768u);
        r->d[0] = Text_DrawString(m, ROM_TXT_COUNTDOWN, ROM_TXT_COUNTDOWN_DST,
                                  6u);               /* 13D762 */
        r->a[7] += 10u;                              /* 13D768 lea 0xa(SP),SP */
        (void)seg_pop32(m, r);
        r->d[1] = r->d[7];                           /* 13D76C move.l D7,D1 */
        /* 13D76E divu.l #0x3c,D1:D1 -- extension bit 10 clear, so the 32/32
         * form: D1 = D7 / 60 (the captures draw 30 at D7 = 0x708, 29 at 0x707). */
        r->d[1] = r->d[1] / 0x0000003Cu;
        push16(m, r, 7u);                            /* 13D776 */
        push32(m, r, r->d[0]);                       /* 13D77A */
        push16(m, r, 2u);                            /* 13D77C */
        push32(m, r, r->d[1]);                       /* 13D780 */
        push32(m, r, 0x0013D788u);
        r->d[0] = Gfx_DrawDecimal(m, r->d[1], 2u, r->d[0], 7u);   /* 13D782 */
        r->a[7] += 12u;                              /* 13D788 lea 0xc(SP),SP */
        (void)seg_pop32(m, r);
        seg_setw(&r->d[7], (uint16_t)((uint16_t)r->d[7] - 1u));   /* 13D78C dbf */
        if ((uint16_t)r->d[7] == 0xFFFFu)
            return SEG_LEAVE;                        /* 13D790: rts at 13D798 */
        fx_frame_13D718(m, r);
        return 0x0013D74Au;                          /* 13D74A trap #5 */
    }
    return SEG_LEAVE;
}
