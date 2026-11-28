/* ROM 0x13F436: (recipe capture): "BOSS TEST MODE" task body.
 * Pre-yield part: set the test-mode byte A5+0x1D1C to 0xFF, drop the
 * word 0x0060 into the sound upload buffer 0x440000, clear playfield layer 3
 * (0x0058AE), rebuild the line RAM tables (0x005B0C), raise A5-0x144C bit 1 and
 * clear A5+0x1D36 bits 2 and 7, then yield at the TRAP #5 at 0x13F462.
 * Task13F436_Frame below runs one pass of the loop the ROM resumes with at
 * 0x13F464 (the loop also re-enters its own preamble at 0x13F444).
 *
 * One pass: adjust the stage select with the accelerating counter (0x19FFF0,
 * bounds 0..0x1B, step 1, buttons from the sign-extended byte A5-0x5DDC), store
 * it back to A5+0x32DC, draw the banner, the "[ BOSS TEST MODE ]" and "ROUND:"
 * labels (attributes 5 and 8), the two-digit decimal stage number at the end of
 * "ROUND:" and the stage name 0x13F6A0[stage*0x19] at 0x61C910. When pad bit 4
 * (A5-0x5DDB = 0x402225, the EXIT bit) is set the pass ends at the 0x13F462
 * yield; otherwise the transition runs: reset the staging block (0x13CAAE),
 * seed the A5+0x87A scroll accumulator (0x004216), add its difficulty-weighted
 * sum (table 0x40888C, 0x807AE and 0x547AE scales, arithmetic shift by 8), clear
 * layer 3, redraw the banner, start timed effect B (0x13CB3A), clear A5-0x144C
 * bit 1, wait 0x14 frames, run the stage-end sequence (0x13F5B8) and the
 * jingle, then loop. The frame yields are collapsed the way
 * src/auto/fn_13F5B8.c does it: the frame returns 1 wherever the ROM would
 * yield (0x13F462, or inside 0x13F5B8), so the caller reschedules the task.
 *
 * Scratch registers at a yield are whatever the last callee left; only the
 * ones the ROM sets here are modelled (D0/A0 around 0x19FFF0, D1 = stage*0x19,
 * D7 = 0xFFFF after the 0x14-frame wait). */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../audio/sound_cmd.h"
#include "../common/fn_leaf1.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (0x005666) */
#include "../objects/obj_shared_ext.h" /* Sound_PreCmdHook (0x13F238) */
#include "../video/sprite.h"

/* 0x13F436 up to the TRAP #5 at 0x13F462. The register leftovers of the two
 * gfx calls are part of the pair: 0x0058AE saves/restores D0-D3/A0, and
 * 0x005B0C leaves D0 = 0x0018xxxx, D1/D3 = 0xFFFF, A0 = 0x5C38 and
 * A1 = 0x621000 (Gfx_InitLineRam_Regs). fnt_13F436's post image carries
 * exactly those values, which is why this runs the register-aware variant. */
static void init_13F436(const f3mem_t *m, f3regs_t *r)
{
    wr8(m, F3_A5 + 0x1D1Cu, 0xFFu);                 /* 0x13F436 st.b */
    wr32(m, 0x00440000u, 0x0060u);                  /* 0x13F43A move.l #0x60 */
    Gfx_ClearPf3(m);                                /* 0x13F444 jsr 0x0058AE */
    Gfx_InitLineRam_Regs(m, r);                     /* 0x13F44A jsr 0x005B0C */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x02u)); /* 0x13F450 bset.b #1 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0xFBu)); /* 0x13F456 bclr.b #2 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0x7Fu)); /* 0x13F45C bclr.b #7 */
}

int Fn_13F436(const f3mem_t *m, f3regs_t *r)
{
    init_13F436(m, r);
    return 0;                                       /* 0x13F462 trap #5 */
}

