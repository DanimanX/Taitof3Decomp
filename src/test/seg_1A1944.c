/* ROM 0x1A1944 test-mode screen "EXIT" (menu entry 6 of the Task_TestModeMenu
 * table at 0x1A2350), in segments (src/kernel/seg.h). It yields with
 * jsr 0x001F62 (Sys_Yield call gate) at 0x1A1986 instead of a TRAP: a segment
 * stops AT that jsr and returns 0x1A1986; the next one resumes at 0x1A198C.
 * The screen ends at the RTS at 0x1A1A24.
 *
 * Entry 0x1A1944 saves D2/D3/A2/A3, clears the layers and draws the title
 * "EXIT" at 0x61C0A4 (colour 0xA) and "ARE YOU SURE?" at 0x61C41C (colour 7),
 * then drops the two 10-byte argument blocks (lea 0x14(SP)) and falls to the
 * loop test at 0x1A1A06: D3 == 0, so the frame ends at the yield.
 *
 * Each 0x1A198C frame then runs the YES/NO cursor through the shared editor at
 * 0x19FFF0 (P1 switch byte 0x402224, bits 3/2 as the two directions, the
 * current value D2, range 0..1, step 1, key-repeat counter at 0x40CBD0), draws
 * the "YES       NO" line at 0x61C79C and paints the live choice with the
 * colour-8 "YES" (0x1A19D4) or "NO" (0x1A19E4) arm of the word/jmp table at
 * 0x1A19D0. Bit 4 of the P2 switch byte 0x402225 confirms; D3 then leaves the
 * loop at 0x1A1918 and D2 == 0 takes the soft-reset jsr 0x0001014E, which is
 * not translated (SEG_LEAVE), while any other value just clears the layers,
 * restores the saved registers and returns through the RTS. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */

#define SEG_YIELD 0x001A1986u       /* jsr 0x001F62, the yield */

/* 0x1A1944 movem.l d2-d3/a2-a3,-(SP): D2 lowest in memory, A3 highest */
static void save_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7] - 16u;
    r->a[7] = sp;
    wr32(m, sp, r->d[2]);
    wr32(m, sp + 4u, r->d[3]);
    wr32(m, sp + 8u, r->a[2]);
    wr32(m, sp + 12u, r->a[3]);
}

/* 0x1A1A1C movem.l (a7)+,d2-d3/a2-a3 */
static void restore_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    r->d[2] = rd32(m, sp);
    r->d[3] = rd32(m, sp + 4u);
    r->a[2] = rd32(m, sp + 8u);
    r->a[3] = rd32(m, sp + 12u);
    r->a[7] = sp + 16u;
}

/* jsr (A2) -> 0x5666: the three arguments pushed in place (they stay until the
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

/* 0x19FFF0: the shared YES/NO cursor editor. Args (from the caller's SP+4):
 * the P1 switch byte, the current value, the low bound, the high bound and the
 * step. D5 = switch byte, D1 = value; A0 = the key-repeat word at 0x40CBD0;
 * returns the new value in D0.w (D1.w). Holding both direction bits (3 and 2)
 * just resets the repeat counter. */
static void editor_19FFF0(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                          uint16_t in, uint16_t cur, uint16_t lo, uint16_t hi,
                          uint16_t step)
{
    uint32_t sp = r->a[7] - 10u;        /* 5 argument words */
    r->a[7] = sp;
    wr16(m, sp, in);                    /* move.w D0w,-(a7) */
    wr16(m, sp + 2u, cur);              /* move.w D2w,-(a7) */
    wr16(m, sp + 4u, 0u);               /* clr.w -(a7) */
    wr16(m, sp + 6u, 1u);               /* move.w #1,-(a7) */
    wr16(m, sp + 8u, 1u);               /* move.w #1,-(a7) */
    wr32(m, sp - 4u, jsr_pc + 2u);
    wr32(m, sp - 8u, r->d[5]);          /* movem.l d3-d5,-(SP) */
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
                                         * lea 0x14(SP) at 0x1A18CC/0x1A19BA */
}

uint32_t Seg_1A1944(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A1944u:
        save_test_regs(m, r);
        r->a[2] = 0x00005666u;      /* jsr (A2) draws text */
        r->a[3] = 0x0061C000u;      /* text RAM base */
        r->d[2] = 0u;
        r->d[3] = 0u;
        Gfx_ClearAllLayers(m);      /* 0x1A1958 */
        draw_text(m, r, 0x001A196Cu, 0x0061C0A4u, 0x001A20C7u, 0xAu); /* EXIT */
        draw_text(m, r, 0x001A197Cu, 0x0061C41Cu, 0x001A20DFu, 0x7u); /* ARE YOU SURE? */
        r->a[7] += 0x14u;           /* 0x1A197E lea 0x14(SP) */
        /* 0x1A1A06 tst.b D3 is zero on entry: the frame ends at the yield */
        return SEG_YIELD;

    case 0x1A198Cu:
        editor_19FFF0(m, r, 0x001A19A2u, (uint16_t)rd8(m, 0x00402224u),
                      (uint16_t)r->d[2], 0u, 1u, 1u);
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];  /* ext.l D0 */
        r->d[2] = r->d[0];
        draw_text(m, r, 0x001A19B8u, 0x0061C79Cu, 0x001A20EDu, 0x7u); /* YES       NO */
        r->a[7] += 0x14u;           /* 0x1A19BA lea 0x14(SP) */
        r->d[0] = r->d[2];
        r->d[0] -= 1u;              /* 0x1A19C0 subq.l #1,D0 */
        /* 0x1A19C2 bhi.w: taken unless D2 is 0 or 1 */
        if (r->d[2] <= 1u) {
            /* 0x1A19C6: word table at 0x1A19D2 indexed by D0.w*2, entries are
             * offsets from 0x1A19D0 for the (0x1A19D0,PC,D0.w) jmp. */
            uint32_t ea = 0x001A19D2u + (uint32_t)(2 * (int32_t)(int16_t)r->d[0]);
            uint16_t off = rd16(m, ea);
            r->d[0] = (r->d[0] & 0xFFFF0000u) | off;
            switch (0x001A19D0u + (uint32_t)(int16_t)off) {
            case 0x1A19D4u:         /* D2 == 0: YES at 0x61C79C */
                draw_text(m, r, 0x001A19F2u, 0x0061C79Cu, 0x001A20FAu, 0x8u);
                break;
            case 0x1A19E4u:         /* D2 == 1: NO at 0x61C7B0 */
                draw_text(m, r, 0x001A19F2u, 0x0061C7B0u, 0x001A20FEu, 0x8u);
                break;
            }
            r->a[7] += 0xAu;        /* 0x1A1906 / 0x1A19F4 lea 0xA(SP) */
        }
        /* 0x1A19F8: bit 4 of the P2 switch byte confirms the choice */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x00402225u) & 0x10u);
        if (!(r->d[0] & 0xFFu))
            r->d[3] = 1u;           /* 0x1A1A04 moveq #1,D3 */
        /* 0x1A1A06 tst.b D3 */
        if ((r->d[3] & 0xFFu) == 0u)
            return SEG_YIELD;       /* 0x1A1A08 beq.w 0x1A1986 */
        if (r->d[2] == 0u)
            return SEG_LEAVE;       /* 0x1A1A10 jsr 0x0001014E, untranslated */
        Gfx_ClearAllLayers(m);      /* 0x1A1A16 */
        restore_test_regs(m, r);
        return Seg_Run(m, r, seg_pop32(m, r));  /* 0x1A1A24 */
    }
    return SEG_LEAVE;
}
