/* ROM 0x1A095E test-mode screen "HI-SCORE RESET", in segments
 * (src/kernel/seg.h).
 *
 * UNVERIFIED (no capture): work/ram has no seg_1A095E or seg_1A09AC data (the
 * fn_1A095E capture exists but every one of its 30 pairs was sampled several
 * frames after its entry, so test_fn skips them all as VBlank-interrupted and
 * a test entry would only ever report REJECT). Everything below was read from
 * work/maincpu.bin; a synthetic driver from a captured screen state exercised
 * the entry, both YES/NO arms and both exits (0x509A -> SEG_LEAVE, and
 * 0x4D06 -> epilogue -> RTS).
 *
 * It yields with jsr 0x001F62 (Sys_Yield call gate) at 0x1A09A6: a segment
 * stops AT that jsr and returns its PC, the next one resumes 6 bytes later at
 * 0x1A09AC. The screen ends at the RTS at 0x1A0A50.
 *
 * Entry 0x1A095E saves D2/D3/A2/A3 (no link frame), A2 = 0x5666 (jsr (A2)
 * draws), A3 = the text base 0x61C000, D2 = the YES/NO choice, D3 = the exit
 * flag. It clears the layers, reloads the settings (0x4D06), draws the title
 * "HI-SCORE RESET" at 0x61C09A (colour 0xA) and "ARE YOU SURE?" at 0x61C41C
 * (colour 7), drops the two 10-byte argument blocks with lea 0x14(SP) and
 * falls to the loop test at 0x1A0A26, which sends D3 == 0 to the yield.
 *
 * Each 0x1A09AC frame runs the YES/NO cursor through the shared editor
 * 0x19FFF0 (P1 switch byte 0x402224, range 0..1, step 1), draws the
 * "YES       NO" line at 0x61C79C and paints the choice with the colour-8 YES
 * (0x1A20FA) or NO (0x1A20FE) arm of the word/jmp table at 0x1A09F0. The
 * editor's five argument words and the line's ten are dropped together by the
 * lea 0x14(SP) at 0x1A09DA. Bit 4 of the P2 switch byte 0x402225 confirms; D3
 * then leaves the loop at 0x1A0A2C, where D2 == 0 runs the factory-default
 * reload jsr 0x0000509A (a translated callee that writes the defaults and then
 * cannot complete its serial write-back without the device: SEG_LEAVE on that
 * arm) and D2 == 1 the settings reload jsr 0x00004D06; both then clear the
 * layers, restore the four registers and return through the RTS. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */
#include "../auto/fn_auto.h"        /* Fn_19FFF0, Fn_00509A */
#include "../auto/eeprom_block.h"   /* settings_apply_4D06 (ROM 0x004D06) */

#define SEG_YIELD 0x001A09A6u       /* jsr 0x001F62, the frame loop */
#define P1_SWITCH 0x00402224u
#define P2_SWITCH 0x00402225u

/* 0x1A095E movem.l d2-d3/a2-a3,-(SP): D2 lowest in memory, A3 highest */
static void save_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7] - 16u;
    r->a[7] = sp;
    wr32(m, sp, r->d[2]);
    wr32(m, sp + 4u, r->d[3]);
    wr32(m, sp + 8u, r->a[2]);
    wr32(m, sp + 12u, r->a[3]);
}

/* 0x1A0A48 movem.l (SP)+,d2-d3/a2-a3 ; 0x1A0A50 rts */
static uint32_t restore_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    r->d[2] = rd32(m, sp);
    r->d[3] = rd32(m, sp + 4u);
    r->a[2] = rd32(m, sp + 8u);
    r->a[3] = rd32(m, sp + 12u);
    r->a[7] = sp + 16u;
    return Seg_Run(m, r, seg_pop32(m, r));
}

/* jsr (A2) -> 0x5666: the three arguments stay until the caller's lea, the
 * callee's frame is written below them and only D0 survives. */
static void draw_text(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      uint32_t dst, uint32_t src, uint16_t col)
{
    uint32_t sp = r->a[7] - 10u;
    r->a[7] = sp;
    wr32(m, sp, src);
    wr32(m, sp + 4u, dst);
    wr16(m, sp + 8u, col);
    wr32(m, sp - 4u, ret);
    wr32(m, sp - 8u, r->a[6]);
    wr32(m, sp - 20u, r->d[7]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 12u, r->a[1]);
    r->d[0] = Text_DrawString(m, src, dst, col);
    r->a[7] = sp;                       /* the callee's rts pops its return PC */
}

