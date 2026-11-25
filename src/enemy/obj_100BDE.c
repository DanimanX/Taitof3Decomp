#include "obj_100BDE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "obj_E8B72.h"   /* Rng_Mix1D60, ROM 0x0D0E3A */

/* ROM 0x100BDE: retry-cadence emitter. Every 10 frames (+0x23) it decrements
 * the +0x21 try counter and, while tries remain (or, on the last try, while
 * +0x06 is clear) and the difficulty roll at 0x100C1A passes, spawns one
 * 0x100CF8 child into group 5, count 2; otherwise it spawns the one-shot
 * 0x101332, and places/aims the new child (0x100C7A). Dying (0x0CEE74) happens first when
 * A5-0x144B bit 5 is set or the try counter has run out. */
int Obj_100BDE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x100BDE: btst.b #5,(-0x144B,A5) ; bne 0x0CEE74
     * (ext word EB B5 = -0x144B, so the EA is 0x408000-0x144B = 0x406BB5) */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x100BEA: tst.b (0x21,A0) ; beq 0x0CEE74 */
    if (rd8(m, obj + 0x21u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x100BF4: subq.b #1,(0x23,A0) ; bne 0x100CF6 */
    uint8_t cadence = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, cadence);
    if (cadence != 0u)
        return 0;

    /* 0x100BFC: subq.b #1,(0x21,A0) ; move.b #0xA,(0x23,A0) */
    uint8_t tries = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
    wr8(m, obj + 0x21u, tries);
    wr8(m, obj + 0x23u, 0x0Au);

    /* 0x100C06: tst.b (0x21,A0) ; bne 0x100C1A */
    int to_cf8;
    if (tries != 0u) {
        /* 0x100C1A: jsr 0x0D0E3A ; clr.w D1w ; move.b (0x87F,A5),D1b ;
         * lsr.w #2,D1w ; add.w D1w,D0w ; cmpi.w #0xAA,D0w ; bhi 0x100C58
         * Unsigned bound: only 0..0xAA falls through to 0x100C32. */
        uint16_t d0w = (uint16_t)(Rng_Mix1D60(m) +
                                  (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 2));
        to_cf8 = (d0w <= 0x00AAu);
    } else {
        /* 0x100C0E: tst.b (0x6,A0) ; bne 0x100C58 ; bra 0x100C32 */
        to_cf8 = (rd8(m, obj + 0x06u) == 0u);
    }

    /* 0x100C32 / 0x100C58: pea (0).w ; move.w #2 ; move.w #5 ; move.l #fn ;
     * jsr 0x1057C ; tst.l D0 ; bmi 0x100CF6 (rts) */
    uint32_t child = Obj_Spawn(m, obj, to_cf8 ? 0x00100CF8u : 0x00101332u, 5u, 2u, 0u);
    if ((int32_t)child < 0)
        return 0;

    /* 0x100C7A: place the child at x = 0x140 with a random height, aim it
     * back across the screen and give it a random speed */
    wr16(m, child + 0x14u, 0x0140u);
    uint16_t d7 = (uint16_t)(Rng_Mix1D60(m) & 0x7Fu);
    wr16(m, child + 0x18u, (uint16_t)((Rng_Mix1D60(m) & 0x3Fu) + d7 + 0x1Du));
    d7 = (uint16_t)(Rng_Mix1D60(m) & 0x7Fu);
    uint16_t d0 = (uint16_t)((Rng_Mix1D60(m) & 0x3Fu) + d7 + 0x1Du);
    /* 0x100CBE: D1 = child y - roll, D0 = 0x140 ; jsr 0x0D0E52 */
    uint16_t d1 = (uint16_t)(rd16(m, child + 0x18u) - d0);
    wr16(m, child + 0x58u, Obj_Atan256(m, (int16_t)d1, (int16_t)0x0140));
    wr16(m, child + 0x60u, (uint16_t)((Rng_Mix1D60(m) & 0x07u) + 0x17u));
    wr8(m, child + 0x4Eu, 0u);
    /* 0x100CE8: on the last try the child inherits +0x4E */
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, child + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
