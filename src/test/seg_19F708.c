/* ROM 0x19F708 test-mode screen (entered from Task_TestModeMenu 0x1A1A26), in
 * segments (src/kernel/seg.h). It yields with jsr 0x001F62 (Sys_Yield call
 * gate) at 0x19F742 instead of a TRAP: a segment stops AT that jsr and
 * returns 0x19F742; the next one resumes at 0x19F748. The screen ends at the
 * RTS at 0x19FFEE (return through seg_pop32 as usual).
 *
 * The routine dumps the EEPROM switch settings over two pages. Entry draws
 * nothing: it jumps straight to the 0x19FFB4 poll head, which yields once per
 * frame while the switch bytes still read as "no exit". Each 0x19F748 frame
 * then redraws one page - the switch table at 0x19F81E while the page index
 * D5 != 5, the confirm page at 0x19FA78 once D5 == 5 (that page also commits
 * the 0x402234/0x402235 exit flags at 0x19FF42). An exit switch or a coin
 * edge (0x40221C bit 3/4) drops out at 0x19FFD8: flags cleared, layers
 * cleared, the saved D2-D7/A2-A4 restored and the RTS taken.
 *
 * Every text draw is jsr 0x00005666 (Text_DrawString) with three arguments
 * pushed, either directly through A3 or through the ON/OFF helper at
 * 0x19F684 (A0 = the settings byte, D1.w = the bit to test). The pushes are
 * NOT popped per call: they accumulate and one lea at the end of each page
 * drops them (0x1A4 for the switch page, 0x1FE + 0x14A for the confirm page).
 * Their bytes, the jsr's return PC and the callee frames stay in Work RAM
 * below SP and the seg_19F748 capture compares them (the pushes reach 0x1C9
 * below the entry SP on the captured switch page, deeper on the confirm page,
 * past the harness's SP-0x100 window), so text_label/text_flag write that
 * stack image exactly while leaving the registers as the callees leave them. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */

#define SEG_YIELD 0x0019F742u       /* jsr 0x001F62, the one yield */

/* 0x19F708 movem.l d2-d7/a2-a4,-(SP): D2 lowest in memory, A4 highest */
static void save_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7] - 36u;
    r->a[7] = sp;
    for (unsigned i = 0; i < 6u; ++i)
        wr32(m, sp + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 3u; ++i)
        wr32(m, sp + 24u + 4u * i, r->a[2u + i]);
}

/* 0x19FFEA movem.l (a7)+,d2-d7/a2-a4 */
static void restore_test_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    for (unsigned i = 0; i < 6u; ++i)
        r->d[2u + i] = rd32(m, sp + 4u * i);
    for (unsigned i = 0; i < 3u; ++i)
        r->a[2u + i] = rd32(m, sp + 24u + 4u * i);
    r->a[7] = sp + 36u;
}

/* The three arguments of one draw - move.w #col,-(SP) ; pea dst ; pea src -
 * pushed and left in place until the page's closing lea. Returns the SP. */
static uint32_t push_text_args(const f3mem_t *m, f3regs_t *r, uint32_t src,
                               uint32_t dst, uint16_t col)
{
    r->a[7] -= 10u;
    wr32(m, r->a[7], src);
    wr32(m, r->a[7] + 4u, dst);
    wr16(m, r->a[7] + 8u, col);
    return r->a[7];
}

/* jsr (A3) -> 0x5666: link.w A6,#0 then movem.l d7/a0-a1,-(SP), both undone
 * by its rts. Only D0 (the new text cursor) survives; the 10 argument bytes
 * stay until the page's lea, the frame bytes stay for good. */
static void text_label(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                       uint32_t dst, uint32_t src, uint16_t col)
{
    uint32_t sp = push_text_args(m, r, src, dst, col);
    wr32(m, sp - 4u, jsr_pc + 2u);          /* jsr (A3) */
    wr32(m, sp - 8u, r->a[6]);              /* link.w a6,#0 */
    wr32(m, sp - 20u, r->d[7]);             /* movem.l d7/a0-a1,-(SP) */
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 12u, r->a[1]);
    r->d[0] = Text_DrawString(m, src, dst, col);
}

