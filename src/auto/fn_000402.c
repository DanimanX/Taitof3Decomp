/* ROM 0x000402: (abort capture fnt_, recipes17cw2.tsv): F3 RAM test of the eleven boot
 * blocks, ending in a jump to the caller's continuation (A4).
 *
 * Each block loads A0 = base, A1 = message, D0 = byte count and A6 = the
 * continuation, then jumps into one of three identical loops - byte 0x000522,
 * word 0x000554, long 0x000586 - chosen by the access width:
 *
 *   0x000522  D3=3 ; D1b=0xFF
 *   0x000554  D3=3 ; D1w=0xFFFF
 *   0x000586  D3=3 ; D1=0xFFFFFFFF (moveq #-1)
 *   loop: move.b #0,0x4A0000 (IO strobe) ; store D1 ; read back D2 ;
 *         cmp ; beq ok ; <failure: A2 = the dead "bra" after the error jump,
 *         jmp to the per-width diagnostic>
 *   ok:   subi 0x5555/0x55/0x55555555 from D1 ; dbf D3 (four patterns)
 *         A0 += size ; D0 -= size ; bne loop ; jmp (A6)
 *
 * The diagnostics (0x0005B8 byte, 0x00067E word, 0x000744 long) reload the
 * sound bank, the line RAM, the character font and every layer, then print four
 * label strings and the address / write / read words into text RAM
 * 0x61C314..0x61C638, and spin forever at 0x00067A / 0x000740 / 0x000806 (the
 * jmp (A2) behind each bra.s is unreachable).
 *
 * Rows: 0x000402 0x620000 "LINE SET RAM" 0x1000 W ; 0x00041C 0x622000 "LINE
 * DATA RAM" 0xE000 W ; 0x000436 0x600000 "OBJECT RAM" 0x10000 W ; 0x000450
 * 0x400000 "WORK RAM" 0x20000 B ; 0x00046A 0x610000 "SCR0" 0x1000 W ;
 * 0x000484 0x611000 "SCR1" 0x1000 W ; 0x00049E 0x612000 "SCR2" 0x1000 W ;
 * 0x0004B8 0x613000 "SCR3" 0x1000 W ; 0x0004D2 0x61C000 "MASK RAM" 0x2000 W ;
 * 0x0004EC 0x621000 "PIVOT PORT" 0x1000 W ; 0x000506 0x630000 "PIVOT RAM"
 * 0x10000 W. No instruction inside 0x000402..0x00051E uses the long loop at
 * 0x000586, so 0x000744 stays unreachable from the boot path; it is translated
 * for the register-level test just the same.
 *
 * No capture exists. Under the f3mem model 0x630000 (pivot RAM) is outside
 * every emulated array, so the last block reads back 0 and takes the PIVOT RAM
 * ERROR path, exactly as the ROM would on a dead pivot RAM.
 *
 * The three handlers push A1, A0, D1 and D2 for the display only AFTER the
 * four init calls, so the screen shows the clobbered values, not the block
 * message and the failing pattern: 0x0014DA and
 * 0x005B0C overwrite D0-D3 and A0/A1 (the shared C helpers do not model those
 * register side effects, so both are translated here at register level), and
 * A1 at 0x0005D2 is 0x621000 from 0x005B0C -- the first line therefore prints
 * the line-set RAM the routine just filled, not the block message. 0x005A3A
 * and 0x005A12 save/restore every register they touch. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString 0x005666 */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers 0x005A12, Gfx_DrawHex
                                     * 0x005690 */

enum { WIDTH_BYTE, WIDTH_WORD, WIDTH_LONG };

/* 0x005666 with the caller's three arguments and its move/link frame. */
static void draw_string(const f3mem_t *m, f3regs_t *r, uint32_t src,
                        uint32_t dst, uint16_t col, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, col);                       /* move.w #7,-(A7) */
    push_long(m, r, dst);                       /* pea.l */
    push_long(m, r, src);
    push_long(m, r, ret);                       /* jsr return address */
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    r->d[0] = Text_DrawString(m, src, dst, col);
    r->a[7] = sp;                               /* lea 0xa(A7),A7 */
}