int Task13F436_Frame(const f3mem_t *m, f3regs_t *r)
{
    /* 0x13F464/0x13F468: predecrement MOVEM saves D0 then A0. */
    push_long(m, r, r->d[0]);
    push_long(m, r, r->a[0]);

    /* 0x13F46C..0x13F484: arguments (lower, upper, step, value, buttons) for
     * 0x19FFF0, then the call; the callee pops its own frame and return PC. */
    push_word(m, r, 1u);                            /* 0x13F46C move.w #1 */
    push_word(m, r, 0x1Bu);                         /* 0x13F470 move.w #0x1B */
    push_word(m, r, 0u);                            /* 0x13F474 move.w #0 */
    push_word(m, r, rd16(m, F3_A5 + 0x32DCu));      /* 0x13F478 move.w (0x32DC,A5) */
    push_word(m, r, (uint16_t)(int16_t)(int8_t)rd8(m, F3_A5 - 0x5DDCu)); /* 0x13F47C..0x13F482 */
    push_long(m, r, 0x0013F48Au);
    Fn_19FFF0(m, r);                                /* 0x13F484 jsr 0x19FFF0 */
    r->a[7] += 0x0Au;                               /* 0x13F48A lea (0xa,SP),SP */

    r->a[0] = rd32(m, r->a[7]);                     /* 0x13F48E movem.l (SP)+,{A0} */
    r->a[7] += 4u;
    wr16(m, F3_A5 + 0x32DCu, (uint16_t)r->d[0]);    /* 0x13F492 move.w D0,(0x32DC,A5) */
    r->d[0] = rd32(m, r->a[7]);                     /* 0x13F496 movem.l (SP)+,{D0} */
    r->a[7] += 4u;

    Stage_DrawBanner13D11E(m);                      /* 0x13F49A jsr 0x13D11E */

    /* 0x13F4A0..0x13F4B6: "[ BOSS TEST MODE ]" at 0x13F59E, attribute 5. */
    (void)Text_DrawString(m, 0x0013F59Eu, 0x0061C416u, 5u);
    /* 0x13F4BA..0x13F4D0: "ROUND:" at 0x13F5B1, attribute 8; the end pointer it
     * returns is the destination of the decimal below. */
    r->d[0] = Text_DrawString(m, 0x0013F5B1u, 0x0061C7A0u, 8u);

    /* 0x13F4D4..0x13F4EC: the stage number, two digits, attribute 7. */
    r->d[1] = (r->d[1] & 0xFFFF0000u) |
              (uint16_t)rd16(m, F3_A5 + 0x32DCu);   /* 0x13F4D6 move.w (0x32DC,A5),D1 */
    (void)Gfx_DrawDecimal(m, r->d[1], 2u, r->d[0], 7u); /* 0x13F4DA..0x13F4E6 jsr 0x571A */

    /* 0x13F4F0 lea 0x13F6A0,A0 ; 0x13F4F6 muls.w #0x19,D1 ; 0x13F4FA lea (A0,D1.w) ;
     * 0x13F4FE..0x13F510: the stage name, attribute 7. */
    {
        int16_t stage = (int16_t)rd16(m, F3_A5 + 0x32DCu);
        uint16_t idx = (uint16_t)(stage * 0x19);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | idx;
        (void)Text_DrawString(m, 0x0013F6A0u + (uint32_t)(int32_t)(int16_t)idx,
                              0x0061C910u, 7u);
    }

    /* 0x13F514 btst.b #4,(A5-0x5DDB) ; 0x13F51A bne.w 0x13F462: the EXIT bit
     * sends the loop straight back to its yield. */
    if (rd8(m, F3_A5 - 0x5DDBu) & 0x10u)
        return 1;                                   /* 0x13F462 trap #5 */

    Leaf_13CAAE(m, r);                              /* 0x13F51E jsr 0x13CAAE */
    (void)Fn_004216(m, r);                          /* 0x13F524 jsr 0x004216 */

    /* 0x13F52A..0x13F53C: sum (A5+0x32DE - 1) words of the 0x40888C table. */
    {
        int16_t count = (int16_t)rd16(m, F3_A5 + 0x32DEu);   /* 0x13F52A */
        uint32_t a0 = F3_A5 + 0x88Cu;                        /* 0x13F52E */
        uint16_t d1 = 0;                                     /* 0x13F532 moveq #0 */
        for (;;) {
            count = (int16_t)(count - 1);                    /* 0x13F534 subq.w #1 */
            if (count <= 0)                                  /* 0x13F536 ble */
                break;
            d1 = (uint16_t)(d1 + rd16(m, a0));               /* 0x13F53A add.w (A0)+ */
            a0 += 2u;
        }
        /* 0x13F53E muls.l #0x807AE ; 0x13F546 asr.l #8 ; 0x13F548 add.l */
        wr32(m, F3_A5 + 0x087Au,
             rd32(m, F3_A5 + 0x087Au) + (uint32_t)((int32_t)((uint32_t)d1 * 0x807AEu) >> 8));
        /* 0x13F54C moveq #0,D1 ; 0x13F54E move.w (A0),D1w ; muls.l #0x547AE ;
         * 0x13F558 asr.l #8 ; 0x13F55A add.l - D1 keeps the shifted product. */
        d1 = rd16(m, a0);
        wr32(m, F3_A5 + 0x087Au,
             rd32(m, F3_A5 + 0x087Au) + (uint32_t)((int32_t)((uint32_t)d1 * 0x547AEu) >> 8));
        r->d[1] = (uint32_t)((int32_t)((uint32_t)d1 * 0x547AEu) >> 8);
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)count;  /* D0.w after the dbf test */
        r->a[0] = a0;
    }

    Gfx_ClearPf3(m);                                /* 0x13F55E jsr 0x0058AE */
    Stage_DrawBanner13D11E(m);                      /* 0x13F564 jsr 0x13D11E */
    (void)Fn_13CB3A(m, r);                          /* 0x13F56A jsr 0x13CB3A */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xFDu)); /* 0x13F570 bclr.b #1 */

    /* 0x13F576 move.w #0x14,D7 ; subq.w #1 ; 0x13F57C trap #5 / dbf: 0x14
     * frame yields with no other effect, so they collapse to D7 = 0xFFFF. */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;

    if (Fn_13F5B8(m, r) != 0)                       /* 0x13F582 bsr 0x13F5B8 */
        return 1;                                   /* yielded inside the sequence */

    /* 0x13F586..0x13F596: jingle 0x4B through the sound hook. */
    {
        int16_t cmd = Sound_PreCmdHook(m, 0x004B);  /* 0x13F58A jsr 0x13F238 */
        Sound_PlayCmdForced(m, cmd);                /* 0x13F590 jsr 0x003B66 */
    }

    init_13F436(m, r);                              /* 0x13F59A bra.w 0x13F444 */
    return 1;                                       /* 0x13F462 trap #5 */
}