/* jsr (A0) -> 0x19F684 through D2: draws "ON " (col 5) when none of the mask
 * bits is set in the settings byte and "OFF" (col 4) when one is, at the
 * cursor in D0. Leaves A0 = the settings byte, D1 = the sign-extended mask,
 * D0 = the new cursor; its frame and the 0x5666 frame below it are written. */
static void text_flag(const f3mem_t *m, f3regs_t *r, uint32_t jsr_pc,
                      uint32_t addr, uint16_t mask)
{
    uint32_t dst = r->d[0];
    uint32_t sp = push_text_args(m, r, addr, dst, mask);
    wr32(m, sp - 4u, jsr_pc + 2u);          /* jsr (A0) */
    uint32_t a6 = sp - 8u;                  /* link.w a6,#0 */
    wr32(m, a6, r->a[6]);

    r->d[1] = (uint32_t)(int32_t)(int16_t)mask;
    r->a[0] = addr;
    r->d[0] = rd8(m, addr) & r->d[1];
    uint16_t col = r->d[0] ? 4u : 5u;
    uint32_t str = r->d[0] ? 0x001A1F1Cu : 0x001A1F18u;

    wr16(m, a6 - 2u, col);                  /* move.w #col,-(SP) */
    wr32(m, a6 - 6u, dst);                  /* move.l (0xc,A6),-(SP) */
    wr32(m, a6 - 10u, str);                 /* pea (0x1a1f18/1c) */
    wr32(m, a6 - 14u, 0x0019F6BEu);         /* jsr 0x00005666 */
    wr32(m, a6 - 18u, a6);                  /* 0x5666 link: A6 of its caller */
    wr32(m, a6 - 30u, r->d[7]);             /* movem.l d7/a0-a1,-(SP) */
    wr32(m, a6 - 26u, addr);                /* A0 = the settings byte */
    wr32(m, a6 - 22u, r->a[1]);
    r->d[0] = Text_DrawString(m, str, dst, col);
}

/* 0x19FF42: fold four switch bytes into the 0x402234/0x402235 exit flags. */
static void commit_switch_flags(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t addr[4] = { r->d[3], r->d[4], r->d[7], 0x00402230u };
    static const uint32_t word[4] = { 0x00402234u, 0x00402234u,
                                      0x00402235u, 0x00402235u };
    static const uint8_t bit[4] = { 1u, 2u, 1u, 2u };

    for (unsigned i = 0; i < 4u; ++i) {
        if (i < 3u)
            r->a[0] = addr[i];              /* movea.l D3/D4/D7,A0 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, addr[i]);
        seg_setw(&r->d[0], (uint16_t)((r->d[0] & 0xFFu) & 0x80u));
        if (r->d[0] & 0xFFu)
            wr8(m, word[i], (uint8_t)(rd8(m, word[i]) & (uint8_t)~bit[i]));
        else
            wr8(m, word[i], (uint8_t)(rd8(m, word[i]) | bit[i]));
    }
}

/* 0x19FFD8: clear the exit flags, clear the layers, restore and rts. */
static uint32_t test_exit(const f3mem_t *m, f3regs_t *r)
{
    wr8(m, 0x00402234u, 0u);
    wr8(m, 0x00402235u, 0u);
    Gfx_ClearAllLayers(m);
    restore_test_regs(m, r);
    return Seg_Run(m, r, seg_pop32(m, r));  /* 0x19FFEE */
}

/* 0x19FFB4: one poll of the exit switches; the yield or the epilogue.
 * D3 is the PLAYER 1 switch byte, 0x40221C latches coins (bit 4) and the
 * test-mode exit (bit 3). */
static uint32_t poll_head(const f3mem_t *m, f3regs_t *r)
{
    r->a[0] = r->d[3];
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, r->d[3]) & 0x90u);
    if (!(r->d[0] & 0xFFu))
        return test_exit(m, r);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x0040221Cu) & 0x10u);
    if (!(r->d[0] & 0xFFu))
        return test_exit(m, r);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, 0x0040221Cu) & 0x08u);
    if (!(r->d[0] & 0xFFu))
        return test_exit(m, r);
    return SEG_YIELD;
}

