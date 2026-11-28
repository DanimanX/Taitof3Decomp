/* ROM 0x19F504 test-mode screen "MONITOR TEST" (menu entry 0 of the
 * Task_TestModeMenu table at 0x1A2350), in segments (src/kernel/seg.h). It
 * yields with jsr 0x001F62 (Sys_Yield call gate): the entry stops at 0x19F622,
 * the replay frame at 0x19F62A. The resumes are 0x19F628 (bra to the exit
 * check) and 0x19F630 (the colour animation); the screen ends at the RTS at
 * 0x19F682.
 *
 * Entry 0x19F504: link.w a6,#-0x14, save D2-D7/A2-A3, clear the layers, copy
 * the 48-long character map 0x1A1D98 to 0x61E200 and the 15 colour rows of
 * 0x1A1E58 to 0x61E2C0 (eight longs each, one per sub-row). It then fades the
 * seven RGB ramps at 0x1A1EA8 towards the targets at 0x1A1EE0 in 32 steps,
 * writing one palette long per step at 0x440000 + D7*4 (two D7 slots per ramp
 * row after every 15 writes, the counter at -0x10(A6)), clears the frame
 * counter at -0x14(A6), draws the test grid with 0x19F38A and yields.
 *
 * 0x19F628 only falls into the exit check at 0x19F652. 0x19F630, the frame
 * after that yield, animates while P2 switch bit 5 (0x402225) is clear: it
 * bumps -0x14(A6), masks it to one bit and alternates the grid (0x19F38A) and
 * the bar pattern (0x19F4A6). The exit check leaves when 0x402225 bit 4 is
 * clear, when 0x40221D bit 4 is clear, or when both are set and 0x40221D bit 3
 * is clear; otherwise the frame yields again at 0x19F62A. Leaving clears the
 * layers (0x19F676), restores the saved registers, unlinks and returns.
 *
 * 0x19F38A (grid) and 0x19F4A6 (bars) are ROM subroutines with no other
 * translation, so they are translated here; they save the registers they use
 * except the scratch D0/D1/A0/A1 that the caller's yield then returns. */
#include "../kernel/seg.h"
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */

#define SEG_YIELD 0x0019F622u       /* jsr 0x001F62, the entry's yield */
#define SEG_YIELD2 0x0019F62Au      /* jsr 0x001F62, the frame's yield */

/* 0x19F504 link.w a6,#-0x14 ; 0x19F508 movem.l d2-d7/a2-a3,-(SP) */
static void save_frame(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[6]);
    r->a[6] = r->a[7];
    r->a[7] -= 0x14u;
    r->a[7] -= 32u;
    for (unsigned i = 0; i < 6u; ++i)
        wr32(m, r->a[7] + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 2u; ++i)
        wr32(m, r->a[7] + 24u + 4u * i, r->a[2u + i]);
}

/* 0x19F67C movem.l (a7)+,d2-d7/a2-a3 ; 0x19F680 unlk a6 ; 0x19F682 rts */
static uint32_t restore_frame(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 0; i < 6u; ++i)
        r->d[2u + i] = rd32(m, r->a[7] + 4u * i);
    for (unsigned i = 0; i < 2u; ++i)
        r->a[2u + i] = rd32(m, r->a[7] + 24u + 4u * i);
    r->a[7] += 32u;
    r->a[7] = r->a[6];                      /* unlk a6 */
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return Seg_Run(m, r, seg_pop32(m, r));  /* rts */
}

/* 0x19F38A: fill the 40x15 text grid at 0x61C000 with the four border/body
 * tiles 0x1A1E94/0x1A1E9C, the repeating row 0x1A1EA4 and the inverted body
 * rows of the third pass. A2 is restored; D0/D1/A0/A1 keep the last pass's
 * scratch values, which is what the caller's yield returns. */
