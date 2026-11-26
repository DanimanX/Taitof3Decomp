#include "obj_124E30.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"
#include "obj_11B06E.h"
#include "obj_11B09A.h"

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

/* ROM 0x0CC33A: aim from the object at (A5+0x1D28) toward this object, offset
 * by (D0w,D1w): D0w = (D0w + mid(A1+0x2C,A1+0x2E) + A1+0x14 - A0+0x14) with
 * D0w negated first, same shape for D1w with +0x30/+0x32/+0x18, then
 * 0x006422. With no target the result is the fixed 0x80. *out_a1 mirrors the
 * A1 the ROM leaves behind; the -1 case returns to the caller's `rts` before
 * its `addi.b`/`lsl.w`/`move.w` sequence can store anything. */
uint16_t Aim_0CC33A(const f3mem_t *m, uint32_t a0, uint16_t d0w,
                    uint16_t d1w, uint32_t *out_a1)
{
    uint32_t d2 = rd32(m, F3_A5 + 0x1D28u);
    if (d2 == 0u) {
        *out_a1 = 0xFFFFFFFFu;                         /* lea (-0x1).w,A1 */
        return 0x0080u;
    }
    uint32_t a1 = d2;
    *out_a1 = a1;
    uint16_t t = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu));
    t = (uint16_t)((int16_t)t >> 1);                   /* asr.w #1 */
    d0w = (uint16_t)(0u - d0w);                        /* neg.w D0 */
    d0w = (uint16_t)(d0w + t);
    d0w = (uint16_t)(d0w + rd16(m, a1 + 0x14u));
    d0w = (uint16_t)(d0w - rd16(m, a0 + 0x14u));

    t = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u));
    t = (uint16_t)((int16_t)t >> 1);
    d1w = (uint16_t)(0u - d1w);                        /* neg.w D1 */
    d1w = (uint16_t)(d1w + t);
    d1w = (uint16_t)(d1w + rd16(m, a1 + 0x18u));
    d1w = (uint16_t)(d1w - rd16(m, a0 + 0x18u));

    return Obj_Atan2(m, d0w, d1w);                     /* 0x0CC37C -> 0x006422 */
}

/* ROM 0x124E30: a parented gun pod. While the parent's +0x59 is clear it
 * animates from +0x0C, republishes itself, copies the parent colour +0x1D,
 * integrates through 0x11B09A, then either fires (bit 7 of A5+0x30EE: three
 * 0x0CD124 shots carrying +0x20's low byte, then +0x20 -= 0x640 and sound
 * 0x2A) or steers +0x20 toward the target (angle + 0x20, stored in the HIGH
 * byte). Bit 5 of the same flag byte fires three 0x11B5E6 bullets while
 * A5+0x30F4 is clear; clearing bit 8 deploys three 0x0CD504 pods. Re-arms
 * 0x124E28. A set parent byte frees the object (0x125008 -> 0x105EC).
 *
 * Capture note: the parent is 0x4128E0 and its +0x59 lands on the ROM long at
 * 0x122F86, so the captured byte reads non-zero and every pair takes the
 * free path. */
int Obj_124E30_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x124E30: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x125008 */
    if (rd8(m, rd32(m, obj + 0x08u) + 0x59u) != 0u) {
        Obj_Free(m, obj);                              /* 0x125008 -> 0x0105EC */
        return 0;
    }

    /* 0x124E3C: movea.l (0xC,A0),A1 ; jsr 0x0CBD52 */
    Obj_Animate(m, obj, rd32(m, obj + 0x0Cu));
    (void)Obj_11B06E_Tick(m, obj);                     /* 0x124E46 */
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du)); /* 0x124E50 */
    (void)Obj_11B09A_Tick(m, obj);                     /* 0x124E56 */

    /* 0x124E5C: bftst (0x30EE,A5){7:1} ; beq 0x124F00 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x01u) != 0u) {
        /* 0x124E66: move.w #0x4 ; jsr 0x0CC2BC ; tst.w D0w ; bne 0x124F18 */
        if (sub_0CC2BC(m, 0x0004u) == 0u) {
            /* 0x124E76..0x124EDE: three 0x0CD124 shots; each argument is one
             * of the 0x5747/0x4E50/0x4557 constants with the pod heading's
             * low byte in its own low word */
            uint32_t ang = (uint32_t)(rd16(m, obj + 0x20u) & 0xFFu);
            (void)Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, 0x57470000u | ang);
            (void)Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, 0x4E500000u | ang);
            (void)Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, 0x45570000u | ang);
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0640u)); /* 0x124EE2 */
            Sound_PlayCmd(m, 0x002A);                  /* 0x124EE8 -> 0x13F238 + 0x39D8 */
        }
    } else {
        /* 0x124F00: move.w #0x4E,D0w ; move.w #0x50,D1w ; jsr 0x0CC33A */
        uint32_t a1;
        uint16_t d0 = Aim_0CC33A(m, obj, 0x004Eu, 0x0050u, &a1);
        if (a1 != 0xFFFFFFFFu) {                       /* no-target path skips to the rts */
            d0 = (uint16_t)(d0 + 0x20u);               /* addi.b #0x20,D0b */
            wr16(m, obj + 0x20u, (uint16_t)(d0 << 8)); /* lsl.w #8: heading HIGH byte */
        }
    }

    /* 0x124F18: tst.b (0x30F4,A5) ; bne 0x124F9A ; bftst {5:1} ; beq */
    if (rd8(m, F3_A5 + 0x30F4u) == 0u &&
        (rd8(m, F3_A5 + 0x30EEu) & 0x04u) != 0u &&
        sub_0CC2BC(m, 0x0024u) == 0u) {
        (void)Obj_Spawn(m, obj, 0x0011B5E6u, 9u, 2u, 0x34200800u);
        (void)Obj_Spawn(m, obj, 0x0011B5E6u, 9u, 2u, 0x49240C00u);
        (void)Obj_Spawn(m, obj, 0x0011B5E6u, 9u, 2u, 0x5C291000u);
    }

    /* 0x124F9A: bfclr (0x30EE,A5){8:1} ; beq 0x125004 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) {
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & 0x7Fu));
        (void)Obj_Spawn(m, obj, 0x000CD504u, 9u, 2u, 0x59490000u);
        (void)Obj_Spawn(m, obj, 0x000CD504u, 9u, 2u, 0x50510000u);
        (void)Obj_Spawn(m, obj, 0x000CD504u, 9u, 2u, 0x47590000u);
    }

    wr32(m, obj + 0x00u, 0x00124E30u);                 /* 0x125004 -> 0x124E28 */
    return 0;
}
