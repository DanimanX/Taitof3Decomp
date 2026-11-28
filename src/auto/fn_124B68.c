/* ROM 0x124B68: UNVERIFIED (no capture): stage-boss camera advance tick with
 * a randomised ring of child objects (A0 = object).
 * While the camera x counter A5+0x30D2 is below 0x1AE it is advanced by the
 * phase word +0x06 >> 3; the object then takes the camera position
 * (+0x14 = A5+0x30D2 word, +0x18 = A5+0x30D6 word), draws two random bytes
 * (-0x3F..0, -0x20..+0x1F) into the two high bytes of D1 through BFINS and
 * spawns two 0x13E4EE children (group 5, arg = D1); every other call it also
 * plays hooked sound 0x35. It counts +0x06 down and re-arms 0x124B68 through
 * the 0x124B60 thunk while the count stays positive; at 0 it tail-jumps to the
 * boss-script entry stored in +0x0C (`jmp (A1)`), which the boss script
 * (Fn_124800 0x124868) always sets to 0x124876 -- that step stores +0x06 =
 * 0x5A and arms 0x124886. Any other +0x0C value would jump into another script
 * step and is reported as untranslated. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"

int Fn_124B68(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d1;

    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x01AE) {     /* 0x124B68..0x124B6E bge 0x124B7C */
        uint16_t step = (uint16_t)(rd16(m, obj + 0x06u) >> 3); /* 0x124B72 move.w (0x6,A0) ; lsr.w #3 */
        wr16(m, F3_A5 + 0x30D2u,                           /* 0x124B78 add.w D0w,(0x30D2,A5) */
             (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + step));
    }
    wr16(m, obj + 0x14u, rd16(m, F3_A5 + 0x30D2u));        /* 0x124B7C move.w */
    wr16(m, obj + 0x18u, rd16(m, F3_A5 + 0x30D6u));        /* 0x124B82 move.w */

    d1 = 0x00000003u;                                      /* 0x124B88 moveq #3,D1 */
    {
        uint32_t rnd;
        /* 0x124B8A jsr 0x1D8A (Rng_NextMem) ; andi.w #0x3F ; subi.w #0x3F ;
         * 0x124B98 bfins D0,D1{0:8}: bits 31..24 of D1. */
        rnd = Rng_NextMem(m);
        rnd = (rnd & 0xFFFF0000u) | (uint16_t)((uint16_t)rnd & 0x003Fu);
        rnd = (rnd & 0xFFFF0000u) | (uint16_t)((uint16_t)rnd - 0x003Fu);
        r->d[0] = rnd;
        d1 = (d1 & 0x00FFFFFFu) | ((rnd & 0xFFu) << 24);
        /* 0x124B9C jsr 0x1D8A ; andi.w #0x3F ; subi.w #0x20 ;
         * 0x124BAA bfins D0,D1{8:8}: bits 23..16 of D1. */
        rnd = Rng_NextMem(m);
        rnd = (rnd & 0xFFFF0000u) | (uint16_t)((uint16_t)rnd & 0x003Fu);
        rnd = (rnd & 0xFFFF0000u) | (uint16_t)((uint16_t)rnd - 0x0020u);
        r->d[0] = rnd;
        d1 = (d1 & 0xFF00FFFFu) | ((rnd & 0xFFu) << 16);
    }
    r->d[1] = d1;
    /* 0x124BAE..0x124BBE: move.l D1,-(A7) ; move.w #2,-(A7) ; move.w #5,-(A7) ;
     * move.l #0x13E4EE,-(A7) ; jsr 0x01057C ; lea 0xC(A7),A7.
     * 0x01057C saves D1-D4/A0-A2 and returns the object in D0. */
    r->d[0] = Obj_Spawn(m, r->a[0], 0x0013E4EEu, 5u, 2u, d1);

    /* 0x124BC8 jsr 0x1D8A ; andi.w #1 ; bne 0x124BEA: every other tick it also
     * plays hooked sound 0x35 (0x124BD6..0x124BE6). */
    {
        uint32_t rnd = Rng_NextMem(m);
        rnd = (rnd & 0xFFFF0000u) | (uint16_t)((uint16_t)rnd & 0x0001u);
        r->d[0] = rnd;
        if ((uint16_t)rnd == 0u)
            Sound_PlayHooked(m, 0x0035u);
    }

    {
        int16_t rem = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x124BEA subq.w #1 */
        wr16(m, obj + 0x06u, (uint16_t)rem);
        if (rem > 0) {                                     /* 0x124BEE bgt 0x124B60 */
            wr32(m, obj, 0x00124B68u);                     /* 0x124B60 re-arm thunk */
            return 0;
        }
    }

    {   /* 0x124BF2 movea.l (0xC,A0),A1 ; 0x124BF6 jmp (A1) */
        uint32_t a1 = rd32(m, obj + 0x0Cu);

        r->a[1] = a1;
        /* The only writer of this object's +0x0C is 0x124868 in the boss
         * script (Fn_124800), which stores 0x124876, so the dynamic jmp lands
         * on that step; it is translated in full here:
         *   0x124876 move.w #0x5A,(0x6,A0)
         *   0x12487C beq.b 0x12488C   (never taken: 0x5A is not zero)
         *   0x12487E move.l #0x124886,(A0)
         *   0x124884 rts */
        wr16(m, obj + 0x06u, 0x005Au);
        wr32(m, obj, 0x00124886u);
    }
    return 0;
}