static void draw_grid_19F38A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7] - 12u;
    r->a[7] = sp;
    wr32(m, sp, r->d[2]);                   /* movem.l a2/d3/d2,-(SP) */
    wr32(m, sp + 4u, r->d[3]);
    wr32(m, sp + 8u, r->a[2]);
    r->a[2] = 0x0061C000u;
    Gfx_ClearAllLayers(m);                  /* 0x19F394 */

    /* 0x19F39A: rows 0..0xD (0x19F3FE cmp.w #0xE), columns 0..0x27. Rows 0/1
     * use the 0x1A1E9C tiles outside columns 2..0x25, the rest 0x1A1E94. */
    for (r->d[3] = 0u; (int32_t)r->d[3] < 0xE; r->d[3] += 1u) {
        for (r->d[2] = 0u; (int32_t)r->d[2] < 0x28; r->d[2] += 1u) {
            r->d[0] = r->d[3] - 2u;         /* 0x19F39E */
            int corner = (int32_t)r->d[0] < 0;
            if (corner) {
                r->d[0] = r->d[2] - 2u;     /* 0x19F3A4 */
                corner = (int32_t)r->d[0] < 0;
                if (!corner) {
                    r->d[0] = 0x25u;
                    corner = (int32_t)r->d[2] > 0x25;   /* 0x19F3AA */
                }
            }
            r->d[0] = r->d[3] << 7;
            r->d[1] = r->d[2] + r->d[2];
            r->d[0] += r->d[1];
            r->d[1] = 1u;
            r->d[1] &= r->d[2];
            r->d[1] += r->d[1];             /* (D2 & 1) * 2 */
            r->a[0] = r->d[1];
            r->d[1] = 1u;
            r->d[1] &= r->d[3];             /* D3 & 1 */
            uint32_t src = (corner ? 0x001A1E9Cu : 0x001A1E94u)
                           + r->a[0] + 4u * r->d[1];
            wr16(m, r->a[2] + r->d[0], rd16(m, src));
        }
    }
    /* 0x19F402: row 0xE alone (0x19F428 cmp.w #0xF) takes the two-word row
     * pattern 0x1A1EA4. */
    for (r->d[3] = 0xEu; (int32_t)r->d[3] < 0xF; r->d[3] += 1u) {
        for (r->d[2] = 0u; (int32_t)r->d[2] < 0x28; r->d[2] += 1u) {
            r->d[0] = r->d[3] << 7;
            r->d[1] = r->d[2] + r->d[2];
            r->d[0] += r->d[1];
            r->d[1] = 1u;
            r->d[1] &= r->d[2];
            wr16(m, r->a[2] + r->d[0], rd16(m, 0x001A1EA4u + 2u * r->d[1]));
        }
    }
    /* 0x19F430: rows 0xF..0x1C, the inverted tile rows. The outer loop test at
     * 0x19F498 is `moveq #0x1D,D0`, which is the value D0 carries into the
     * caller's yield. */
    for (r->d[3] = 0xFu; ; ) {
        for (r->d[2] = 0u; ; ) {
            /* 0x19F432: rows 0xF..0x1A use 0x1A1E94; below that only the
             * columns outside 2..0x25 take the 0x1A1E9C tiles. */
            r->d[0] = 0x1Au;                /* cmpi.w #0x1A */
            int inner = (int32_t)r->d[3] > 0x1A
                        && ((int32_t)r->d[2] < 2 || (int32_t)r->d[2] > 0x25);
            r->d[0] = r->d[3] << 7;
            r->a[1] = r->d[0];
            r->d[0] = r->d[2] + r->d[2];
            r->a[1] += r->d[0];
            r->d[1] = 1u;
            r->d[1] += r->d[3];
            r->d[0] = 1u;
            r->d[1] &= r->d[0];             /* (D3+1) & 1 */
            r->d[0] &= r->d[2];
            r->d[0] += r->d[0];             /* (D2 & 1) * 2 */
            r->a[0] = r->d[0];
            uint32_t src = (inner ? 0x001A1E9Cu : 0x001A1E94u)
                           + r->a[0] + 4u * r->d[1];
            wr16(m, r->a[2] + r->a[1], rd16(m, src));
            r->d[2] += 1u;                  /* 0x19F48E */
            r->d[0] = 0x28u;
            if ((int32_t)r->d[2] >= 0x28)   /* 0x19F494 blt */
                break;
        }
        r->d[3] += 1u;                      /* 0x19F496 */
        r->d[0] = 0x1Du;                    /* 0x19F498 moveq #0x1D,D0 */
        if ((int32_t)r->d[3] >= 0x1D)       /* 0x19F49C blt */
            break;
    }
    r->d[2] = rd32(m, sp);                  /* movem.l (SP)+,d2/d3/a2 */
    r->d[3] = rd32(m, sp + 4u);
    r->a[2] = rd32(m, sp + 8u);
    r->a[7] = sp + 12u;
}

