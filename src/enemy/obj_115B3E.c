#include "obj_115B3E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"

/* ROM 0x0CBE34: blink the palette index at +0x1D. With +0x35 set it toggles
 * between 0x40 (and 0x42, which clears +0x35); otherwise it copies +0x4F. */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));      /* 0x0CBE5E */
        return;
    }
    if (rd8(m, obj + 0x1Du) == 0x40u) {                /* 0x0CBE3C */
        wr8(m, obj + 0x35u, 0u);                       /* sf (0x35,A0) */
        wr8(m, obj + 0x1Du, 0x42u);
        return;
    }
    wr8(m, obj + 0x1Du, 0x40u);                        /* 0x0CBE54 */
}

/* ROM 0x0CC2BC: probability gate. D0w = threshold; the scaled threshold is
 * (0x180 - A5+0x87E) * D0w, shifted and byte-swapped; divu gives the counter
 * quotient in the low word and the remainder in the high word (`swap D0`).
 * Callers test the LOW word of D0, so the quotient is what they see. */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint32_t d1 = (uint32_t)(uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    d1 = ((uint32_t)(uint16_t)d0w * d1) & 0xFFFFFFFFu;  /* mulu.w */
    d1 = (d1 << 8) & 0xFFFFFFFFu;                       /* lsl.l #8 */
    d1 = ((d1 << 16) | (d1 >> 16));                     /* swap */
    uint16_t thr = (uint16_t)d1;
    if (thr == 0u)
        thr = 1u;                                       /* 0x0CC2D0 */
    uint32_t num = rd16(m, F3_A5 + 0x0878u);            /* divu.w D1w,D0 */
    uint32_t quot = num / thr;
    uint32_t rem = num % thr;
    uint32_t d0 = (rem << 16) | (quot & 0xFFFFu);
    d0 = ((d0 << 16) | (d0 >> 16));                     /* swap D0 */
    return (uint16_t)(d0 & 0xFFFFu);
}

/* ROM 0x113456: while the on-screen flag is set and x is inside
 * [-0x0A,0x11C], jitter the aim by (rand&0xF)-5 for one frame, steer toward
 * the screen anchor (0xA0,0x74), bias the heading by 0x58 and move. */
static void sub_113456(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return;                                        /* 0x11345C */
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -0x0A || x > 0x011C)
        return;                                        /* 0x113474 */

    uint16_t saved = rd16(m, obj + 0x42u);             /* push (0x42,A0) */
    uint16_t r = (uint16_t)(Rng_NextMem(m) & 0x0Fu);
    wr16(m, obj + 0x42u, (uint16_t)(r - 5u));          /* 0x113486 */

    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    d0 = Obj_Atan2(m, d0, d1);                          /* 0x11349C */
    d0 = (uint16_t)(d0 + 0x58u);                        /* addi.b #0x58,D0b */
    Obj_MoveByAngle(m, obj, (uint16_t)(d0 & 0xFFu));
    wr16(m, obj + 0x42u, saved);                        /* move.w (SP)+ */
}

/* ROM 0x115B3E: a patrolling "fan" enemy. While its HP (+0x24) lasts it
 * blinks, counts +0x56 down toward a new anchor (+0x52/+0x54), steers +0x20
 * by +-2 per frame, moves with 0x0CBE12, and gates a shot on 0x0CC2BC. */