/* 0x005690 with the caller's four arguments and its move/link frame. */
static void draw_hex(const f3mem_t *m, f3regs_t *r, uint32_t val,
                     uint16_t digits, uint32_t dst, uint16_t col, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, col);
    push_long(m, r, dst);
    push_word(m, r, digits);
    push_long(m, r, val);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[0] = Gfx_DrawHex(m, val, digits, dst, col);
    r->a[7] = sp;                               /* lea 0xc(A7),A7 */
}

/* ROM 0x0014E8 / 0x001504: relay one sound bank. Index = (D0 & 0xFF) modulo
 * the table's divisor, then D3+1 blocks of 16-bit pairs: each pair is a source
 * offset and a destination offset, and the long at A1+source is copied to
 * 0x440000+dest (the sound CPU window, unmapped here: the writes are dropped
 * exactly as f3mem drops them). Register-accurate, because the diagnostic
 * prints D0-D3/A0 afterwards. */
static void sound_relay(const f3mem_t *m, uint32_t table, uint32_t src,
                        uint16_t divisor, f3regs_t *r)
{
    uint32_t n = r->d[0] & 0xFFu;                             /* 0x0014F4 andi.l */
    uint16_t quo = (uint16_t)(n / divisor);                   /* 0x0014FA divu.w */
    uint16_t rem = (uint16_t)(n % divisor);
    uint32_t a2, a0 = table;                                  /* 0x0014E8 lea */

    r->d[0] = ((uint32_t)quo << 16) | rem;                    /* 0x0014FE swap D0 */
    a2 = rd32(m, a0 + (uint32_t)rem * 8u);                    /* 0x001520 movea.l */
    r->a[0] = a0;
    r->a[1] = src;                                            /* 0x0014EE lea */
    r->a[2] = a2;
    r->d[3] = rd32(m, a0 + 4u + (uint32_t)rem * 8u);          /* 0x001524 move.l */
    if ((int32_t)r->d[3] < 0)                                 /* 0x001528 bmi 0x00154C */
        return;

    r->a[0] = 0x00440000u;                                    /* 0x00152A lea */
    do {
        uint16_t d2 = (uint16_t)(rd16(m, a2) - 1u);           /* 0x001530/0x001532 */
        a2 += 2u;
        r->d[2] = set_word(r->d[2], d2);
        r->d[0] = 0;                                          /* 0x001534 moveq #0 */
        for (;;) {
            r->d[0] = rd16(m, a2);                            /* 0x001536 move.w (A2)+ */
            a2 += 2u;
            r->d[1] = set_word(r->d[1], rd16(m, a2));         /* 0x001538 */
            a2 += 2u;
            r->a[2] = a2;
            /* 0x00153A move.l (A1,D0.l),(A0,D1.w) */
            wr32(m, 0x00440000u + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[1],
                 rd32(m, r->a[1] + r->d[0]));
            r->d[0] += 4u;                                    /* 0x001540 addq.l #4 */
            r->d[1] = set_word(r->d[1], (uint16_t)((uint16_t)r->d[1] + 4u)); /* 0x001542 */
            d2--;                                             /* 0x001544 dbra D2 */
            if (d2 == 0xFFFFu)
                break;
            r->d[2] = set_word(r->d[2], d2);
        }
        r->d[2] = set_word(r->d[2], d2);
        r->d[3] = set_word(r->d[3], (uint16_t)((uint16_t)r->d[3] - 1u)); /* 0x001548 dbra D3 */
    } while ((uint16_t)r->d[3] != 0xFFFFu);
}