/* 0x19F4A6: the bar pattern. Clears the layers, then writes 0x16 + (D6 << 9) +
 * D2 (16-bit) into rows (D3 + D4*4) of text RAM at 0x61C088 + D5*2, four
 * D3 values per column and fifteen D2 columns per D6 step. */
static void draw_bars_19F4A6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7] - 20u;
    r->a[7] = sp;
    wr32(m, sp, r->d[2]);                   /* movem.l d2-d6,-(SP) */
    wr32(m, sp + 4u, r->d[3]);
    wr32(m, sp + 8u, r->d[4]);
    wr32(m, sp + 12u, r->d[5]);
    wr32(m, sp + 16u, r->d[6]);
    Gfx_ClearAllLayers(m);                  /* 0x19F4AA */

    r->d[2] = 0u;                           /* 0x19F4B0 */
    r->d[6] = 0x20u;                        /* 0x19F4B2 */
    r->d[4] = 0u;                           /* 0x19F4B4 */
    for (;;) {
        r->d[5] = 0u;                       /* 0x19F4B6 */
        for (;;) {
            r->d[3] = 0u;                   /* 0x19F4B8 */
            for (;;) {
                r->d[1] = 9u;               /* 0x19F4BA */
                r->d[0] = r->d[6] << 9;
                r->d[0] += r->d[2];
                r->d[0] = (r->d[0] & 0xFFFF0000u)
                          | (uint16_t)((r->d[0] + 0x16u) & 0xFFFFu);
                r->a[0] = r->d[3];
                r->a[0] += r->d[4] * 4u;    /* lea (a0,d4*4),a0 */
                r->d[1] = r->a[0];
                r->d[1] <<= 7;
                r->a[0] = r->d[1];
                wr16(m, 0x0061C088u + r->a[0] + 2u * r->d[5],
                     (uint16_t)r->d[0]);
                r->d[3] += 1u;
                r->d[0] = r->d[3] - 4u;
                if ((int32_t)r->d[0] >= 0)
                    break;                  /* 0x19F4E0 blt */
            }
            r->d[2] += 1u;                  /* 0x19F4E2 */
            r->d[0] = 0xFu;
            if ((int32_t)r->d[2] >= 0xF) {  /* 0x19F4E6 blt */
                r->d[6] += 1u;              /* 0x19F4EA */
                r->d[2] = 0u;               /* 0x19F4EC */
            }
            r->d[5] += 1u;                  /* 0x19F4EE */
            r->d[0] = 0x20u;
            if ((int32_t)r->d[5] >= 0x20)   /* 0x19F4F4 blt */
                break;
        }
        r->d[4] += 1u;                      /* 0x19F4F6 */
        r->d[0] = r->d[4] - 7u;
        if ((int32_t)r->d[0] >= 0)          /* 0x19F4FC blt */
            break;
    }
    for (unsigned i = 2; i <= 6u; ++i)      /* movem.l (SP)+,d2-d6 */
        r->d[i] = rd32(m, sp + 4u * (i - 2u));
    r->a[7] = sp + 20u;
}

/* 0x19F652: the exit check both resumes fall into. Bit 4 of 0x402225 and bit
 * 4 of 0x40221D must be set, and 0x40221D bit 3 must be set to keep going;
 * otherwise the screen clears the layers, restores and returns. */
static uint32_t exit_check(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x00402225u) & 0x10u);
    if (r->d[0] & 0xFFu) {                  /* 0x19F65C beq -> exit */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x0040221Du) & 0x10u);
        if (r->d[0] & 0xFFu) {              /* 0x19F668 beq -> exit */
            r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x0040221Du) & 0x08u);
            if (r->d[0] & 0xFFu)            /* 0x19F674 bne 0x19F62A */
                return SEG_YIELD2;
        }
    }
    Gfx_ClearAllLayers(m);                  /* 0x19F676 */
    return restore_frame(m, r);
}

