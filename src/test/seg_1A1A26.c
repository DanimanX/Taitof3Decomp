/* ROM 0x1A1A26 Task_TestModeMenu, the test-mode main menu, in segments
 * (src/kernel/seg.h). It yields with jsr 0x001F62 (Sys_Yield call gate) at
 * 0x1A1A68 instead of a TRAP: a segment stops AT that jsr and returns
 * 0x1A1A68; the next one resumes at 0x1A1A6E. The routine has no normal
 * return: the last frame leaves through 0x1A1B96 (below), and the bare rts at
 * 0x1A1BA4 is not reachable from this code (0x1A1BA0 bra.w jumps back to the
 * yield).
 *
 * Entry 0x1A1A26 saves D2-D6/A2-A4, clears the layers and counts the menu
 * entries: the table at 0x1A2350 holds 36-byte records {fn.l, label[32]}, and
 * the count stops at the first null fn (7 entries, the ".w*36" walk of
 * 0x1A1A58). D0 then holds count*36 = 0xFC, the value the yield returns with.
 *
 * Each 0x1A1A6E frame walks the selection D3 with the P2 switch byte 0x402225
 * (bit 1 down, bit 0 up, wrapping through the count), draws the fixed lines -
 * "TESTMODE MAIN MENU" at 0x61C196, the version line via jsr 0x1A1BA6 at
 * 0x61CD10 ("%s  VER %d.%d %s" with "DARIUS GAIDEN", the two bytes at
 * 0x1A1C34/35 = 2.5 and the one-character region string at 0x1A1C68 +
 * 4*(word at 0x1FFFFE), which is "O" for the dump's version 3) and the build
 * date at 0x61CE18 ("%4.4d/%2.2d/%2.2d %2.2d:%2.2d" from the five words at
 * 0x1A24C2 = 1994/09/19 11:02) - then one row per entry at 0x61C316 + i*0x100:
 * the label in colour 7, and for the selected row the label in colour 8
 * followed by the ">" marker at 0x1A24C0 over the same address in colour 7.
 *
 * All the draws go through jsr (A4) = 0x005666 and are pushed in place; the
 * lea 0x42(SP) at 0x1A1B0E drops the three fixed lines (10 + 26 + 30 bytes),
 * and each row drops its own 10 bytes with lea 0xA(SP). Bit 4 of 0x402225
 * leaves the frame: the selected fn at 0x1A2350 + D3*36 is called, then
 * 0x005A3A (Gfx_LoadCharFont), then the frame yields again. That callee is a
 * whole test-mode screen (a segment group of its own, which yields inside),
 * so the exit path returns SEG_LEAVE before any side effect; every captured
 * frame has bit 4 set and never takes it. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */

#define SEG_YIELD   0x001A1A68u     /* jsr 0x001F62, the yield */
#define MENU_TABLE  0x001A2350u     /* {fn.l, "  label"} x 7, then a null fn */
#define MENU_STRIDE 36u
#define P2_SWITCH   0x00402225u

/* 0x1A1A26 movem.l d2-d6/a2-a4,-(SP): D2 lowest in memory, A4 highest */
static void save_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7] - 32u;
    r->a[7] = sp;
    for (unsigned i = 0; i < 5u; ++i)
        wr32(m, sp + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 3u; ++i)
        wr32(m, sp + 20u + 4u * i, r->a[2u + i]);
}

/* jsr (A4) -> 0x5666: the three arguments pushed in place (they stay until the
 * caller's lea), the jsr's return PC, then link.w a6,#0 and
 * movem.l d7/a0-a1,-(SP). Only D0 (the end of the drawn line) survives. */