uint32_t Seg_1A095E(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A095Eu:
        save_test_regs(m, r);
        r->a[2] = 0x00005666u;      /* jsr (A2) draws text */
        r->a[3] = 0x0061C000u;      /* text RAM base */
        r->d[2] = 0u;                       /* yes/no choice */
        r->d[3] = 0u;                       /* exit flag */
        Gfx_ClearAllLayers(m);              /* 0x1A0972 */
        push_long(m, r, 0x001A097Eu);
        settings_apply_4D06(m, r);          /* 0x1A0978 jsr 0x004D06 */
        draw_text(m, r, 0x001A098Eu, 0x0061C09Au, 0x001A2136u, 0xAu); /* HI-SCORE RESET */
        draw_text(m, r, 0x001A099Eu, 0x0061C41Cu, 0x001A20DFu, 0x7u); /* ARE YOU SURE? */
        r->a[7] += 0x14u;           /* 0x1A099E lea 0x14(SP) */
        return SEG_YIELD;           /* 0x1A0A26/0x1A0A28 beq.w 0x1A09A6 */

    case 0x1A09ACu:
        /* One frame of the YES/NO loop. */
        r->d[0] = rd8(m, P1_SWITCH);        /* 0x1A09B8 moveq #0/move.b */
        push_word(m, r, 1u);                /* step */
        push_word(m, r, 1u);                /* high bound */
        push_word(m, r, 0u);                /* low bound */
        push_word(m, r, (uint16_t)r->d[2]); /* current value */
        push_word(m, r, (uint16_t)r->d[0]); /* P1 switch byte */
        push_long(m, r, 0x001A09C6u);
        Fn_19FFF0(m, r);                    /* 0x1A09C2; leaves the five words */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.l D0 */
        r->d[2] = r->d[0];                  /* 0x1A09C8 move.l D0,D2 */
        draw_text(m, r, 0x001A09DAu, 0x0061C79Cu, 0x001A20EDu, 0x7u); /* YES   NO */
        r->a[7] += 0x14u;                   /* 0x1A09DA lea 0x14(SP) */
        r->d[0] = r->d[2];
        r->d[0] -= 1u;                      /* 0x1A09E0 subq.l #1,D0 */
        if (r->d[2] <= 1u) {                /* 0x1A09E2 bhi.w 0x1A0A18 */
            int32_t idx = (int32_t)(int16_t)(uint16_t)r->d[0];
            uint16_t off = rd16(m, (uint32_t)(0x001A09F2u + 2 * idx));
            seg_setw(&r->d[0], off);        /* 0x1A09E6 move.w */
            switch (0x001A09F0u + (uint32_t)(int16_t)off) {
            case 0x1A09F4u:                 /* D2 == 0: YES at 0x61C79C */
                draw_text(m, r, 0x001A0A14u, 0x0061C79Cu, 0x001A20FAu, 0x8u);
                break;
            case 0x1A0A04u:                 /* D2 == 1: NO at 0x61C7B0 */
                draw_text(m, r, 0x001A0A14u, 0x0061C7B0u, 0x001A20FEu, 0x8u);
                break;
            }
            r->a[7] += 0xAu;                /* 0x1A0A14 lea 0xA(SP) */
        }
        /* 0x1A0A18: bit 4 of the P2 switch byte confirms the choice */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (!(r->d[0] & 0xFFu))             /* 0x1A0A22 bne */
            r->d[3] = 1u;                   /* 0x1A0A24 moveq #1,D3 */
        /* 0x1A0A26 tst.b D3 */
        if ((r->d[3] & 0xFFu) == 0u)
            return SEG_YIELD;               /* 0x1A0A28 beq.w 0x1A09A6 */
        if (r->d[2] == 0u) {                /* 0x1A0A2C tst.l D2 ; bne */
            push_long(m, r, 0x001A0A36u);
            /* 0x1A0A30 jsr 0x0000509A: the defaults are written and then the
             * serial write-back through 0x004DEE fails without the device, so
             * this arm enters the reset loop - SEG_LEAVE. */
            if (Fn_00509A(m, r) < 0)
                return SEG_LEAVE;
            goto cleared;
        }
        r->d[2] -= 1u;                      /* 0x1A0A38 subq.l #1,D2 */
        if (r->d[2] == 0u) {                /* 0x1A0A3A bne 0x1A0A42 */
            push_long(m, r, 0x001A0A42u);
            settings_apply_4D06(m, r);      /* 0x1A0A3C jsr 0x004D06 */
        }
    cleared:
        Gfx_ClearAllLayers(m);              /* 0x1A0A42 */
        return restore_test_regs(m, r);     /* 0x1A0A48..0x1A0A50 */
    }
    return SEG_LEAVE;
}
