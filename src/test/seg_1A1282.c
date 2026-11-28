/* ROM 0x1A1282 test-mode screen "DIFFICULTY" (menu entry 4 of the
 * Task_TestModeMenu table at 0x1A2350), in segments (src/kernel/seg.h). It
 * yields with jsr 0x001F62 (Sys_Yield call gate) at 0x1A12DC (frame loop),
 * 0x1A176A (the exit flag armed: layers cleared) and 0x1A17A8 (the confirm
 * screen); a segment stops AT that jsr and returns its PC, and the next one
 * resumes 6 bytes later (0x1A12E2, 0x1A1770, 0x1A17AE). The screen ends at the
 * RTS at 0x1A1854.
 *
 * Entry: link.w a6,#-0xA, save D2-D7/A2-A4, A2 = the 0x80-byte settings block
 * 0x406C54, A3 = the text base 0x61C000, A4 = 0x5666, D6 = the P1 switch byte
 * 0x402224, D7 = the shared editor 0x19FFF0, -0x6(A6) = 9 rows, D3 = the
 * selection 0, -0x2(A6) = the exit flag byte 0. It clears the layers, reloads
 * the settings through 0x4D06, loads the character block (0x1A006C) and draws
 * the title "DIFFICULTY" at 0x61C09C in colour 0xA, then falls into the frame
 * loop test at 0x1A1756, which finds the flag clear and yields.
 *
 * Each 0x1A12E2 frame walks the selection D3 with the P2 switch byte 0x402225
 * (bit 1 down, bit 0 up, wrapping through the nine rows) and draws one row per
 * iteration at 0x61C000 + 0x492 + row*0x80: row 0 is "DIFFICULTY   :" with the
 * 4-value name from the word/jmp table at 0x1A13A0 (EASY/NORMAL/HARD/SUPER
 * HARD), rows 1-7 are the seven ROUND rows (label, a 15-slot bar drawn by
 * 0x1A0086 and the value formatted by 0x1A1BA6 as "(%2.2d)"), row 8 is "EXIT".
 * The row dispatch is `move.l D4,D0 ; subq.l #8,D0 ; bhi 0x1A174C`: SUBQ sets
 * the borrow, so BHI is taken only for a row above 8 and every row 0..8 falls
 * through to the word table at 0x1A1330+2*D4 (D0.w = D4-8 is the negative word
 * index that lands there). D2 is the colour: 8 for the selected row, 7 for the
 * rest. The selected row first runs its settings field through the editor at
 * 0x19FFF0 (row 0: bits {14:2} of the word at A2+0xA in 0..3; rows 1-7: the
 * four-bit fields {0:4}/{4:4}/{8:4}/{12:4} of the word at A2+0x14 and
 * {0:4}/{4:4}/{8:4} of the word at A2+0x16, all in 0..0xF) and writes the
 * result back with bfins. Bit 4 of 0x402225 on the selected EXIT row arms the
 * exit flag (0x1A1746). When it is armed the frame clears D3 and the flag,
 * clears the layers and yields.
 *
 * 0x1A1770 then draws the confirm page - "DIFFICULTY" at 0x61C09C, "SAVE
 * MODIFIED DATA" at 0x61C316 and "ARE YOU SURE?" at 0x61C41C, all colour 7
 * except the title - and falls to 0x1A182A, which yields (the flag is clear
 * again). 0x1A17AE runs the YES/NO editor (range 0..1) per frame, draws
 * "YES       NO" at 0x61C79C and paints the live choice with the colour-8 arm
 * of the word/jmp table at 0x1A17F0 (D3 == 0 -> YES at 0x61C79C, D3 == 1 ->
 * NO at 0x61C7B0); bit 4 of 0x402225 arms the flag again.
 *
 * Once the flag is set: D3 == 0 writes the block back through 0x4DEE, which
 * cannot succeed without the serial device and enters the reset loop
 * (SEG_LEAVE); D3 == 1 reloads the settings through 0x4D06. Both clear the
 * layers, restore the saved registers and return through the RTS. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */
#include "../auto/fn_auto.h"        /* Fn_1A006C, Fn_19FFF0, Fn_1A0086, Fn_1A1BA6 */
#include "../auto/eeprom_block.h"   /* settings_apply_4D06 (0x004D06),
                                     * settings_refresh_4DEE (0x004DEE) */

#define SEG_YIELD  0x001A12DCu      /* jsr 0x001F62, the frame loop */
#define SEG_YIELD2 0x001A176Au      /* jsr 0x001F62, the confirm page */
#define SEG_YIELD3 0x001A17A8u      /* jsr 0x001F62, the YES/NO editor */
#define SETTINGS   0x00406C54u      /* the 0x80-byte settings block */
#define P1_SWITCH  0x00402224u
#define P2_SWITCH  0x00402225u
#define ROWS_ADDR  0x001A1330u      /* word table: one offset per row */
#define ROUND_DST  0x0061C592u      /* row 1 label; each row adds 0x80 */
#define ROUND_STRIDE 0x80u

/* 0x1A1282 link.w a6,#-0xA ; 0x1A1286 movem.l d2-d7/a2-a4,-(SP) */
static void save_frame(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[6]);
    r->a[6] = r->a[7];
    r->a[7] -= 0xAu;
    r->a[7] -= 36u;
    for (unsigned i = 0; i < 6u; ++i)
        wr32(m, r->a[7] + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 3u; ++i)
        wr32(m, r->a[7] + 24u + 4u * i, r->a[2u + i]);
}

/* 0x1A184E movem.l (SP)+,d2-d7/a2-a4 ; 0x1A1852 unlk a6 ; 0x1A1854 rts */
static uint32_t restore_frame(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 0; i < 6u; ++i)
        r->d[2u + i] = rd32(m, r->a[7] + 4u * i);
    for (unsigned i = 0; i < 3u; ++i)
        r->a[2u + i] = rd32(m, r->a[7] + 24u + 4u * i);
    r->a[7] += 36u;
    r->a[7] = r->a[6];                      /* unlk a6 */
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return Seg_Run(m, r, seg_pop32(m, r));  /* rts */
}

/* jsr (A4) -> 0x5666: the three arguments stay on the stack until the
 * caller's lea, the callee's return PC, its link word and its
 * movem.l d7/a0-a1 block are written below them, and only D0 (the end of the
 * drawn line) survives. Every call site here is the 2-byte jsr (A4). */
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

/* jsr (A0) -> 0x19FFF0, the shared value editor (see seg_1A1944.c). The five
 * argument words, the return PC and the callee's movem.l d3-d5 block stay for
 * the caller's lea 0xA(SP); D0.w/D1.w come back as the new value. */
static void edit_value(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                       uint16_t in, uint16_t cur, uint16_t lo, uint16_t hi,
                       uint16_t step)
{
    uint32_t sp = r->a[7] - 10u;        /* 5 argument words */
    r->a[7] = sp;
    wr16(m, sp, in);                    /* move.w D0w,-(a7) */
    wr16(m, sp + 2u, cur);              /* bfextu/move.w D0w,-(a7) */
    wr16(m, sp + 4u, lo);               /* clr.w -(a7) */
    wr16(m, sp + 6u, hi);               /* move.w #<hi>,-(a7) */
    wr16(m, sp + 8u, step);             /* move.w #<step>,-(a7) */
    wr32(m, sp - 4u, jsr_pc + 2u);
    wr32(m, sp - 8u, r->d[5]);          /* 0x19FFF0 movem.l d3-d5,-(SP) */
    wr32(m, sp - 12u, r->d[4]);
    wr32(m, sp - 16u, r->d[3]);

    uint16_t d3;
    r->d[5] = (r->d[5] & 0xFFFF0000u) | in;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | cur;
    r->a[0] = 0x0040CBD0u;

    r->d[0] = (r->d[0] & 0xFFFF0000u) | in;         /* 0x1A0002 move.w D5w,D0w */
    if ((r->d[0] & 4u) != 0u) {                     /* andi.w #4,D0w */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | in;     /* 0x1A000A */
        if ((r->d[0] & 8u) != 0u) {                 /* andi.w #8,D0w */
            wr16(m, 0x0040CBD0u, 0u);               /* 0x1A0062 clr.w (A0) */
            goto done;
        }
    }
    if (rd16(m, 0x0040CBD0u) != 0u && rd16(m, 0x0040CBD0u) <= 0x14u)
        goto bump;                                  /* 0x1A0016 ble 0x1A0058 */
    d3 = rd16(m, 0x0040CBD0u) >= 0x78u ? 0xAu : 1u; /* 0x1A001C cmpi.w #0x78 */
    for (uint32_t d4 = 0;; ) {
        if ((r->d[5] & 8u) != 0u)
            r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)((r->d[1] - step) & 0xFFFFu);
        else
            r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)((r->d[1] + step) & 0xFFFFu);
        if ((int16_t)r->d[1] > (int16_t)hi)         /* 0x1A003E ble */
            r->d[1] = (r->d[1] & 0xFFFF0000u) | hi;
        if ((int16_t)r->d[1] < (int16_t)lo)         /* 0x1A0048 bge */
            r->d[1] = (r->d[1] & 0xFFFF0000u) | lo;
        d4 += 1u;                                   /* 0x1A0052 addq.l #1,D4 */
        if ((int32_t)d4 >= (int32_t)d3)             /* 0x1A0054 cmp.l D3,D4 */
            break;
    }