static void draw_text(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                      uint32_t dst, uint32_t src, uint16_t col)
{
    uint32_t sp = r->a[7] - 10u;
    r->a[7] = sp;
    wr32(m, sp, src);
    wr32(m, sp + 4u, dst);
    wr16(m, sp + 8u, col);
    wr32(m, sp - 4u, jsr_pc + 2u);
    wr32(m, sp - 8u, r->a[6]);
    wr32(m, sp - 20u, r->d[7]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 12u, r->a[1]);
    r->d[0] = Text_DrawString(m, src, dst, col);
    r->a[7] = sp;                       /* the callee's rts pops its return PC */
}

/* jsr 0x1A1BA6: link.w a6,#-0x40, format the argument list into the 0x40-byte
 * buffer with 0x005C62, draw it with 0x005666, then lea 0x16(SP)/unlk/rts.
 * The caller pushes the format arguments first (last argument first), then
 * fmt.l, col.w and dst.l, so dst sits at (8,A6), the colour at (0xC,A6), the
 * format at (0xE,A6) and the argument list at (0x12,A6) - the callee's
 * `pea 0x12(a6)` pushes that address, there is no separate pointer slot. The
 * translated callee returns with SP at dst and leaves the format scratch in
 * D0/D1 (D1 = 0xFFFF after these two calls). */
static void draw_format(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                        uint32_t dst, uint16_t col, uint32_t fmt)
{
    r->a[7] -= 4u;                      /* fmt.l */
    wr32(m, r->a[7], fmt);
    r->a[7] -= 2u;                      /* col.w */
    wr16(m, r->a[7], col);
    r->a[7] -= 4u;                      /* dst.l */
    wr32(m, r->a[7], dst);
    r->a[7] -= 4u;                      /* jsr 0x1A1BA6 */
    wr32(m, r->a[7], jsr_pc + 2u);
    Fn_1A1BA6(m, r);                    /* leaves SP at dst */
}

/* 0x1A1B16..0x1A1B7A: one row of the menu at 0x61C316 + i*0x100. The selected
 * row draws its label in colour 8 and then the ">" marker over the same
 * address in colour 7; the others draw the label in colour 7. */
static void draw_row(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc, uint32_t i,
                     int selected)
{
    uint32_t dst = 0x0061C316u + (uint32_t)(uint16_t)((i + i) << 7);
    uint32_t label = MENU_TABLE + 4u + i * MENU_STRIDE;

    if (selected) {
        draw_text(m, r, jsr_pc, dst, label, 0x8u);      /* 0x1A1B38 */
        r->a[7] += 0xAu;                                /* 0x1A1B3A */
        draw_text(m, r, jsr_pc, dst, 0x001A24C0u, 0x7u); /* 0x1A1B74 '>' */
        r->a[7] += 0xAu;                                /* 0x1A1B76 */
    } else {
        draw_text(m, r, jsr_pc, dst, label, 0x7u);      /* 0x1A1B74 */
        r->a[7] += 0xAu;                                /* 0x1A1B76 */
    }
}

