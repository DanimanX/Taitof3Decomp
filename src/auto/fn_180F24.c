/* ROM 0x180F24: boss sub-part polar placement & spawner (A0 = object).
 * Places object relative to parent (+0x08) via 0x17BDA2, frees self (0x0105EC)
 * if +0x50 or +0x54 is outside bounds [0x1849F8] / [0x1849FC], else spawns
 * 0x17C856 (group 0xB, count 1), sets +0x06 = 1 (folding the impossible beq
 * at 0x180F6A), and arms 0x180F74. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x17BDA2 with exact D0/D1/D2/D3/A1/A4 register tracking. */
static void polar_place_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d0w = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Cu);
    uint16_t d1w = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Du);
    uint16_t d2w = rd8(m, obj + 0x0Eu);
    uint16_t d3w = rd8(m, obj + 0x0Fu);

    wr16(m, obj + 0x16u, d0w);
    wr16(m, obj + 0x1Au, d1w);
    wr16(m, obj + 0x20u, 0u);
    wr8(m, obj + 0x20u, (uint8_t)d3w);

    uint32_t a1 = 0x00027E40u;
    int32_t c = (int16_t)rd16(m, a1 + 0x80u + (uint32_t)d3w * 2u);
    int32_t s = (int16_t)rd16(m, a1 + (uint32_t)d3w * 2u);
    int32_t rad = (int16_t)(uint16_t)(d2w << 2);
    uint32_t p0 = (uint32_t)(c * rad);
    uint32_t p1 = (uint32_t)(s * rad);

    wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)(p0 >> 16)));
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)(p1 >> 16)));

    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t px = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t py = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, px);
    wr16(m, obj + 0x54u, py);
    wr16(m, obj + 0x14u, px);
    wr16(m, obj + 0x18u, py);

    r->d[0] = (p0 << 16) | px;
    r->d[1] = (p1 << 16) | py;
    r->d[2] = (uint32_t)(uint16_t)rad;
    r->d[3] = (uint32_t)d3w;
    r->a[1] = a1;
    r->a[4] = a4;
}

int Fn_180F24(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    polar_place_regs(m, r);                              /* 0x180F24 -> 0x17BDA2 */

    int16_t px = (int16_t)rd16(m, obj + 0x50u);          /* 0x180F28 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)px;
    if (px < (int16_t)rd16(m, 0x001849F8u) || px > (int16_t)rd16(m, 0x001849FAu)) { /* 0x180F2C cmp2.w */
        Obj_Free(m, obj);                                /* 0x180F34 -> 0x17C78E */
        return 0;
    }

    int16_t py = (int16_t)rd16(m, obj + 0x54u);          /* 0x180F38 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)py;
    if (py < (int16_t)rd16(m, 0x001849FCu) || py > (int16_t)rd16(m, 0x001849FEu)) { /* 0x180F3C cmp2.w */
        Obj_Free(m, obj);                                /* 0x180F44 -> 0x17C78E */
        return 0;
    }

    r->d[0] = Obj_Spawn(m, obj, 0x0017C856u, 0x0Bu, 1u, 0u); /* 0x180F48..0x180F60 */
    wr16(m, obj + 0x06u, 1u);                            /* 0x180F64 (Z=0, 0x180F6A beq folded) */
    wr32(m, obj + 0x00u, 0x00180F74u);                   /* 0x180F6C */
    return 0;
}