int Obj_115B3E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto stop_115cd6;                              /* 0x115B44 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto stop_115cd6;                              /* 0x115B4C */

    if ((rd8(m, obj + 0x50u) & 0x08u) != 0u)
        goto killed_115c24;                            /* 0x115B56 */

    sub_0CBE34(m, obj);                                /* 0x115B5A */

    if ((rd8(m, obj + 0x50u) & 0x01u) != 0u)
        goto tail_115c08;                              /* 0x115B66 */

    /* 0x115B6A: subq.w #1 ; bgt 0x115B84 */
    {
        uint16_t n = (uint16_t)(rd16(m, obj + 0x56u) - 1u);
        wr16(m, obj + 0x56u, n);
        uint16_t d0;
        if ((int16_t)n > 0) {
            d0 = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, obj + 0x14u));
            uint16_t d1 = (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u));
            d0 = Obj_Atan2(m, d0, d1);                 /* 0x115B94 */
        } else {
            /* 0x115B72: cmpi.w #0x1A4,(0x14,A0) ; bge 0x115CF8 */
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x01A4)
                goto die_115cf8;
            d0 = 0x00F0u;                              /* 0x115B7C */
        }

        /* 0x115B9A: sub.w (0x20,A0),D0w ; tst.b D0b
         * The subtraction is word-wide (0x0090 style heading), and only the
         * LOW byte of the difference is tested. Reading the high byte here
         * (rd8 at +0x20 is the heading's integer part) inverts the sign. */
        uint8_t diff = (uint8_t)(uint16_t)(d0 - rd16(m, obj + 0x20u));
        if ((int8_t)diff < 0)
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 2u));  /* 0x115BAC */
        else
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + 2u));  /* 0x115BA4 */

        /* 0x115BB0: move.w (0x20,A0),D0w ; jsr 0x0CBE12 */
        Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) & 0xFFu));

        /* 0x115BBA: cmpi.w #6,(0x42,A0) ; ble ; subq.w #1 */
        if ((int16_t)rd16(m, obj + 0x42u) > 6)
            wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 1u));

        /* 0x115BC8 */
        if ((int16_t)rd16(m, obj + 0x56u) > 0 && rd8(m, obj + 0x22u) != 0u) {
            if (sub_0CC2BC(m, 0x00B4u) == 0u)          /* 0x115BE2: tst.w D0w */
                wr8(m, obj + 0x51u, (uint8_t)(rd8(m, obj + 0x51u) | 0x01u));
            /* 0x115BEE: btst.l #5 / andi.w #3 on A5+0x878 */
            if ((rd16(m, F3_A5 + 0x0878u) & 0x20u) == 0u &&
                (rd16(m, F3_A5 + 0x0878u) & 0x03u) == 0u)
                wr8(m, obj + 0x51u, (uint8_t)(rd8(m, obj + 0x51u) | 0x02u));
        }
    }

tail_115c08:
    sub_113456(m, obj);                                /* 0x115C08 */

    /* 0x115C0E: subq.w #1 ; bgt 0x115B36 */
    {
        uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, n);
        if ((int16_t)n > 0) {
            wr32(m, obj + 0x00u, 0x00115B3Eu);         /* 0x115B36 */
            return 0;
        }
    }
    /* 0x115C16: st (0x22,A0) ; move.w #0x64 ; bra 0x115B14 */
    wr8(m, obj + 0x22u, 0xFFu);
    wr16(m, obj + 0x06u, 0x0064u);
    {
        /* 0x115B14: new anchor from the RNG and the scroll-lock anchor */
        uint16_t r = (uint16_t)(Rng_NextMem(m) & 0x1Fu);
        wr16(m, obj + 0x52u, (uint16_t)(r + 0x00DCu));
        uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
        wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x10u));
        wr32(m, obj + 0x00u, 0x00115B3Eu);             /* 0x115B36 */
    }
    return 0;

killed_115c24:
    /* 0x115C24: spawn one 0x115E5A child (group 3) and switch to 0x115C54 */
    Obj_Spawn(m, rd32(m, obj + 0x36u), 0x00115E5Au, 3u, 2u, 0u);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));          /* 0x115C40 */
    wr16(m, obj + 0x06u, 0x04B0u);
    wr32(m, obj + 0x00u, 0x00115C54u);
    return 0;

stop_115cd6:
    /* 0x115CD6: blink out, set flag bit 4, re-arm 0x78, next 0x115CF2 */
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x10u));
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x00115CF2u);
    return 0;

die_115cf8:
    /* 0x115CF8: flag bit 5, free, next 0x115D14 */
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
    Obj_Free(m, obj);
    wr32(m, obj + 0x00u, 0x00115D14u);
    return 0;
}