/* ROM 0x0014DA: bank 0 (index (D0 & 0xFF) % 6 from table 0x16890, source
 * 0x16900) then bank 1 ((D0 & 0xFF) % 8 from 0x168C0, source 0x1FB40) -- D0 is
 * 0 for the second call, which leaves D0-D3/A0-A2 from the bank-1 loop.
 *
 * Not Kernel_SoundBank0_0014DA (src/kernel/fn_010420.c): that helper writes
 * 0x440000 + (uint16_t)D1, while the ROM's `move.l (A1,D0.l),(A0,D1.w)` uses
 * a sign-extended word index, so an entry offset >= 0x8000 lands at
 * 0x438000+n -- the work-RAM mirror -- instead of 0x448000+n. Bank 0 does hold
 * such an offset, so the helper and the ROM differ over real work RAM here. */
void SoundBank0_0014DA_Regs(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = 0;                                              /* 0x0014DA moveq #0,D0 */
    sound_relay(m, 0x00016890u, 0x00016900u, 6u, r);
    r->d[0] = 0;                                              /* 0x0014E0 moveq #0,D0 */
    sound_relay(m, 0x000168C0u, 0x0001FB40u, 8u, r);
}

/* ROM 0x005BE8: D1 = 0xE7, fill that many +1 words at A1 with D0, A1 += 0x30. */
static uint32_t line_fill(const f3mem_t *m, uint32_t a1, uint16_t d0, f3regs_t *r)
{
    uint16_t d1 = 0x00E7u;                                    /* 0x005BE8 move.w #0xe7 */
    do {
        wr16(m, a1, d0);                                      /* 0x005BEC move.w D0,(A1)+ */
        a1 += 2u;
        d1--;                                                 /* 0x005BEE dbra D1 */
    } while (d1 != 0xFFFFu);
    r->d[1] = set_word(r->d[1], d1);
    return a1 + 0x30u;                                        /* 0x005BF2 lea (0x30,A1) */
}

/* ROM 0x005B0C: rebuild the line-scroll tables at 0x624000+scroll and the
 * 0x620000 row table from the 32-word ROM table at 0x5BF8, then set the two
 * line-RAM control words. Register-accurate (D0-D3, A0, A1), see above. */