uint32_t Seg_1A1A26(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A1A26u:
        save_test_regs(m, r);
        r->a[2] = 0x0061C000u;      /* text RAM base */
        r->a[3] = 0x001A24C2u;      /* the five build-date words */
        r->a[4] = 0x00005666u;      /* jsr (A4) draws text */
        r->d[5] = MENU_TABLE;
        r->d[6] = P2_SWITCH;
        r->d[3] = 0u;
        Gfx_ClearAllLayers(m);      /* 0x1A1A4A */
        /* 0x1A1A50: count the entries, D0 = count*36, A0 = the table base */
        r->d[4] = 0u;
        r->d[2] = 0u;
        while (rd32(m, MENU_TABLE + r->d[4] * MENU_STRIDE) != 0u)
            r->d[4] += 1u;          /* 0x1A1A56 addq.l #1,D4 */
        r->a[0] = MENU_TABLE;       /* 0x1A1A60 movea.l D5,A0 */
        r->d[0] = r->d[4] * MENU_STRIDE;    /* lsl #3 / add / lsl #2 */
        return SEG_YIELD;           /* 0x1A1A68 jsr 0x001F62 */

    case 0x1A1A6Eu:
        /* 0x1A1A6E: D3 walks the selection; D0.b is the tested bit */
        r->a[0] = r->d[6];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, r->d[6]) & 0x2u);
        if (!(r->d[0] & 0xFFu)) {
            r->d[3] += 1u;                          /* 0x1A1A78 */
            if ((int32_t)r->d[3] >= (int32_t)r->d[4])   /* 0x1A1A7C blt */
                r->d[3] = 0u;                       /* 0x1A1A7E */
        }
        r->a[0] = r->d[6];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, r->d[6]) & 0x1u);
        if (!(r->d[0] & 0xFFu)) {
            r->d[3] -= 1u;                          /* 0x1A1A8A */
            if ((int32_t)r->d[3] < 0)               /* 0x1A1A8C bpl */
                r->d[3] = r->d[4] - 1u;             /* 0x1A1A8E moveq #-1 */
        }
        /* 0x1A1A92: "TESTMODE MAIN MENU" at 0x61C196 */
        draw_text(m, r, 0x001A1AA0u, 0x0061C196u, 0x001A247Eu, 0xAu);
        /* 0x1A1AA2: the version line; the region is the one-character string
         * at 0x1A1C68 + 4*version (pea pushes that address). */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x001FFFFEu);
        {
            uint32_t region = 0x001A1C68u +
                              (uint32_t)(4 * (int32_t)(int16_t)(uint16_t)r->d[0]);
            r->a[7] -= 4u;                  /* 0x1A1AA8 pea (region) */
            wr32(m, r->a[7], region);
            r->d[0] = rd8(m, 0x001A1C35u);  /* 0x1A1AB0: major = 5 */
            r->a[7] -= 4u;
            wr32(m, r->a[7], r->d[0]);
            r->d[0] = rd8(m, 0x001A1C34u);  /* 0x1A1ABA: minor = 2 */
            r->a[7] -= 4u;
            wr32(m, r->a[7], r->d[0]);
            r->a[7] -= 4u;                  /* 0x1A1AC4 pea "DARIUS GAIDEN" */
            wr32(m, r->a[7], 0x001A2470u);
            draw_format(m, r, 0x001A1AD8u, 0x0061CD10u, 0x7u, 0x001A2491u);
        }
        /* 0x1A1ADE: the build date, five words from 0x1A24C2 */
        for (unsigned k = 0; k < 5u; ++k) {
            uint32_t off = 8u - 2u * k;     /* movea.w 8/6/4/2/0(A3),A0 */
            r->a[0] = (uint32_t)(int32_t)(int16_t)rd16(m, r->a[3] + off);
            r->a[7] -= 4u;                  /* move.l A0,-(SP) */
            wr32(m, r->a[7], r->a[0]);
        }
        draw_format(m, r, 0x001A1B08u, 0x0061CE18u, 0x7u, 0x001A24A2u);
        r->a[7] += 0x42u;                   /* 0x1A1B0E lea 0x42(SP) */

        /* 0x1A1B12: one row per entry */
        for (r->d[2] = 0u; (int32_t)r->d[2] < (int32_t)r->d[4]; r->d[2] += 1u) {
            int sel = r->d[2] == r->d[3];
            draw_row(m, r, sel ? 0x001A1B1Au : 0x001A1B56u, r->d[2], sel);
        }
        /* 0x1A1B80: bit 4 of the P2 switch byte leaves the menu */
        r->a[0] = r->d[6];
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, r->d[6]) & 0x10u);
        if (!(r->d[0] & 0xFFu)) {
            /* 0x1A1B8C: D0 = 0x1A2350 + D3*36, jsr ([D0]) runs the selected
             * screen (a segment group of its own, which yields inside), then
             * 0x005A3A loads the character font. Neither is translatable
             * inside this segment, so the path stops here. */
            return SEG_LEAVE;
        }
        return SEG_YIELD;           /* 0x1A1B88 bne.w 0x1A1A68 */
    }
    return SEG_LEAVE;
}