bump:
    if (rd16(m, 0x0040CBD0u) < 0x78u)
        wr16(m, 0x0040CBD0u, (uint16_t)(rd16(m, 0x0040CBD0u) + 1u));
done:
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)r->d[1];
    r->d[5] = rd32(m, sp - 8u);         /* movem.l (SP)+,d3-d5 */
    r->d[4] = rd32(m, sp - 12u);
    r->d[3] = rd32(m, sp - 16u);
    r->a[7] = sp;                       /* rts pops its return PC; the five
                                         * argument words stay for the caller's
                                         * lea 0xA(SP) */
}

/* jsr 0x1A0086(pc), the 15-slot bar (src/auto/fn_1A0086.c): the arguments
 * dst.l, value.l, slots.l and colour.w are pushed last-first, then the 4-byte
 * jsr's return PC. The callee pops only the return address. */
static void draw_bar(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                     uint32_t dst, uint32_t value, uint32_t slots, uint16_t col)
{
    r->a[7] -= 2u;                      /* move.w D2w,-(a7) */
    wr16(m, r->a[7], col);
    r->a[7] -= 4u;                      /* pea (0xf).w */
    wr32(m, r->a[7], slots);
    r->a[7] -= 4u;                      /* move.l D0,-(a7) */
    wr32(m, r->a[7], value);
    r->a[7] -= 4u;                      /* pea (dst,A3) */
    wr32(m, r->a[7], dst);
    r->a[7] -= 4u;                      /* jsr 0x1A0086(pc) */
    wr32(m, r->a[7], jsr_pc + 4u);
    Fn_1A0086(m, r);                    /* leaves SP at dst */
}

/* jsr 0x1A1BA6.l with one long argument: the caller pushes the argument
 * first, then fmt/col/dst and the 6-byte jsr's return PC (see seg_1A1A26). */
static void draw_format(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                        uint32_t dst, uint32_t fmt, uint16_t col, uint32_t value)
{
    r->a[7] -= 4u;                      /* move.l D0,-(a7) */
    wr32(m, r->a[7], value);
    r->a[7] -= 4u;                      /* pea fmt */
    wr32(m, r->a[7], fmt);
    r->a[7] -= 2u;                      /* move.w D2w,-(a7) */
    wr16(m, r->a[7], col);
    r->a[7] -= 4u;                      /* pea (dst,A3) */
    wr32(m, r->a[7], dst);
    r->a[7] -= 4u;                      /* jsr 0x001A1BA6 */
    wr32(m, r->a[7], jsr_pc + 6u);
    Fn_1A1BA6(m, r);                    /* leaves SP at dst */
}

