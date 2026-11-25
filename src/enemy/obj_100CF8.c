#include "obj_100CF8.h"
#include "../objects/obj_shared_ext.h"
#include "obj_E8B72.h"

/* ROM 0x100CF8: one-shot init tick for the object spawned from the 0x100EBA
 * parameter record. Seeds the object from that record, picks a palette, rolls
 * the sprite list (8 entries), installs the behaviour record, sets the +0x04
 * flags, turns the +0x58 heading into a +0x50/+0x54 velocity, rolls one of
 * four stat record pointers into +0x1E..+0x32 and its long into +0x0C, then rolls
 * +0x18/+0x3E back by the +0x0E amount and arms 0x100D86. */

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables scaled by D1w (0..50). */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    int32_t idx = (int32_t)(int16_t)d0w * 4;
    uint32_t d0 = rd32(m, (uint32_t)(0x000D2D64 + idx));
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2ECC + idx));
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
    return 0;
}

int Obj_100CF8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x100CF8: lea (0x1C0,PC),A1 ; jsr 0x0CEC4C (A1 = 0x100EBA) */
    Obj_InitMover(m, obj, 0x00100EBAu);

    /* 0x100D02: jsr 0x0CEE2C */
    Obj_ClassColour(m, obj);

    /* 0x100D08: movea.l (0x1DE,PC,D0w*4),A1 with (PC) = 0x100D14 ;
     * table 0x100EF2, 8 entries, D0w = Rng_Mix1D60() & 7 */
    uint16_t idx = (uint16_t)(Rng_Mix1D60(m) & 0x0007u);
    uint32_t a1 = rd32(m, 0x00100EF2u + (uint32_t)idx * 4u);

    /* 0x100D1A: jsr 0x0D0678 */
    Obj_SetAnim(m, obj, a1);

    /* 0x100D20: bset.b #4,(0x7,A0) */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    /* 0x100D26: bclr.b #1,(0x34,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
    /* 0x100D2C: bset.b #2,(0x34,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));

    /* 0x100D32..0x100D48: jsr 0x0D2D44 with D0w = +0x58, D1w = +0x60 */
    {
        uint32_t vx, vy;
        if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u),
                       &vx, &vy) < 0)
            return -1;
        wr32(m, obj + 0x50u, vx);                    /* 0x100D40 */
        wr32(m, obj + 0x54u, vy);                    /* 0x100D44 */
    }

    /* 0x100D48: movea.l (0x11E,PC,D0w*4),A1 with (PC) = 0x100D54 ;
     * 0x100E72 holds 4 record POINTERS, D0w = Rng_Mix1D60() & 3 */
    {
        uint16_t sel = (uint16_t)(Rng_Mix1D60(m) & 0x0003u);
        uint32_t rec = rd32(m, 0x00100E72u + (uint32_t)sel * 4u);
        wr16(m, obj + 0x1Eu, rd16(m, rec + 0x00u));  /* 0x100D5A */
        wr16(m, obj + 0x2Cu, rd16(m, rec + 0x02u));  /* 0x100D5E */
        wr16(m, obj + 0x2Eu, rd16(m, rec + 0x04u));  /* 0x100D62 */
        wr16(m, obj + 0x30u, rd16(m, rec + 0x06u));  /* 0x100D66 */
        wr16(m, obj + 0x32u, rd16(m, rec + 0x08u));  /* 0x100D6A */
        wr32(m, obj + 0x0Cu, rd32(m, rec + 0x0Au));  /* 0x100D6E */
    }

    /* 0x100D72..0x100D7A: D0w = +0x0E ; sub.w D0w to +0x3E and +0x18 */
    {
        uint16_t d0w = rd16(m, obj + 0x0Eu);
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
    }

    /* 0x100D7E: move.l #0x100D86,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x00100D86u);
    return 0;
}