static void init_line_ram_005B0C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t a0 = 0x00005BF8u;                                /* 0x005B0C lea (0xEA,PC) */
    uint32_t a1 = 0x00624000u;                                /* 0x005B10 */
    uint16_t d0 = (rd8(m, a5 - 0x5D58u) == 0u) ? 0x0030u : 0x0000u; /* 0x005B1A */
    uint16_t d3 = 7u;                                         /* 0x005B16 moveq #7 */

    a1 += d0;                                                 /* 0x005B22 adda.l D0,A1 */
    wr16(m, a5 - 0x5D56u, d0);                                /* 0x005B24 */
    r->a[0] = a0;
    r->a[1] = a1;
    do {
        for (unsigned i = 0; i < 4u; ++i) {                   /* 0x005B28..0x005B3C */
            d0 = rd16(m, a0);                                 /* 0x005B28 move.w (A0)+,D0 */
            a0 += 2u;
            r->d[0] = set_word(r->d[0], d0);
            a1 = line_fill(m, a1, d0, r);
        }
        a1 += 0x800u;                                         /* 0x005B40 lea (0x800,A1) */
        r->a[1] = a1;
        d3--;                                                 /* 0x005B44 dbra D3 */
    } while (d3 != 0xFFFFu);
    r->d[3] = set_word(r->d[3], d3);
    r->a[0] = a0;                                             /* A0 past the 32 table words */

    a1 = 0x00620000u;                                         /* 0x005B48 */
    d3 = 7u;                                                  /* 0x005B4E */
    uint32_t d = 0x00624000u;                                 /* 0x005B50 move.l */
    d >>= 8;                                                  /* 0x005B56 lsr.l */
    d >>= 4;                                                  /* 0x005B58 */
    d <<= 8;                                                  /* 0x005B5A */
    d <<= 2;                                                  /* 0x005B5C */
    d = (d & 0xFFFF0000u) | (uint16_t)((uint16_t)d + 0x000Fu); /* 0x005B5E addi.w */
    d = (d & 0xFFFF0000u) | (uint16_t)((uint16_t)d & 0x7FFFu); /* 0x005B62 andi.w */
    r->a[1] = a1;
    r->d[0] = set_word(r->d[0], (uint16_t)d);
    do {
        uint16_t d1 = 0x00FFu;                                /* 0x005B66 move.w #0xff */
        do {
            wr16(m, a1, (uint16_t)d);                         /* 0x005B6A move.w D0,(A1)+ */
            a1 += 2u;
            d1--;                                             /* 0x005B6C dbra D1 */
        } while (d1 != 0xFFFFu);
        r->a[1] = a1;
        r->d[1] = set_word(r->d[1], d1);
        /* 0x005B70 ror.l #4,D2: eight of them rotate D2 by 32 bits, i.e.
         * leave it as 0x0014DA left it (0xFFFF in the low word). */
        d = (d & 0xFFFF0000u) | (uint16_t)((uint16_t)d + 0x0400u); /* 0x005B72 addi.w */
        r->d[0] = set_word(r->d[0], (uint16_t)d);
        d3--;                                                 /* 0x005B76 dbra D3 */
    } while (d3 != 0xFFFFu);
    r->d[3] = set_word(r->d[3], d3);

    if (rd16(m, a5 - 0x5D58u) == 0u) {                        /* 0x005B7A tst.w */
        wr16(m, 0x00626000u, 0x0800u);                        /* 0x005B82 */
        r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, 0x006275FEu) | 0x0080u)); /* 0x005B8A */
        wr16(m, 0x006275FEu, (uint16_t)r->d[0]);              /* 0x005B94 */
        wr16(m, 0x006257FEu, 0x6E00u);                        /* 0x005B9A */
        r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, 0x006247FEu) | 0x8000u)); /* 0x005BA2 */
        wr16(m, 0x006247FEu, (uint16_t)r->d[0]);              /* 0x005BAC */
        return;                                               /* 0x005BB2 bra 0x005BE6 */
    }
    wr16(m, 0x006261FEu, 0x0800u);                            /* 0x005BB6 */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, 0x00627400u) | 0x0080u)); /* 0x005BBE */
    wr16(m, 0x00627400u, (uint16_t)r->d[0]);                  /* 0x005BC8 */
    wr16(m, 0x00625600u, 0x6E00u);                            /* 0x005BCE */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, 0x00624600u) | 0x8000u)); /* 0x005BD6 */
    wr16(m, 0x00624600u, (uint16_t)r->d[0]);                  /* 0x005BE0 */
}

/* 0x0005B8 / 0x00067E / 0x000744: the per-width failure screen. The three
 * differ only in the hex widths and in the D1/D2 text-RAM columns (byte
 * 8/2/2 at 0x61C538/0x61C638, word 4/4/4 at 0x61C530/0x61C630, long 8/8/8 at
 * 0x61C52C/0x61C62C); the trailing bra.s is the endless spin. */