/* 0x1A1342: row 0, the difficulty itself. The selected row edits the two bits
 * {14:2} of the word at A2+0xA (0..3, step 1), draws "DIFFICULTY   :" at
 * 0x61C492 in colour D2 and then the name for the resulting value at that
 * cursor, leaving it in D5. */
static void row_difficulty(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[4] == r->d[3]) {                       /* 0x1A1342 cmp.l D3,D4 */
        uint16_t cur = (uint16_t)ee_bfget(m, SETTINGS + 0xAu, 14u, 2u);
        edit_value(m, r, 0x001A1362u, (uint16_t)rd8(m, P1_SWITCH),
                   cur, 0u, 3u, 1u);
        r->a[7] += 0xAu;                            /* 0x1A1364 lea 0xA(SP) */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];  /* ext.l */
        ee_bfput(m, SETTINGS + 0xAu, 14u, 2u, r->d[0]);  /* 0x1A136A bfins */
    }
    draw_text(m, r, 0x001A137Cu, 0x0061C492u, 0x001A21A8u, (uint16_t)r->d[2]);
    r->a[7] += 0xAu;                                /* 0x1A137E lea 0xA(SP) */
    r->d[5] = r->d[0];                              /* 0x1A1382 move.l D0,D5 */

    /* 0x1A1384: `subq.l #3,D0 ; bhi 0x1A174C` skips values above 3 (a borrow
     * or a zero leaves BHI clear), so 0..3 index the word table at 0x1A13A0
     * through the negative word index D0.w = value-3: entry value-3. The arms
     * draw the name at the cursor D5 left by the label and set D5 to its end. */
    r->d[0] = ee_bfget(m, SETTINGS + 0xAu, 14u, 2u);
    {
        const uint32_t value = r->d[0];
        r->d[0] -= 3u;                              /* 0x1A138A subq.l #3,D0 */
        if (value <= 3u) {                          /* 0x1A138C bhi not taken */
            uint32_t ea = 0x001A13A0u +
                          2u * (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            uint16_t off = rd16(m, ea);
            uint32_t str = 0x001A21D8u, jsr = 0x001A13EEu;  /* SUPER HARD */
            seg_setw(&r->d[0], off);                /* 0x1A1390 move.w (ea),D0 */
            switch (0x001A139Au + (uint32_t)(int16_t)off) {
            case 0x1A13A2u: str = 0x001A21B7u; jsr = 0x001A13ACu; break; /* EASY */
            case 0x1A13B8u: str = 0x001A21C2u; jsr = 0x001A13C2u; break; /* NORMAL */
            case 0x1A13CEu: str = 0x001A21CDu; jsr = 0x001A13D8u; break; /* HARD */
            case 0x1A13E4u: break;                                      /* SUPER HARD */
            }
            draw_text(m, r, jsr, r->d[5], str, (uint16_t)r->d[2]);
            r->a[7] += 0xAu;                        /* lea 0xA(SP) */
            r->d[5] = r->d[0];                      /* 0x1A13B2 move.l D0,D5 */
        }
    }
}

/* One ROUND row (0x1A13FA, 0x1A146E, 0x1A14E2, 0x1A1556, 0x1A15CA, 0x1A163E,
 * 0x1A16B2: the same code every 0x74 bytes). The selected row edits its
 * four-bit field in 0..0xF, then the row draws its label at `dst`, the 15-slot
 * bar four cells on (0x1A0086) and the value formatted as "(%2.2d)" at dst+0x20
 * (0x1A1BA6). All three argument blocks plus the editor's are dropped at once
 * by the closing lea 0x26(SP). */