/* 0x19F81E: page index D5 != 5, the switch table. The closing lea 0x1A4(SP)
 * at 0x19FA70 drops the 42 pushed argument blocks. */
static void draw_switch_page(const f3mem_t *m, f3regs_t *r)
{
    text_label(m, r, 0x0019F82Cu, 0x0061C186u, 0x001A1F52u, 0x7u);   /* 0x19F82C SERVICE: */
    text_flag(m, r, 0x0019F838u, 0x00402220u, 0x2u);   /* 0x19F838 */
    text_label(m, r, 0x0019F848u, 0x0061C1A8u, 0x001A1F5Bu, 0x7u);   /* 0x19F848 TILT   : */
    text_flag(m, r, 0x0019F854u, 0x00402220u, 0x1u);   /* 0x19F854 */
    text_label(m, r, 0x0019F864u, 0x0061C286u, 0x001A1F64u, 0x7u);   /* 0x19F864 COIN A : */
    text_flag(m, r, 0x0019F870u, 0x00402220u, 0x10u);   /* 0x19F870 */
    text_label(m, r, 0x0019F880u, 0x0061C2A8u, 0x001A1F6Du, 0x7u);   /* 0x19F880 COIN B : */
    text_flag(m, r, 0x0019F88Cu, 0x00402220u, 0x20u);   /* 0x19F88C */
    text_label(m, r, 0x0019F89Cu, 0x0061C488u, 0x001A1F76u, 0x8u);   /* 0x19F89C PLAYER 1 */
    text_label(m, r, 0x0019F8ACu, 0x0061C486u, 0x001A1F7Fu, 0x7u);   /* 0x19F8AC START  : */
    text_flag(m, r, 0x0019F8B8u, 0x00402224u, 0x80u);   /* 0x19F8B8 */
    text_label(m, r, 0x0019F8C8u, 0x0061C586u, 0x001A1F88u, 0x7u);   /* 0x19F8C8 FIRE 1 : */
    text_flag(m, r, 0x0019F8D4u, 0x00402224u, 0x10u);   /* 0x19F8D4 */
    text_label(m, r, 0x0019F8E4u, 0x0061C686u, 0x001A1F91u, 0x7u);   /* 0x19F8E4 FIRE 2 : */
    text_flag(m, r, 0x0019F8F0u, 0x00402224u, 0x20u);   /* 0x19F8F0 */
    text_label(m, r, 0x0019F900u, 0x0061C786u, 0x001A1F9Au, 0x7u);   /* 0x19F900 FIRE 3 : */
    text_flag(m, r, 0x0019F90Cu, 0x00402224u, 0x40u);   /* 0x19F90C */
    text_label(m, r, 0x0019F91Cu, 0x0061C886u, 0x001A1FA3u, 0x7u);   /* 0x19F91C UP     : */
    text_flag(m, r, 0x0019F928u, 0x00402224u, 0x1u);   /* 0x19F928 */
    text_label(m, r, 0x0019F938u, 0x0061C986u, 0x001A1FACu, 0x7u);   /* 0x19F938 DOWN   : */
    text_flag(m, r, 0x0019F944u, 0x00402224u, 0x2u);   /* 0x19F944 */
    text_label(m, r, 0x0019F954u, 0x0061CA86u, 0x001A1FB5u, 0x7u);   /* 0x19F954 LEFT   : */
    text_flag(m, r, 0x0019F960u, 0x00402224u, 0x4u);   /* 0x19F960 */
    text_label(m, r, 0x0019F970u, 0x0061CB86u, 0x001A1FBEu, 0x7u);   /* 0x19F970 RIGHT  : */
    text_flag(m, r, 0x0019F97Cu, 0x00402224u, 0x8u);   /* 0x19F97C */
    text_label(m, r, 0x0019F98Cu, 0x0061C4AAu, 0x001A1FC7u, 0x6u);   /* 0x19F98C PLAYER 2 */
    text_label(m, r, 0x0019F99Cu, 0x0061C4A8u, 0x001A1F7Fu, 0x7u);   /* 0x19F99C START  : */
    text_flag(m, r, 0x0019F9A8u, 0x00402228u, 0x80u);   /* 0x19F9A8 */
    text_label(m, r, 0x0019F9B8u, 0x0061C5A8u, 0x001A1F88u, 0x7u);   /* 0x19F9B8 FIRE 1 : */
    text_flag(m, r, 0x0019F9C4u, 0x00402228u, 0x10u);   /* 0x19F9C4 */
    text_label(m, r, 0x0019F9D4u, 0x0061C6A8u, 0x001A1F91u, 0x7u);   /* 0x19F9D4 FIRE 2 : */
    text_flag(m, r, 0x0019F9E0u, 0x00402228u, 0x20u);   /* 0x19F9E0 */
    text_label(m, r, 0x0019F9F0u, 0x0061C7A8u, 0x001A1F9Au, 0x7u);   /* 0x19F9F0 FIRE 3 : */
    text_flag(m, r, 0x0019F9FCu, 0x00402228u, 0x40u);   /* 0x19F9FC */
    text_label(m, r, 0x0019FA0Cu, 0x0061C8A8u, 0x001A1FA3u, 0x7u);   /* 0x19FA0C UP     : */
    text_flag(m, r, 0x0019FA18u, 0x00402228u, 0x1u);   /* 0x19FA18 */
    text_label(m, r, 0x0019FA28u, 0x0061C9A8u, 0x001A1FACu, 0x7u);   /* 0x19FA28 DOWN   : */
    text_flag(m, r, 0x0019FA34u, 0x00402228u, 0x2u);   /* 0x19FA34 */
    text_label(m, r, 0x0019FA44u, 0x0061CAA8u, 0x001A1FB5u, 0x7u);   /* 0x19FA44 LEFT   : */
    text_flag(m, r, 0x0019FA50u, 0x00402228u, 0x4u);   /* 0x19FA50 */
    text_label(m, r, 0x0019FA60u, 0x0061CBA8u, 0x001A1FBEu, 0x7u);   /* 0x19FA60 RIGHT  : */
    r->a[2] = r->d[0];              /* 0x19FA62 movea.l D0,A2 */
    text_flag(m, r, 0x0019FA6Eu, 0x00402228u, 0x8u);   /* 0x19FA6E */
    r->a[7] += 0x1A4u;
}

