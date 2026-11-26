/* ROM 0x198FBE: two-phase boss explosion/debris spawner tick (A0 = object).
 * Phase 1 (entry 0x198FBE): counts down +0x06, re-arming 0x198FBE while > 0;
 * when <= 0, decrements +0x22 and while > 0 spawns 0x183268 (group 9, 2 groups)
 * with random arg (randU(0x20) + 0x88) and resets +0x06 = 5 + randU(5).
 * Phase 2 (0x198FCA, when +0x22 <= 0): resets +0x22 = 0x46, +0x16 = 0x20,
 * +0x1A = 0, then loops at 0x198FDC copying parent (+0x08)->(+0x50/+0x54)
 * plus (+0x16/+0x1A), spawning 0x183308 with packed signed random offsets
 * (rand(0x40), rand(0x20)) in high word, setting +0x06 = randU(5), re-arming
 * 0x199042 while +0x06 != 0, or decrementing +0x22 and freeing (0x0105EC)
 * once +0x22 <= 0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_198FBE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t c6 = (int16_t)(rd16(m, obj + 0x06u) - 1u);       /* 0x198FBE */
    wr16(m, obj + 0x06u, (uint16_t)c6);
    if (c6 > 0) {                                             /* 0x198FC2 */
        wr32(m, obj, 0x00198FBEu);                            /* 0x198FB6 */
        return 0;
    }

    for (;;) {
        int16_t c22 = (int16_t)(rd16(m, obj + 0x22u) - 1u);   /* 0x198FC4 */
        wr16(m, obj + 0x22u, (uint16_t)c22);
        if (c22 <= 0)                                         /* 0x198FC8 */
            break;

        /* 0x198F80 */
        uint32_t arg = (uint32_t)Obj_RandRangeU(m, 0x0020u);  /* 0x198F82 */
        arg = (arg & 0xFFFFFF00u) | (uint8_t)(arg + 0x88u);   /* 0x198F88 */
        (void)Obj_Spawn(m, obj, 0x00183268u, 9u, 2u, arg);    /* 0x198F9C */
        uint16_t wait = (uint16_t)(Obj_RandRangeU(m, 5u) + 5u); /* 0x198FA6..0x198FAE */
        r->d[0] = (uint32_t)wait;
        wr16(m, obj + 0x06u, wait);                           /* 0x198FB0 */
        if (wait == 0u)                                       /* 0x198FB4 */
            continue;
        wr32(m, obj, 0x00198FBEu);                            /* 0x198FB6 */
        return 0;
    }

    /* 0x198FCA */
    wr16(m, obj + 0x22u, 0x0046u);
    wr16(m, obj + 0x16u, 0x0020u);
    wr16(m, obj + 0x1Au, 0x0000u);

    for (;;) {
        /* 0x198FDC */
        uint32_t a4 = rd32(m, obj + 0x08u);
        r->a[4] = a4;
        uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, x);
        wr16(m, obj + 0x54u, y);

        uint32_t d1 = 0u;                                     /* 0x198FF8 */
        uint16_t rx = Obj_RandRange(m, 0x0040u);              /* 0x198FFA..0x198FFC */
        d1 = (d1 & 0xFFFFFF00u) | (rx & 0xFFu);               /* 0x199002 */
        d1 <<= 8;                                             /* 0x199004 */
        uint16_t ry = Obj_RandRange(m, 0x0020u);              /* 0x199006..0x199008 */
        d1 = (d1 & 0xFFFFFF00u) | (ry & 0xFFu);               /* 0x19900E */
        d1 = (d1 << 16) | (d1 >> 16);                         /* 0x199010 */
        r->d[1] = d1;

        (void)Obj_Spawn(m, obj, 0x00183308u, 9u, 2u, d1);     /* 0x199022 */
        uint16_t wait2 = Obj_RandRangeU(m, 5u);               /* 0x19902C..0x19902E */
        r->d[0] = setw(0x00000005u, wait2);
        wr16(m, obj + 0x06u, wait2);                          /* 0x199034 */
        if (wait2 != 0u) {                                    /* 0x199038 */
            wr32(m, obj, 0x00199042u);                        /* 0x19903A */
            return 0;
        }

        /* 0x199048 */
        int16_t rem = (int16_t)(rd16(m, obj + 0x22u) - 1u);
        wr16(m, obj + 0x22u, (uint16_t)rem);
        if (rem <= 0)                                         /* 0x19904C */
            break;
    }

    Obj_Free(m, obj);                                         /* 0x19904E..0x199056 */
    return 0;
}