static void round_row(const f3mem_t *m, f3regs_t *r, uint32_t base,
                      uint32_t field, unsigned bit, uint32_t src, uint32_t dst)
{
    if (r->d[4] == r->d[3]) {                       /* cmp.l D3,D4 */
        uint16_t cur = (uint16_t)ee_bfget(m, field, bit, 4u);
        edit_value(m, r, base + 0x20u, (uint16_t)rd8(m, P1_SWITCH),
                   cur, 0u, 0xFu, 1u);
        r->a[7] += 0xAu;                            /* lea 0xA(SP) */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];  /* ext.l */
        ee_bfput(m, field, bit, 4u, r->d[0]);       /* bfins */
    }
    draw_text(m, r, base + 0x3Au, dst, src, (uint16_t)r->d[2]);
    draw_bar(m, r, base + 0x4Eu, dst + 0x1Cu,
             ee_bfget(m, field, bit, 4u), 0xFu, (uint16_t)r->d[2]);
    draw_format(m, r, base + 0x66u, dst + 0x20u, 0x001A22DCu,
                (uint16_t)r->d[2], ee_bfget(m, field, bit, 4u));
    r->a[7] += 0x26u;                               /* lea 0x26(SP) */
}

/* 0x1A1724: row 8, EXIT. Bit 4 of the P2 switch byte on the selected row arms
 * the screen's exit flag. */
static void row_exit(const f3mem_t *m, f3regs_t *r)
{
    draw_text(m, r, 0x001A1730u, 0x0061CA12u, 0x001A20C7u, (uint16_t)r->d[2]);
    r->a[7] += 0xAu;                                /* 0x1A1732 lea 0xA(SP) */
    if (r->d[4] != r->d[3])                         /* 0x1A1736 cmp / bne */
        return;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
    if (r->d[0] & 0xFFu)                            /* 0x1A1744 bne */
        return;
    wr8(m, r->a[6] - 2u, 1u);                       /* 0x1A1746 move.b #1 */
}

/* 0x1A1314..0x1A1752: one pass of the row loop - the colour D2, the dispatch
 * through 0x1A1330 and the increment/comparison at 0x1A174C. */
