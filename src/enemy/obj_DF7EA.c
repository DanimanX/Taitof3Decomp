#include "obj_DF7EA.h"
#include "../objects/obj_common.h"

/* ROM 0x0D2D44: D0 = speed.w, D1 = angle (0..0x168). Reads the 16.16
 * (dx,dy) vector for the angle from tables 0x0D2D64/0x0D2ECC and scales it by
 * a 51-entry angle-dependent factor (jump table 0x0D4F26). Returns -1 for an
 * angle index past 50 (the table's end), leaving the caller's state alone. */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k returns the
     * constant 0..50 (k=0 at 0x0D4FF2 clears both; past 50 there is no entry,
     * the caller must not get there), so (D0*k, D1*k) mod 2^32 is exact. */
    if (d2w > 50u)
        return -1;

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0CEDE4: init the animation/state block of obj from the template at
 * A1 (fields live 0x14 bytes into the template). */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0D0E3A: 16-bit LCG step at A5+0x1D60 (n = n*5+1), D1 preserved;
 * returns the high byte of the new seed. */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0D0678: point obj at a 12-byte-entry animation sequence (A1). */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 2u));
    wr32(m, obj + 0x10u, rd32(m, a1 + 4u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 8u));
}

/* ROM 0x0DF7EA: child "floater" attached to the enemy at obj+0x08 (whose own
 * parent is at +0x08): start at that enemy's screen position offset by
 * (-4, -12), then set a 16.16 velocity whose angle is 0xB4 + (0..15) from the
 * A5+0x1D60 LCG and whose speed is 0x0C, via 0x0D2D44; hand over to 0x0DF844. */
int Obj_DF7EA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DF7EA: lea (-0x2f9e,PC),A1 -> extension word 0x0DF7EE -> 0x0DC850 */
    sub_0CEDE4(m, obj, 0x000DC850u);

    /* 0x0DF7F4: lea (0x8a,PC),A1 -> 0x0DF880 */
    sub_0D0678(m, obj, 0x000DF880u);

    /* 0x0DF7FE: A1 = *(*(obj+8)+8); copy its 0x3A/0x3E to 0x14/0x18 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    a1 = rd32(m, a1 + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0004u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x000Cu));

    /* 0x0DF81C: angle = 0xB4 - ((roll & 0xF) sign-extended), speed = 0x0C */
    {
        uint16_t d0 = sub_0D0E3A(m);
        d0 = (uint16_t)(d0 & 0x000Fu);
        d0 = (uint16_t)(0u - d0);
        d0 = (uint16_t)(d0 + 0x00B4u);

        uint32_t dd0, dd1;
        if (sub_0D2D44(m, d0, 0x000Cu, &dd0, &dd1) < 0)
            return -1;
        wr32(m, obj + 0x3Au, dd0);
        wr32(m, obj + 0x3Eu, dd1);
    }

    /* 0x0DF83C */
    wr32(m, obj + 0x00u, 0x000DF844u);
    return 0;                                       /* 0x0DF842 rts */
}
