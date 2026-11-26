/* ROM 0x1800C8: delayed boss turret/pod initializer tick (A0 = object).
 * Counts down +0x06 and re-arms 0x1800C8 while > 0. Once <= 0, sets +0x0E = 8,
 * places via 0x17BDA2, plays hooked sound 0x1A, seeds stats from 0x17E444
 * (0x184888), initializes anim index from 0x17E45A (0x179A5C), installs 0x18017E
 * script10, spawns 0x17C4D4 child into +0x62 (freeing self via 0x0105EC if <= 0),
 * computes centered hitbox offsets (+0x16/+0x1A), clears +0x58/+0x5A, sets
 * +0x34=2, +0x0C=0x17D4C0, and transitions to 0x17C240. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1800C8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        uint16_t c = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, c);
        if ((int16_t)c > 0) {                           /* 0x1800CC */
            wr32(m, obj + 0x00u, 0x001800C8u);          /* 0x1800C0 */
            return 0;
        }
    }

    wr8(m, obj + 0x0Eu, 8u);                            /* 0x1800CE..0x1800D0 */
    Obj_PolarPlace(m, obj);                             /* 0x1800D6 (0x17BDA2) */
    Sound_PlayHooked(m, 0x001Au);                       /* 0x1800DA..0x1800EA */
    Obj_184888(m, obj, 0x0017E444u);                    /* 0x1800EE */

    {
        uint16_t idx = rd16(m, 0x0017E45Au);            /* 0x1800FA */
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, 0x0017E45Cu);
        if (Obj_AnimInit(m, obj, idx) < 0)              /* 0x18010A */
            return -1;
    }

    wr32(m, obj + 0x44u, 0x0018017Eu);                  /* 0x180116 */
    wr16(m, obj + 0x48u, rd16(m, 0x00180182u));         /* 0x180120 */
    wr16(m, obj + 0x4Au, 0u);                           /* 0x180126 */

    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 3u, 2u, 7u); /* 0x18012A */
        r->d[0] = child;
        wr32(m, obj + 0x62u, child);                    /* 0x180148 */
        if ((int32_t)child <= 0) {                      /* 0x18014C */
            /* 0x17BDA2 left D1 = sin*(r<<2) swapped; 0x184888 didn't touch D1;
             * 0x17960A->0x1798BA didn't touch D1; 0x1798BA left A1 = +0x3A + (d0w*4). */
            uint32_t d3 = rd8(m, obj + 0x0Fu);
            int32_t s = (int32_t)(int16_t)rd16(m, 0x00027E40u + d3 * 2u);
            int32_t rad = (int32_t)(int16_t)(uint16_t)(rd8(m, obj + 0x0Eu) << 2);
            uint32_t p1 = (uint32_t)(s * rad);
            r->d[1] = (p1 << 16) | (p1 >> 16);
            uint16_t ang_idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
            r->a[1] = rd32(m, obj + 0x3Au) + (uint32_t)ang_idx * 4u;
            Obj_Free(m, obj);                           /* 0x17C78E */
            return 0;
        }
    }

    {
        uint32_t spr = rd32(m, obj + 0x10u);            /* 0x180150 */
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);
        uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3);
        r->a[1] = spr + 4u;
        r->d[0] = setw(r->d[0], d0w);
        r->d[1] = setw(r->d[1], d1w);
        wr16(m, obj + 0x16u, d0w);
        wr16(m, obj + 0x1Au, d1w);
    }

    wr16(m, obj + 0x58u, 0u);                           /* 0x180164 */
    wr16(m, obj + 0x5Au, 0u);                           /* 0x180168 */
    wr8(m, obj + 0x34u, 2u);                            /* 0x18016C */
    wr32(m, obj + 0x0Cu, 0x0017D4C0u);                  /* 0x180172 */
    wr32(m, obj + 0x00u, 0x0017C240u);                  /* 0x18017A -> 0x17C238 */
    return 0;
}
