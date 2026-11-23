#include "obj_10B83E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D0E52: 8-way radial distance (D0.w, D1.w) -> 0..0x167 */
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    /* cmp.w D0w,D1w ; bls -> bfffo D0, else bfffo D1 */
    uint16_t src;
    if ((int16_t)d1 > (int16_t)d0) {       /* bhi */
        /* (d1 >= d0 handled by the bls below) */
    }
    src = 0u;
    if ((int16_t)d1 > (int16_t)d0)
        src = d1;
    else
        src = d0;
    (void)src;

    /* bfffo on the larger of the two (unsigned, 16-bit window) */
    uint16_t hi = (d1 > d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((hi & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }
    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t t = d0;
        d0 = d1;
        d1 = t;
    }

    d1 = (uint16_t)(d1 << 6);
    d1 = (uint16_t)(d1 + d0);
    d0 = (uint16_t)rd8(m, 0x000D0EBCu + d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x000D0EB4u + 2u * d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x0D06E0: aim helper. Returns D4 (best distance to player 1) and
 * leaves D2/D3, D5/D6 as the two players' |dx|/|dy| words. */
static int16_t sub_0D06E0(const f3mem_t *m, uint32_t obj,
                          uint16_t *out_d2, uint16_t *out_d3,
                          uint16_t *out_d5, uint16_t *out_d6)
{
    int16_t d4;

    wr16(m, obj + 0x42u, 0x00B4u);

    /* player 1 at 0x41B190 */
    d4 = -1;
    if ((int16_t)rd16(m, 0x41B190u + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        uint16_t d2 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, 0x41B190u + 0x14u));
        d2 = (uint16_t)(d2 + rd16(m, a2));
        a2 += 2u;
        d2 = (uint16_t)(d2 - 0x17u);
        int16_t d2s = (int16_t)d2;
        if (d2s < 0)
            d2 = (uint16_t)(0u - d2);
        uint16_t d3 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x41B190u + 0x18u));
        d3 = (uint16_t)(d3 + rd16(m, a2));
        d3 = (uint16_t)(d3 - 0x0Fu);
        if ((int16_t)d3 < 0)
            d3 = (uint16_t)(0u - d3);
        d4 = (int16_t)sub_0D0E52(m, d2, d3);
        *out_d2 = d2;
        *out_d3 = d3;
    } else {
        *out_d2 = 0u;
        *out_d3 = 0u;
    }

    /* player 2 at 0x41B21A */
    int16_t d7 = -1;
    if ((int16_t)rd16(m, 0x41B21Au + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        uint16_t d5 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, 0x41B21Au + 0x14u));
        d5 = (uint16_t)(d5 + rd16(m, a2));
        d5 = (uint16_t)(d5 - 0x17u);
        if ((int16_t)d5 < 0)
            d5 = (uint16_t)(0u - d5);
        uint16_t d6 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x41B21Au + 0x18u));
        d6 = (uint16_t)(d6 + rd16(m, a2));
        d6 = (uint16_t)(d6 - 0x0Fu);
        if ((int16_t)d6 < 0)
            d6 = (uint16_t)(0u - d6);
        d7 = (int16_t)sub_0D0E52(m, d5, d6);
        *out_d5 = d5;
        *out_d6 = d6;
    } else {
        *out_d5 = 0u;
        *out_d6 = 0u;
    }

    /* 0x10B8C0 compares the speed written here against 0xB4 */
    if (d4 < 0) {
        if (d7 < 0)
            return -1;                       /* both players gone: 0x10B7B2 rts */
    } else if (d7 < 0) {
        wr16(m, obj + 0x42u, (uint16_t)d4);
        return *out_d2 > *out_d3 ? (int16_t)*out_d2 : (int16_t)*out_d3;
    } else {
        uint16_t x1 = (*out_d2 > *out_d3) ? *out_d2 : *out_d3;
        uint16_t x2 = (*out_d5 > *out_d6) ? *out_d5 : *out_d6;
        if (x2 < x1) {
            wr16(m, obj + 0x42u, (uint16_t)d7);
            return (int16_t)x2;
        }
        wr16(m, obj + 0x42u, (uint16_t)d4);
        return (int16_t)x1;
    }
    return *out_d2 > *out_d3 ? (int16_t)*out_d2 : (int16_t)*out_d3;
}

/* ROM 0x10B83E */
int Obj_10B83E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;                            /* 0x10B8EC */

    {
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c != 0u)
            return 0;
    }

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x28u);

    {
        uint32_t spawned = Obj_Spawn(m, obj, 0x0010B8F8u, 0xBu, 1u, 0u);
        if ((int32_t)spawned < 0) {
            /* 0x10B8EE */
            wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
            return 0;
        }
        {
            uint32_t a4 = spawned;
            wr32(m, a4 + 0x14u, rd32(m, obj + 0x3Au));
            {
                uint16_t r = Obj_Next4Way(m);
                int16_t d0 = (int16_t)((uint16_t)(r & 0x003Fu) - 0x20u);
                wr16(m, a4 + 0x14u, (uint16_t)(rd16(m, a4 + 0x14u) + (uint16_t)d0));
            }
            wr32(m, a4 + 0x18u, 0xFFE00000u);
            wr32(m, a4 + 0x58u, 0x010E0000u);

            /* 0x10B8BA: jsr 0x0D06E0 leaves a speed in obj+0x42 */
            sub_0D06E0(m, obj, &(uint16_t){0}, &(uint16_t){0}, &(uint16_t){0}, &(uint16_t){0});
            if (rd16(m, obj + 0x42u) < 0x00B4u) {
                wr32(m, a4 + 0x18u, 0x00E80000u);
                wr32(m, a4 + 0x58u, 0x005A0000u);
            }
            wr8(m, a4 + 0x4Eu, 0u);
            if (rd8(m, obj + 0x21u) == 0u)
                wr8(m, a4 + 0x4Eu, rd8(m, obj + 0x4Eu));
        }
    }
    return 0;
}