/* 0x19FA78: page index D5 == 5, the confirm page. The first block ends at
 * 0x19FD48 with lea 0x1FE(SP), the second at 0x19FF3E with lea 0x14A(SP). */
static void draw_confirm_page(const f3mem_t *m, f3regs_t *r)
{
    text_label(m, r, 0x0019FA86u, 0x0061C186u, 0x001A1FD0u, 0x7u);   /* 0x19FA86 TILT     : */
    text_flag(m, r, 0x0019FA92u, 0x00402220u, 0x1u);   /* 0x19FA92 */
    text_label(m, r, 0x0019FAA2u, 0x0061C206u, 0x001A1FDBu, 0x7u);   /* 0x19FAA2 SERVICE A: */
    text_flag(m, r, 0x0019FAAEu, 0x00402220u, 0x2u);   /* 0x19FAAE */
    text_label(m, r, 0x0019FABEu, 0x0061C286u, 0x001A1FE6u, 0x7u);   /* 0x19FABE SERVICE B: */
    text_flag(m, r, 0x0019FACAu, 0x00402220u, 0x4u);   /* 0x19FACA */
    text_label(m, r, 0x0019FADAu, 0x0061C306u, 0x001A1FF1u, 0x7u);   /* 0x19FADA SERVICE C: */
    text_flag(m, r, 0x0019FAE6u, 0x00402220u, 0x8u);   /* 0x19FAE6 */
    text_label(m, r, 0x0019FAF6u, 0x0061C1A8u, 0x001A1FFCu, 0x7u);   /* 0x19FAF6 COIN A   : */
    text_flag(m, r, 0x0019FB02u, 0x00402220u, 0x10u);   /* 0x19FB02 */
    text_label(m, r, 0x0019FB12u, 0x0061C228u, 0x001A2007u, 0x7u);   /* 0x19FB12 COIN B   : */
    text_flag(m, r, 0x0019FB1Eu, 0x00402220u, 0x20u);   /* 0x19FB1E */
    text_label(m, r, 0x0019FB2Eu, 0x0061C2A8u, 0x001A2012u, 0x7u);   /* 0x19FB2E COIN C   : */
    text_flag(m, r, 0x0019FB3Au, 0x00402220u, 0x40u);   /* 0x19FB3A */
    text_label(m, r, 0x0019FB4Au, 0x0061C328u, 0x001A201Du, 0x7u);   /* 0x19FB4A COIN D   : */
    text_flag(m, r, 0x0019FB56u, 0x00402220u, 0x80u);   /* 0x19FB56 */
    text_label(m, r, 0x0019FB66u, 0x0061C408u, 0x001A1F76u, 0x8u);   /* 0x19FB66 PLAYER 1 */
    text_label(m, r, 0x0019FB76u, 0x0061C486u, 0x001A2028u, 0x7u);   /* 0x19FB76 START    : */
    text_flag(m, r, 0x0019FB82u, 0x00402224u, 0x80u);   /* 0x19FB82 */
    text_label(m, r, 0x0019FB92u, 0x0061C506u, 0x001A2033u, 0x7u);   /* 0x19FB92 FIRE 1   : */
    text_flag(m, r, 0x0019FB9Eu, 0x00402224u, 0x10u);   /* 0x19FB9E */
    text_label(m, r, 0x0019FBAEu, 0x0061C586u, 0x001A203Eu, 0x7u);   /* 0x19FBAE FIRE 2   : */
    text_flag(m, r, 0x0019FBBAu, 0x00402224u, 0x20u);   /* 0x19FBBA */
    text_label(m, r, 0x0019FBCAu, 0x0061C606u, 0x001A2049u, 0x7u);   /* 0x19FBCA FIRE 3   : */
    text_flag(m, r, 0x0019FBD6u, 0x00402224u, 0x40u);   /* 0x19FBD6 */
    text_label(m, r, 0x0019FBE6u, 0x0061C686u, 0x001A2054u, 0x7u);   /* 0x19FBE6 UP       : */
    text_flag(m, r, 0x0019FBF2u, 0x00402224u, 0x1u);   /* 0x19FBF2 */
    text_label(m, r, 0x0019FC02u, 0x0061C706u, 0x001A205Fu, 0x7u);   /* 0x19FC02 DOWN     : */
    text_flag(m, r, 0x0019FC0Eu, 0x00402224u, 0x2u);   /* 0x19FC0E */
    text_label(m, r, 0x0019FC1Eu, 0x0061C786u, 0x001A206Au, 0x7u);   /* 0x19FC1E LEFT     : */
    text_flag(m, r, 0x0019FC2Au, 0x00402224u, 0x4u);   /* 0x19FC2A */
    text_label(m, r, 0x0019FC3Au, 0x0061C806u, 0x001A2075u, 0x7u);   /* 0x19FC3A RIGHT    : */
    text_flag(m, r, 0x0019FC46u, 0x00402224u, 0x8u);   /* 0x19FC46 */
    text_label(m, r, 0x0019FC56u, 0x0061C42Au, 0x001A1FC7u, 0x6u);   /* 0x19FC56 PLAYER 2 */
    text_label(m, r, 0x0019FC66u, 0x0061C4A8u, 0x001A2028u, 0x7u);   /* 0x19FC66 START    : */
    text_flag(m, r, 0x0019FC72u, 0x00402228u, 0x80u);   /* 0x19FC72 */
    text_label(m, r, 0x0019FC82u, 0x0061C528u, 0x001A2033u, 0x7u);   /* 0x19FC82 FIRE 1   : */
    text_flag(m, r, 0x0019FC8Eu, 0x00402228u, 0x10u);   /* 0x19FC8E */
    text_label(m, r, 0x0019FC9Eu, 0x0061C5A8u, 0x001A203Eu, 0x7u);   /* 0x19FC9E FIRE 2   : */
    text_flag(m, r, 0x0019FCAAu, 0x00402228u, 0x20u);   /* 0x19FCAA */
    text_label(m, r, 0x0019FCBAu, 0x0061C628u, 0x001A2049u, 0x7u);   /* 0x19FCBA FIRE 3   : */
    text_flag(m, r, 0x0019FCC6u, 0x00402228u, 0x40u);   /* 0x19FCC6 */
    text_label(m, r, 0x0019FCD6u, 0x0061C6A8u, 0x001A2054u, 0x7u);   /* 0x19FCD6 UP       : */
    text_flag(m, r, 0x0019FCE2u, 0x00402228u, 0x1u);   /* 0x19FCE2 */
    text_label(m, r, 0x0019FCF2u, 0x0061C728u, 0x001A205Fu, 0x7u);   /* 0x19FCF2 DOWN     : */
    text_flag(m, r, 0x0019FCFEu, 0x00402228u, 0x2u);   /* 0x19FCFE */
    text_label(m, r, 0x0019FD0Eu, 0x0061C7A8u, 0x001A206Au, 0x7u);   /* 0x19FD0E LEFT     : */
    text_flag(m, r, 0x0019FD1Au, 0x00402228u, 0x4u);   /* 0x19FD1A */
    text_label(m, r, 0x0019FD2Au, 0x0061C828u, 0x001A2075u, 0x7u);   /* 0x19FD2A RIGHT    : */
    text_flag(m, r, 0x0019FD36u, 0x00402228u, 0x8u);   /* 0x19FD36 */
    text_label(m, r, 0x0019FD46u, 0x0061C908u, 0x001A2080u, 0x5u);   /* 0x19FD46 PLAYER 3 */
    r->a[7] += 0x1FEu;

    text_label(m, r, 0x0019FD5Au, 0x0061C986u, 0x001A2028u, 0x7u);   /* 0x19FD5A START    : */
    text_flag(m, r, 0x0019FD66u, 0x0040222Cu, 0x80u);   /* 0x19FD66 */
    text_label(m, r, 0x0019FD76u, 0x0061CA06u, 0x001A2033u, 0x7u);   /* 0x19FD76 FIRE 1   : */
    text_flag(m, r, 0x0019FD82u, 0x0040222Cu, 0x10u);   /* 0x19FD82 */
    text_label(m, r, 0x0019FD92u, 0x0061CA86u, 0x001A203Eu, 0x7u);   /* 0x19FD92 FIRE 2   : */
    text_flag(m, r, 0x0019FD9Eu, 0x0040222Cu, 0x20u);   /* 0x19FD9E */
    text_label(m, r, 0x0019FDAEu, 0x0061CB06u, 0x001A2049u, 0x7u);   /* 0x19FDAE FIRE 3   : */
    text_flag(m, r, 0x0019FDBAu, 0x0040222Cu, 0x40u);   /* 0x19FDBA */
    text_label(m, r, 0x0019FDCAu, 0x0061CB86u, 0x001A2054u, 0x7u);   /* 0x19FDCA UP       : */
    text_flag(m, r, 0x0019FDD6u, 0x0040222Cu, 0x1u);   /* 0x19FDD6 */
    text_label(m, r, 0x0019FDE6u, 0x0061CC06u, 0x001A205Fu, 0x7u);   /* 0x19FDE6 DOWN     : */
    text_flag(m, r, 0x0019FDF2u, 0x0040222Cu, 0x2u);   /* 0x19FDF2 */
    text_label(m, r, 0x0019FE02u, 0x0061CC86u, 0x001A206Au, 0x7u);   /* 0x19FE02 LEFT     : */
    text_flag(m, r, 0x0019FE0Eu, 0x0040222Cu, 0x4u);   /* 0x19FE0E */
    text_label(m, r, 0x0019FE1Eu, 0x0061CD06u, 0x001A2075u, 0x7u);   /* 0x19FE1E RIGHT    : */
    text_flag(m, r, 0x0019FE2Au, 0x0040222Cu, 0x8u);   /* 0x19FE2A */
    text_label(m, r, 0x0019FE3Au, 0x0061C92Au, 0x001A2089u, 0xAu);   /* 0x19FE3A PLAYER 4 */
    text_label(m, r, 0x0019FE4Au, 0x0061C9A8u, 0x001A2028u, 0x7u);   /* 0x19FE4A START    : */
    text_flag(m, r, 0x0019FE5Au, 0x00402230u, 0x80u);   /* 0x19FE5A */
    text_label(m, r, 0x0019FE6Au, 0x0061CA28u, 0x001A2033u, 0x7u);   /* 0x19FE6A FIRE 1   : */
    text_flag(m, r, 0x0019FE7Au, 0x00402230u, 0x10u);   /* 0x19FE7A */
    text_label(m, r, 0x0019FE8Au, 0x0061CAA8u, 0x001A203Eu, 0x7u);   /* 0x19FE8A FIRE 2   : */
    text_flag(m, r, 0x0019FE9Au, 0x00402230u, 0x20u);   /* 0x19FE9A */
    text_label(m, r, 0x0019FEAAu, 0x0061CB28u, 0x001A2049u, 0x7u);   /* 0x19FEAA FIRE 3   : */
    text_flag(m, r, 0x0019FEBAu, 0x00402230u, 0x40u);   /* 0x19FEBA */
    text_label(m, r, 0x0019FECAu, 0x0061CBA8u, 0x001A2054u, 0x7u);   /* 0x19FECA UP       : */
    text_flag(m, r, 0x0019FEDAu, 0x00402230u, 0x1u);   /* 0x19FEDA */
    text_label(m, r, 0x0019FEEAu, 0x0061CC28u, 0x001A205Fu, 0x7u);   /* 0x19FEEA DOWN     : */
    text_flag(m, r, 0x0019FEFAu, 0x00402230u, 0x2u);   /* 0x19FEFA */
    text_label(m, r, 0x0019FF0Au, 0x0061CCA8u, 0x001A206Au, 0x7u);   /* 0x19FF0A LEFT     : */
    text_flag(m, r, 0x0019FF1Au, 0x00402230u, 0x4u);   /* 0x19FF1A */
    text_label(m, r, 0x0019FF2Au, 0x0061CD28u, 0x001A2075u, 0x7u);   /* 0x19FF2A RIGHT    : */
    r->a[2] = r->d[0];              /* 0x19FF2C movea.l D0,A2 */
    text_flag(m, r, 0x0019FF3Cu, 0x00402230u, 0x8u);   /* 0x19FF3C */
    r->a[7] += 0x14Au;
}