static void ram_error_report(const f3mem_t *m, f3regs_t *r, int width)
{
    uint16_t a0_digits = 8u, d_digits = (width == WIDTH_LONG) ? 8u : 2u;
    uint32_t d1_dst = 0x0061C538u, d2_dst = 0x0061C638u;

    if (width == WIDTH_WORD) {
        a0_digits = 4u;
        d_digits = 4u;
        d1_dst = 0x0061C530u;
        d2_dst = 0x0061C630u;
    } else if (width == WIDTH_LONG) {
        d1_dst = 0x0061C52Cu;
        d2_dst = 0x0061C62Cu;
    }

    /* 0x0005B8/0x0005BC/0x0005C0/0x0005C4: reload the machine. The last two
     * calls restore every register; the first two overwrite D0-D3 and A0-A2,
     * so A1 (the message pointer) is 0x621000 by the time it is pushed. */
    SoundBank0_0014DA_Regs(m, r);
    init_line_ram_005B0C(m, r);
    (void)Fn_005A3A(m, r);
    Gfx_ClearAllLayers(m);

    /* The three handlers are identical apart from the hex widths and their
     * own JSR return addresses: 0x0005DA.. for bytes, 0x0006A0.. for words,
     * 0x000766.. for longs. */
    uint32_t ret = (width == WIDTH_BYTE) ? 0x0005DAu
                 : (width == WIDTH_WORD) ? 0x0006A0u : 0x000766u;
    draw_string(m, r, r->a[1], 0x0061C314u, 7u, ret);
    draw_string(m, r, 0x000008CDu, 0x0061C414u, 7u, ret + 0x1Au);
    draw_string(m, r, 0x000008E3u, 0x0061C514u, 7u, ret + 0x34u);
    draw_string(m, r, 0x000008F9u, 0x0061C614u, 7u, ret + 0x4Eu);
    draw_hex(m, r, r->a[0], a0_digits, 0x0061C42Cu, 7u, ret + 0x68u);
    draw_hex(m, r, r->d[1], d_digits, d1_dst, 7u, ret + 0x82u);
    draw_hex(m, r, r->d[2], d_digits, d2_dst, 7u, ret + 0x9Cu);
    /* 0x00067A / 0x000740 / 0x000806: bra.s * -- the ROM never leaves. */
}

/* 0x000522 (byte), 0x000554 (word), 0x000586 (long). Returns 1 on the first
 * mismatch, with D1 = the pattern, D2 = the value read back, A0/D0 = the
 * current address/count and A2 = the loop-restart address the ROM loads.
 *
 * The outer loop restarts at the block entry, so D3 and D1 are reloaded on
 * every pass: the last pattern stored in each cell is 0 (0xFF/0xAA/0x55/0x00
 * for bytes, 0xFFFF/0xAAAA/0x5555/0 for words, 0xFFFFFFFF/0xAAAAAAAA/
 * 0x55555555/0 for longs), which is why the test leaves the block zeroed. */