static void row_loop(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1A130E moveq #0,D4 ; 0x1A1310 bra.w 0x1A174E */
    r->d[4] = 0u;
    for (;;) {
        /* 0x1A174E cmp.l -0x6(A6),D4 ; 0x1A1752 blt.w 0x1A1314 */
        if ((int32_t)r->d[4] >= (int32_t)rd32(m, r->a[6] - 6u))
            break;
        /* 0x1A1314: colour 8 for the selected row, 7 for the rest */
        if (r->d[4] == r->d[3])
            r->d[2] = 8u;
        else
            r->d[2] = 7u;
        /* 0x1A131E move.l D4,D0 ; subq.l #8,D0 ; bhi.w 0x1A174C: SUBQ leaves
         * C set on a borrow and Z set at eight, so the branch is taken only
         * for a row above 8 and rows 0..8 read the word table at
         * 0x1A1340 + 2*(D4-8) = 0x1A1330 + 2*D4. */
        r->d[0] = r->d[4];
        r->d[0] -= 8u;
        if (r->d[4] <= 8u) {
            uint32_t ea = 0x001A1340u +
                          2u * (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            uint16_t off = rd16(m, ea);
            seg_setw(&r->d[0], off);                /* 0x1A1326 move.w */
            switch (ROWS_ADDR + (uint32_t)(int16_t)off) {   /* 0x1A132C jmp */
            case 0x1A1342u: row_difficulty(m, r); break;
            case 0x1A13FAu:                 /* ROUND 1 [ A ]: */
                round_row(m, r, 0x001A13FAu, SETTINGS + 0x14u, 0u,
                          0x001A22CDu, ROUND_DST);
                break;
            case 0x1A146Eu:                 /* ROUND 2 [B-C]: */
                round_row(m, r, 0x001A146Eu, SETTINGS + 0x14u, 4u,
                          0x001A22E4u, ROUND_DST + 0x080u);
                break;
            case 0x1A14E2u:                 /* ROUND 3 [D-F]: */
                round_row(m, r, 0x001A14E2u, SETTINGS + 0x14u, 8u,
                          0x001A22F3u, ROUND_DST + 0x100u);
                break;
            case 0x1A1556u:                 /* ROUND 4 [G-J]: */
                round_row(m, r, 0x001A1556u, SETTINGS + 0x14u, 12u,
                          0x001A2302u, ROUND_DST + 0x180u);
                break;
            case 0x1A15CAu:                 /* ROUND 5 [K-O]: */
                round_row(m, r, 0x001A15CAu, SETTINGS + 0x16u, 0u,
                          0x001A2311u, ROUND_DST + 0x200u);
                break;
            case 0x1A163Eu:                 /* ROUND 6 [P-U]: */
                round_row(m, r, 0x001A163Eu, SETTINGS + 0x16u, 4u,
                          0x001A2320u, ROUND_DST + 0x280u);
                break;
            case 0x1A16B2u:                 /* ROUND 7 [V-Z]: */
                round_row(m, r, 0x001A16B2u, SETTINGS + 0x16u, 8u,
                          0x001A232Fu, ROUND_DST + 0x300u);
                break;
            case 0x1A1724u: row_exit(m, r); break;
            }
        }
        r->d[4] += 1u;                              /* 0x1A174C addq.l #1,D4 */
    }
}

/* 0x1A17AE..0x1A1830: one pass of the YES/NO confirm frame, plus the epilogue
 * at 0x1A1832 when the flag is set. Returns SEG_YIELD3 while the flag is
 * clear, otherwise runs the chosen arm and returns as Seg_Run. */
static uint32_t confirm_frame(const f3mem_t *m, f3regs_t *r)
{
    edit_value(m, r, 0x001A17C4u, (uint16_t)rd8(m, P1_SWITCH),
               (uint16_t)r->d[3], 0u, 1u, 1u);
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];  /* 0x1A17C6 ext.l */
    r->d[3] = r->d[0];                              /* 0x1A17C8 move.l D0,D3 */
    draw_text(m, r, 0x001A17D8u, 0x0061C79Cu, 0x001A20EDu, 0x7u);  /* 0x1A17D8 */
    r->a[7] += 0x14u;                               /* 0x1A17DA lea 0x14(SP) */
    r->d[0] = r->d[3];
    r->d[0] -= 1u;                                  /* 0x1A17E0 subq.l #1,D0 */
    /* 0x1A17E2 bhi.w 0x1A1818: 0 subtracts with a borrow and 1 gives zero, so
     * only D3 == 0 and D3 == 1 reach the table at 0x1A17F0 (word index
     * D0.w = D3-1). */
    if (r->d[3] <= 1u) {
        uint32_t ea = 0x001A17F2u +
                      2u * (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
        uint16_t off = rd16(m, ea);
        seg_setw(&r->d[0], off);
        switch (0x001A17F0u + (uint32_t)(int16_t)off) {
        case 0x1A17F4u:                             /* D3 == 0: YES, colour 8 */
            draw_text(m, r, 0x001A1812u, 0x0061C79Cu, 0x001A20FAu, 0x8u);
            break;
        case 0x1A1804u:                             /* D3 == 1: NO, colour 8 */
            draw_text(m, r, 0x001A1812u, 0x0061C7B0u, 0x001A20FEu, 0x8u);
            break;
        }
        r->a[7] += 0xAu;                            /* 0x1A1814 lea 0xA(SP) */
    }
    /* 0x1A1818: bit 4 of the P2 switch byte confirms the choice */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
    if (!(r->d[0] & 0xFFu))                         /* 0x1A1822 bne */
        wr8(m, r->a[6] - 2u, 1u);                   /* 0x1A1824 */
    /* 0x1A182A tst.b -0x2(A6) ; beq.w 0x1A17A8 */
    if (!rd8(m, r->a[6] - 2u))
        return SEG_YIELD3;
    /* 0x1A1832 tst.l D3 ; bne 0x1A183E */
    if (r->d[3] == 0u) {
        push_long(m, r, 0x001A183Cu);               /* 0x1A1836 jsr 0x004DEE */
        if (settings_refresh_4DEE(m, r) != 0)
            return SEG_LEAVE;                       /* 0x010170 reset loop */
    } else {
        r->d[3] -= 1u;                              /* 0x1A183E subq.l #1,D3 */
        if (r->d[3] == 0u) {                        /* 0x1A1840 bne 0x1A1848 */
            push_long(m, r, 0x001A1848u);           /* 0x1A1842 jsr 0x004D06 */
            settings_apply_4D06(m, r);
        }
    }
    Gfx_ClearAllLayers(m);                          /* 0x1A1848 */
    return restore_frame(m, r);
}

uint32_t Seg_1A1282(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A1282u:
        save_frame(m, r);
        r->a[2] = SETTINGS;
        r->a[3] = 0x0061C000u;      /* text RAM base */
        r->a[4] = 0x00005666u;      /* jsr (A4) draws text */
        r->d[6] = P1_SWITCH;
        r->d[7] = 0x0019FFF0u;      /* the shared editor */
        r->d[0] = 9u;               /* moveq #9,D0 */
        wr32(m, r->a[6] - 6u, r->d[0]);         /* 0x1A12AA: nine rows */
        r->d[3] = 0u;                           /* 0x1A12AE */
        wr8(m, r->a[6] - 2u, 0u);               /* 0x1A12B0 clr.b -2(A6) */
        Gfx_ClearAllLayers(m);                  /* 0x1A12B4 */
        r->a[7] -= 4u;                          /* 0x1A12BA jsr 0x004D06 */
        wr32(m, r->a[7], 0x001A12C0u);
        settings_apply_4D06(m, r);
        r->a[7] -= 4u;                          /* 0x1A12C0 jsr 0x1A006C(pc) */
        wr32(m, r->a[7], 0x001A12C4u);
        Fn_1A006C(m, r);
        draw_text(m, r, 0x001A12D2u, 0x0061C09Cu, 0x001A22C2u, 0xAu);
        r->a[7] += 0xAu;                        /* 0x1A12D4 lea 0xA(SP) */
        /* 0x1A12D8 bra.w 0x1A1756: the exit flag is clear, so the frame yields */
        return SEG_YIELD;

    case 0x1A12E2u:
        /* 0x1A12E2: bit 1 of the P2 switch byte steps the selection down */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x2u);
        if (!(r->d[0] & 0xFFu)) {                       /* 0x1A12EC bne */
            r->d[3] += 1u;                              /* 0x1A12EE */
            if ((int32_t)r->d[3] >= (int32_t)rd32(m, r->a[6] - 6u))
                r->d[3] = 0u;                           /* 0x1A12F4 blt */
        }
        /* 0x1A12F8: bit 0 steps it up */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x1u);
        if (!(r->d[0] & 0xFFu)) {                       /* 0x1A1302 bne */
            r->d[3] -= 1u;                              /* 0x1A1304 */
            if ((int32_t)r->d[3] < 0)                   /* 0x1A1306 bpl */
                r->d[3] = rd32(m, r->a[6] - 6u) - 1u;   /* 0x1A1308 */
        }
        row_loop(m, r);
        /* 0x1A1756 tst.b -0x2(A6) ; beq.w 0x1A12DC */
        if (!rd8(m, r->a[6] - 2u))
            return SEG_YIELD;
        r->d[3] = 0u;                                   /* 0x1A175E moveq #0,D3 */
        wr8(m, r->a[6] - 2u, (uint8_t)r->d[3]);         /* 0x1A1760 move.b D3b */
        Gfx_ClearAllLayers(m);                          /* 0x1A1764 */
        return SEG_YIELD2;                              /* 0x1A176A */

    case 0x1A1770u:
        /* the three fixed lines; 0x1A17A0 lea 0x1E(SP) drops their 30 bytes */
        draw_text(m, r, 0x001A177Eu, 0x0061C09Cu, 0x001A22C2u, 0xAu); /* DIFFICULTY */
        draw_text(m, r, 0x001A178Eu, 0x0061C316u, 0x001A20CCu, 0x7u); /* SAVE MODIFIED DATA */
        draw_text(m, r, 0x001A179Eu, 0x0061C41Cu, 0x001A20DFu, 0x7u); /* ARE YOU SURE? */
        r->a[7] += 0x1Eu;                               /* 0x1A17A0 */
        /* 0x1A17A4 bra.w 0x1A182A: the flag is clear, so this segment yields */
        return SEG_YIELD3;

    case 0x1A17AEu:
        return confirm_frame(m, r);
    }
    return SEG_LEAVE;
}