uint32_t Seg_19F708(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x19F708u:
        save_test_regs(m, r);
        r->a[3] = 0x00005666u;      /* jsr (A3) draws text */
        r->a[4] = 0x0061C000u;      /* text RAM base */
        r->d[2] = 0x0019F684u;      /* jsr (A0) tests one settings byte */
        r->d[3] = 0x00402224u;      /* PLAYER 1 switch byte */
        r->d[4] = 0x00402228u;      /* PLAYER 2 switch byte */
        r->d[6] = 0x00402220u;      /* cabinet switch byte */
        r->d[7] = 0x0040222Cu;      /* PLAYER 3 switch byte */
        Gfx_ClearAllLayers(m);
        r->d[5] = 0u;
        return poll_head(m, r);     /* 0x19F73E bra.w 0x19FFB4 */

    case 0x19F748u:
        /* 0x19F74C: the two fixed lines; 0x19F768 lea 0x14(SP) drops them */
        text_label(m, r, 0x0019F756u, 0x0061C098u, 0x001A1F28u, 0xAu);
        text_label(m, r, 0x0019F766u, 0x0061CE08u, 0x001A1F34u, 0xAu);
        r->a[7] += 0x14u;
        /* 0x19F76C: an all-erased settings block skips the selection switch */
        if (rd8(m, 0x00402221u) == 0xFFu && rd8(m, 0x00402225u) == 0xFFu &&
            rd8(m, 0x00402229u) == 0xFFu && rd8(m, 0x0040222Du) == 0xFFu &&
            rd8(m, 0x00402231u) == 0xFFu)
            goto draw_page;
        {
            /* 0x19F7A0: move.b D5,D0 ; extb.l D0 ; subq.l #4,D0 ; bhi 0x19F816.
             * BHI tests the subtraction's flags, so it is taken exactly when
             * the page index is above 4 (which a negative byte also is): those
             * frames skip the table and keep D5. D5 = 0..4 read the word table
             * at 0x19F7B4 through (0x19F7BC,PC,D0.w*2) with D0.w = D5-4 as a
             * sign-extended word index, i.e. entry D5, and the entry is a byte
             * offset from 0x19F7B4 for the (0x19F7B4,PC,D0.w) jmp. */
            r->d[0] = (uint32_t)(int32_t)(int8_t)(uint8_t)r->d[5];
            uint32_t page = r->d[0];
            r->d[0] -= 4u;                       /* subq.l #4,D0 */
            if (page <= 4u) {                    /* bhi not taken */
                uint32_t ea = 0x0019F7BCu +
                              2u * (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
                uint16_t off = rd16(m, ea);
                seg_setw(&r->d[0], off);         /* move.w (ea),D0 */
                switch (0x0019F7B4u + (uint32_t)(int16_t)off) {
                case 0x19F7BEu:   /* D5 == 0: 0x402225 bit 7 */
                    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, 0x00402225u);
                    seg_setw(&r->d[0], (uint16_t)((r->d[0] & 0xFFu) & 0x80u));
                    r->d[5] = (r->d[0] & 0xFFu) ? 0xFFFFFFFFu : 1u;
                    break;
                case 0x19F7CEu:   /* D5 == 1: 0x402225 bit 7 */
                    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, 0x00402225u);
                    seg_setw(&r->d[0], (uint16_t)((r->d[0] & 0xFFu) & 0x80u));
                    r->d[5] = (r->d[0] & 0xFFu) ? 0xFFFFFFFFu : 2u;
                    break;
                case 0x19F7DEu:   /* D5 == 2: 0x402225 bit 7 */
                    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, 0x00402225u);
                    seg_setw(&r->d[0], (uint16_t)((r->d[0] & 0xFFu) & 0x80u));
                    r->d[5] = (r->d[0] & 0xFFu) ? 0xFFFFFFFFu : 3u;
                    break;
                case 0x19F7EEu:   /* D5 == 3: 0x402221 bit 1 */
                    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, 0x00402221u);
                    seg_setw(&r->d[0], (uint16_t)((r->d[0] & 0xFFu) & 0x02u));
                    r->d[5] = (r->d[0] & 0xFFu) ? 0xFFFFFFFFu : 4u;
                    break;
                case 0x19F7FEu:   /* D5 == 4 -> 5: 0x402225 bit 7 */
                    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, 0x00402225u);
                    seg_setw(&r->d[0], (uint16_t)((r->d[0] & 0xFFu) & 0x80u));
                    if (r->d[0] & 0xFFu) {
                        r->d[5] = 0xFFFFFFFFu;      /* 0x19F814 moveq #-1,D5 */
                    } else {
                        r->d[5] = 5u;
                        Gfx_ClearAllLayers(m);      /* 0x19F80C */
                    }
                    break;
                }
            }
        }
    draw_page:
        if ((r->d[5] & 0xFFu) == 5u) {
            draw_confirm_page(m, r);
            commit_switch_flags(m, r);   /* 0x19FF42 */
        } else {
            draw_switch_page(m, r);
        }
        return poll_head(m, r);          /* 0x19FA74 / 0x19FFB4 */
    }
    return SEG_LEAVE;
}