static int test_block(const f3mem_t *m, f3regs_t *r, int width)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a2 = (width == WIDTH_BYTE) ? 0x00000542u
               : (width == WIDTH_WORD) ? 0x00000574u : 0x000005A4u;
    uint32_t step = (width == WIDTH_BYTE) ? 1u
                  : (width == WIDTH_WORD) ? 2u : 4u;

    for (;;) {
        uint16_t d3 = 3u;                       /* 0x000522/0x000554/0x000586 moveq #3 */
        if (width == WIDTH_BYTE)
            r->d[1] = (r->d[1] & 0xFFFFFF00u) | 0x00FFu; /* 0x000524 move.b #-1 */
        else if (width == WIDTH_WORD)
            r->d[1] = set_word(r->d[1], 0xFFFFu);        /* 0x000556 move.w #-1 */
        else
            r->d[1] = 0xFFFFFFFFu;                       /* 0x000588 moveq #-1 */

        do {
            /* 0x000528/0x00055A/0x00058A write.b #0 to the F3 control port
             * 0x4A0000, the watchdog strobe: write-only MMIO, not modelled (as
             * everywhere else in src/), and it changes no compared byte. It is
             * skipped rather than issued because f3mem logs every access
             * outside RAM/ROM: the byte block below runs 0x8000 of these per
             * call, one log line each. */
            if (width == WIDTH_BYTE) {
                uint8_t d1 = (uint8_t)r->d[1];
                wr8(m, a0, d1);                 /* 0x000530 move.b D1,(A0) */
                uint8_t d2 = rd8(m, a0);        /* 0x000532 move.b (A0),D2 */
                r->d[2] = (r->d[2] & 0xFFFFFF00u) | d2;
                if (d2 != d1)                   /* 0x000534 cmp.b / beq */
                    goto mismatch;
                r->d[1] = (r->d[1] & 0xFFFFFF00u)
                        | (uint8_t)(d1 - 0x55u);         /* 0x000544 subi.b */
            } else if (width == WIDTH_WORD) {
                uint16_t d1 = (uint16_t)r->d[1];
                wr16(m, a0, d1);                /* 0x000562 move.w D1,(A0) */
                uint16_t d2 = rd16(m, a0);      /* 0x000564 move.w (A0),D2 */
                r->d[2] = set_word(r->d[2], d2);
                if (d2 != d1)                   /* 0x000566 cmp.w / beq */
                    goto mismatch;
                r->d[1] = set_word(r->d[1], (uint16_t)(d1 - 0x5555u)); /* 0x000576 */
            } else {
                uint32_t d1 = r->d[1];
                wr32(m, a0, d1);                /* 0x000592 move.l D1,(A0) */
                uint32_t d2 = rd32(m, a0);      /* 0x000594 move.l (A0),D2 */
                r->d[2] = d2;
                if (d2 != d1)                   /* 0x000596 cmp.l / beq */
                    goto mismatch;
                r->d[1] = d1 - 0x55555555u;     /* 0x0005A6 subi.l */
            }
            d3--;                               /* 0x000548/0x00057A/0x0005AC dbf */
        } while (d3 != 0xFFFFu);
        r->d[3] = set_word(r->d[3], d3);

        a0 += step;                             /* 0x00054C/0x00057E/0x0005B0 */
        d0 -= step;                             /* 0x00054E/0x000580/0x0005B2 */
        r->a[0] = a0;
        r->d[0] = d0;
        if (d0 == 0u)                           /* 0x000550/0x000582/0x0005B4 */
            return 0;
        continue;

    mismatch:                                   /* 0x000538/0x00056A/0x00059A */
        r->a[0] = a0;
        r->d[0] = d0;
        r->a[2] = a2;
        return 1;
    }
}

/* One block: the lea/move.l prologue of 0x000402..0x000506 followed by the
 * loop; returns 1 when the diagnostic was entered (the ROM then spins). */
static int run_block(const f3mem_t *m, f3regs_t *r, uint32_t base,
                     uint32_t message, uint32_t count, int width)
{
    r->a[0] = base;
    r->a[1] = message;
    r->d[0] = count;
    if (test_block(m, r, width) == 0)
        return 0;
    ram_error_report(m, r, width);
    return 1;
}

/* The eleven blocks of 0x000402..0x00051E. Returns 1 when a block failed (the
 * ROM is spinning in that block's diagnostic). Shared with Fn_010000, which
 * enters this code through jmp 0x000402 and only reaches 0x01008A when it
 * returns 0. */
/* base, message (A1), byte count (D0), continuation (A6), width. A6 is the
 * next block, 0x000520 (jmp (A4)) after the last one. */
static const struct {
    uint32_t base, message, count, cont;
    int width;
} ram_blocks[] = {
    { 0x00620000u, 0x00000875u, 0x00001000u, 0x0000041Cu, WIDTH_WORD },
    { 0x00622000u, 0x00000888u, 0x0000E000u, 0x00000436u, WIDTH_WORD },
    { 0x00600000u, 0x00000819u, 0x00010000u, 0x00000450u, WIDTH_WORD },
    { 0x00400000u, 0x0000080Au, 0x00020000u, 0x0000046Au, WIDTH_BYTE },
    { 0x00610000u, 0x0000082Au, 0x00001000u, 0x00000484u, WIDTH_WORD },
    { 0x00611000u, 0x00000839u, 0x00001000u, 0x0000049Eu, WIDTH_WORD },
    { 0x00612000u, 0x00000848u, 0x00001000u, 0x000004B8u, WIDTH_WORD },
    { 0x00613000u, 0x00000857u, 0x00001000u, 0x000004D2u, WIDTH_WORD },
    { 0x0061C000u, 0x00000866u, 0x00002000u, 0x000004ECu, WIDTH_WORD },
    { 0x00621000u, 0x0000089Cu, 0x00001000u, 0x00000506u, WIDTH_WORD },
    { 0x00630000u, 0x000008ADu, 0x00010000u, 0x00000520u, WIDTH_WORD },
};