uint32_t Seg_19F504(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x19F504u:
        save_frame(m, r);
        r->a[2] = 0x001A1EA8u;      /* RGB ramp starts */
        r->a[3] = 0x001A1EE0u;      /* RGB ramp targets */
        Gfx_ClearAllLayers(m);      /* 0x19F518 */
        /* 0x19F51E: the 48-long character map */
        for (r->d[3] = 0u; (int32_t)r->d[3] < 0x30; r->d[3] += 1u)
            wr32(m, 0x0061E200u + 4u * r->d[3],
                 rd32(m, 0x001A1D98u + 4u * r->d[3]));
        /* 0x19F536: fifteen rows of eight longs at 0x61E2C0 */
        for (r->d[2] = 0u; (int32_t)r->d[2] < 0xF; r->d[2] += 1u) {
            uint32_t value = rd32(m, 0x001A1E58u + 4u * r->d[2]);
            for (r->d[3] = 0u; (int32_t)r->d[3] < 8; r->d[3] += 1u)
                wr32(m, 0x0061E2C0u + 4u * (r->d[3] + r->d[2] * 8u), value);
        }
        /* 0x19F55E: fade the seven ramps into the palette RAM at 0x440000 */
        wr32(m, r->a[6] - 0x10u, 0u);       /* clr.l -0x10(A6) */
        r->d[7] = 0x201u;
        for (r->d[2] = 0u; (int32_t)r->d[2] < 7; r->d[2] += 1u) {
            uint32_t row = 8u * r->d[2];
            r->d[6] = rd16(m, r->a[2] + row);
            r->d[5] = rd16(m, r->a[2] + row + 2u);
            r->d[4] = rd16(m, r->a[2] + row + 4u);
            /* 0x19F57C: (target - start) / 32, rounded towards -infinity as
             * the ROM's addi.l #0x1F / asr.l #5 pair does. */
            for (unsigned k = 0; k < 3u; ++k) {
                r->d[0] = rd16(m, r->a[3] + row + 2u * k);
                uint32_t start = k == 0u ? r->d[6] : k == 1u ? r->d[5] : r->d[4];
                r->d[0] -= start;
                if ((int32_t)r->d[0] < 0)
                    r->d[0] += 0x1Fu;
                r->d[0] = (uint32_t)((int32_t)r->d[0] >> 5);
                wr32(m, r->a[6] - 0xCu + 4u * k, r->d[0]);
            }
            for (r->d[3] = 0u; (int32_t)r->d[3] < 0x20; r->d[3] += 1u) {
                r->d[0] = r->d[6] & 0xFF00u;    /* 0x19F5C0 */
                r->d[0] <<= 8;
                r->d[1] = r->d[5] & 0xFF00u;
                r->d[0] |= r->d[1];
                r->d[1] = r->d[4] & 0xFF00u;
                r->d[1] = (uint32_t)((int32_t)r->d[1] >> 8);
                r->d[0] |= r->d[1];
                wr32(m, 0x00440000u + 4u * r->d[7], r->d[0]);
                r->d[6] += rd32(m, r->a[6] - 0xCu);
                r->d[5] += rd32(m, r->a[6] - 0x8u);
                r->d[4] += rd32(m, r->a[6] - 0x4u);
                uint32_t count = rd32(m, r->a[6] - 0x10u) + 1u;
                wr32(m, r->a[6] - 0x10u, count);
                r->d[7] += 1u;
                r->d[0] = 0xEu;
                if ((int32_t)r->d[0] < (int32_t)count) {    /* 0x19F600 bge */
                    wr32(m, r->a[6] - 0x10u, 0u);
                    r->d[7] += 1u;
                }
            }
        }
        wr32(m, r->a[6] - 0x14u, 0u);       /* 0x19F61A clr.l -0x14(A6) */
        draw_grid_19F38A(m, r);             /* 0x19F61E */
        return SEG_YIELD;                   /* 0x19F622 */

    case 0x19F628u:
        return exit_check(m, r);            /* 0x19F628 bra 0x19F652 */

    case 0x19F630u:
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x00402225u) & 0x20u);
        if (!(r->d[0] & 0xFFu)) {           /* 0x19F63A bne 0x19F652 */
            uint32_t count = rd32(m, r->a[6] - 0x14u) + 1u;
            count &= 1u;                    /* 0x19F640 and.l D0,-0x14(A6) */
            wr32(m, r->a[6] - 0x14u, count);
            r->d[0] = 1u;
            if (count != 0u)
                draw_bars_19F4A6(m, r);     /* 0x19F64E */
            else
                draw_grid_19F38A(m, r);     /* 0x19F648 */
        }
        return exit_check(m, r);            /* 0x19F652 */
    }
    return SEG_LEAVE;
}
