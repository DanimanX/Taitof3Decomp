/* ROM 0x144894: UNVERIFIED (no capture): configuration ("OPTION") task body.
 * Pre-yield part: clear playfield layer 3 (0x0058AE), draw the stage banner
 * (0x13D11E), reload the flash ramp table (0x13CAAE), then clear bits 14-15 of
 * the 0xE8 attribute words at 0x627200 + (A5-0x5D56) and yield at the TRAP #5
 * at 0x1448C0. Task144894_Frame below runs one pass of the menu loop the ROM
 * resumes with at 0x1448C2.
 *
 * The menu pass redraws the seven option rows at 0x61C220 + 0x80*k with the
 * label pointers from the 0x144AF0 table (colour 8 for the highlighted row, 7
 * otherwise), then moves the highlight on the pad bits of 0x402225 (bit 1 =
 * up, bit 0 = down, wrapping -1 -> 6 and 7 -> 0), and either yields at 0x1448C0
 * when the exit bit (bit 4) is set, or clears layer 3 and enters the screen
 * handler of the row that was highlighted when the pass began: the dispatch
 * index is the D1 read at 0x1448C2, NOT the updated one, and 0x144AF4 +
 * index*8 holds the handler. Those seven screens (0x143D28 0x143DE2 0x143E3E
 * 0x143ECE 0x143F3E 0x14408A 0x14470C) are separate, untranslated routines,
 * so the frame returns -1 at the jmp; every side effect before it is done. A
 * pass that takes the exit branch ends at the 0x1448C0 yield and returns 0. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (0x005666) */
#include "../video/sprite.h"

/* 0x144894 up to the TRAP #5 at 0x1448C0. */
static void body_144894(const f3mem_t *m, f3regs_t *r)
{
    Gfx_ClearPf3(m);                         /* 0x144894 jsr 0x0058AE */
    Stage_DrawBanner13D11E(m);               /* 0x14489A jsr 0x13D11E */
    Leaf_13CAAE(m, r);                       /* 0x1448A0 jsr 0x13CAAE */

    /* 0x1448A6 lea 0x627200,A0 ; 0x1448AC adda.w (A5-0x5D56),A0 ;
     * 0x1448B0 move.w #0xE7,D0 ; clear bits 14-15 of 0xE8 words. */
    uint32_t a0 = 0x00627200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint16_t d0 = 0x00E7u;
    uint16_t d1 = 0;
    do {
        d1 = (uint16_t)(rd16(m, a0) & 0x3FFFu);   /* 0x1448B4 move.w (A0),D1 ; andi.w */
        wr16(m, a0, d1);                          /* 0x1448BA move.w D1,(A0)+ */
        a0 += 2u;
        d0--;                                     /* 0x1448BC dbra D0 */
    } while ((int16_t)d0 >= 0);
    r->a[0] = a0;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)d0; /* D0.w = -1 after dbra */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | d1;
}

int Fn_144894(const f3mem_t *m, f3regs_t *r)
{
    body_144894(m, r);
    return 0;                                /* 0x1448C0 trap #5: the task yields */
}

int Task144894_Frame(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1448C2 move.w (0x331C,A5),D1: the row that was highlighted when this
     * pass began; the redraws and the later dispatch both use this value. */
    uint16_t sel = rd16(m, F3_A5 + 0x331Cu);

    /* 0x1448C6..0x144A82: seven rows, colour 8 for the selected one. */
    for (unsigned k = 0; k < 7u; ++k) {
        uint32_t label = rd32(m, 0x00144AF0u + 8u * k); /* move.l (d16,PC),-(SP) */
        uint32_t dst = 0x0061C220u + 0x80u * k;        /* pea.l 0x61C2x0 */
        uint16_t col = (int16_t)sel == (int16_t)k ? 8u : 7u;
        (void)Text_DrawString(m, label, dst, col);     /* 0x1448DE jsr 0x005666 */
    }

    /* 0x144A86 btst.b #1,(A5-0x5DDB) ; 0x144AD2 btst.b #4: pad bits of 0x402225. */
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x02u))
        wr16(m, F3_A5 + 0x331Cu, (uint16_t)(rd16(m, F3_A5 + 0x331Cu) + 1u)); /* 0x144A90 */
    if (!(rd8(m, F3_A5 - 0x5DDBu) & 0x01u))
        wr16(m, F3_A5 + 0x331Cu, (uint16_t)(rd16(m, F3_A5 + 0x331Cu) - 1u)); /* 0x144A9E */

    /* 0x144AA2..0x144AB6: below zero wraps up (0x144AB0 is an addi.w #0). */
    if ((int16_t)rd16(m, F3_A5 + 0x331Cu) < 0) {
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x331Cu) + 6u); /* 0x144AAC addq.w #6 */
        v = (uint16_t)(v + 1u);                                 /* 0x144AB6 addq.w #1 */
        wr16(m, F3_A5 + 0x331Cu, v);
    }
    /* 0x144ABA..0x144ACE: above six wraps down (0x144AC8 is an addi.w #0). */
    if ((int16_t)rd16(m, F3_A5 + 0x331Cu) > 6) {
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x331Cu) - 6u); /* 0x144AC4 subq.w #6 */
        v = (uint16_t)(v - 1u);                                 /* 0x144ACE subq.w #1 */
        wr16(m, F3_A5 + 0x331Cu, v);
    }

    r->d[1] = (r->d[1] & 0xFFFF0000u) | sel;

    /* 0x144AD2 btst.b #4: exit requested - the pass ends at the 0x1448C0 trap. */
    if (rd8(m, F3_A5 - 0x5DDBu) & 0x10u)
        return 0;

    Gfx_ClearPf3(m);                                /* 0x144ADC jsr 0x0058AE */

    /* 0x144AE2 lsl.w #3,D1 ; 0x144AE4 lea (0xA,PC),A0 (0x144AF0) ;
     * 0x144AEA movea.l (0x4,A0,D1.w),A0 ; 0x144AEE jmp (A0). */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)(sel << 3);
    uint32_t handler = rd32(m, 0x00144AF4u + (uint32_t)(int32_t)(int16_t)(uint16_t)(sel << 3));
    (void)handler;                                  /* 0x143D28 .. 0x14470C: untranslated */
    return -1;                                      /* jmp (A0) into the row's screen */
}