/* Runs blocks [first, last); returns 1 as soon as one fails (the ROM then
 * spins in that block's diagnostic via ram_error_report). */
static int run_blocks(const f3mem_t *m, f3regs_t *r, unsigned first,
                      unsigned last)
{
    for (unsigned i = first; i < last; ++i) {
        r->a[6] = ram_blocks[i].cont;
        if (run_block(m, r, ram_blocks[i].base, ram_blocks[i].message,
                      ram_blocks[i].count, ram_blocks[i].width) != 0)
            return 1;
    }
    return 0;
}

int RamTest_000402(const f3mem_t *m, f3regs_t *r)
{
    return run_blocks(m, r, 0, sizeof ram_blocks / sizeof ram_blocks[0]);
}

int Fn_000402(const f3mem_t *m, f3regs_t *r)
{
    (void)RamTest_000402(m, r);
    /* 0x000520 jmp (A4): the ROM continues at the caller's A4 (0x01008A
     * inside the reset code). Modelled as the return. */
    return 0;
}

/* ROM 0x000402 stopped at 0x000450, the capture stop point (a capture cannot
 * run the whole test: see below).
 *
 * Why 0x000450: the third block (0x00400000, byte width) is the one that wipes
 * work RAM 0x400000-0x41FFFF -- the region the harness compares, and the stack
 * the injected call has to return to -- so the capture stops at that block's
 * prologue: PC = 0x000450, i.e. right after block 2 (0x600000 sprite RAM) set
 * A6 = 0x000450 and the word loop reached `jmp (A6)`. That leaves work RAM as
 * the pre dump had it, so the ABORT capture can take more than one pair, and
 * it compares the two line-RAM blocks (0x620000-0x620FFF, 0x622000-0x62FFFF)
 * and the whole sprite-RAM block (0x600000-0x60FFFF) against the capture.
 *
 * What it cannot cover and why: blocks 3..10 write the state the capture needs
 * (work RAM, 0x610000-0x61FFFF, 0x621000) away, and the last block (0x630000,
 * 0x000506) tests the F3 pivot RAM, which f3mem does not map at all -- the
 * model reads 0 back there and takes the 0x000740 PIVOT RAM ERROR spin, while
 * MAME's f3_map maps 0x630000-0x63FFFF read/write and the real boot passes it.
 * A second capture at F3_EXIT=000506 would cover blocks 3..10 in a single pair
 * (the work-RAM block zeroes the stack the abort returns through).
 *
 * Registers at the stop are the word loop's: A0 = 0x610000 (the 0x10000 bytes
 * of block 2 consumed at 2 bytes a pass), A1 = 0x00000819 (block 2's message),
 * A6 = 0x000450, D0 = 0, D3 = 0xFFFF from the dbra, D2 = 0x0000 (the last
 * value read back) and D1 = 0xAAAB -- the four passes store 0xFFFF, 0xAAAA,
 * 0x5555 and 0x0000 and the subi.w #0x5555 at 0x000576 also runs after the
 * last one, so each cell ends zero while D1 is left at 0 - 0x5555. The high
 * halves of D1/D2/D3 are the caller's (move.w/subi.w keep them). */
int Fn_000402_Early(const f3mem_t *m, f3regs_t *r)
{
    if (run_blocks(m, r, 0, 3) != 0)
        return -1;   /* the diagnostic is not the captured stop */
    return 0;        /* 0x000450 lea 0x400000,A0 */
}
