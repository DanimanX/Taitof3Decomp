#include "obj_101332.h"
#include "../objects/obj_shared.h"
#include "obj_E8B72.h"

/* ROM 0x101332: one-shot init tick for the 0x101332 object. Rolls +0x18 back by
 * 0x20, seeds the object from the movement record at 0x1014E0, picks a palette,
 * rolls a sprite list (8 entries), then sets +0x04 bit 4, clears +0x34 bit 1,
 * sets +0x34 bit 2, turns the +0x58 heading into a +0x50/+0x54 velocity with the
 * 0x0D2D44 speed tables (scaled by +0x60) and arms 0x101390. */

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables {0x0D2D64, 0x0D2ECCu}
 * scaled by D1w through the 0x0D4F26 jump table. Copied from
 * src/objects/objscript_CF4B8.c, where it is verified. */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k returns the
     * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
     * mod 2^32 is exact for every k the table has. */

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

int Obj_101332_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x101332: subi.w #0x20,(0x18,A0) */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x20u));

    /* 0x101338: lea (0x1A4,PC),A1 ; jsr 0x0CEC4C (ext word at 0x10133A -> A1 = 0x1014DE) */
    Obj_InitMover(m, obj, 0x001014DEu);

    /* 0x101342: jsr 0x0CEE2C */
    Obj_InitPalette(m, obj);

    /* 0x101348: movea.l (0x1C2,PC,D0w*4),A1 (full-format ext; base = ext word 0x101354) ;
     * table 0x101516 holds record POINTERS, D0w = Rng_Mix1D60() & 7 */
    uint16_t idx = (uint16_t)(Rng_Mix1D60(m) & 0x0007u);
    uint32_t a1 = rd32(m, 0x00101516u + (uint32_t)idx * 4u);

    /* 0x10135A: jsr 0x0D0678 */
    Obj_SetAnim(m, obj, a1);

    /* 0x101360: bset.b #4,(0x7,A0) */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    /* 0x101366: bclr.b #1,(0x34,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
    /* 0x10136C: bset.b #2,(0x34,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));

    /* 0x101372..0x101380: D0w = +0x58, D1w = +0x60, jsr 0x0D2D44 */
    {
        uint32_t vx, vy;
        if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u),
                       &vx, &vy) < 0)
            return -1;
        wr32(m, obj + 0x50u, vx);                    /* 0x101380 */
        wr32(m, obj + 0x54u, vy);                    /* 0x101384 */
    }

    /* 0x101388: move.l #0x101390,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x00101390u);
    return 0;
}
