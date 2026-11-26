/* ROM 0x199042: boss explosion/spawner tick (A0 = object, +0x08 = parent).
 * Counts down +0x06 and re-arms 0x199042 while > 0. On <= 0, decrements +0x22:
 * frees self via 0x0105EC when <= 0; otherwise tracks parent +0x50/+0x54 +
 * (+0x16/+0x1A), spawns 0x183308 with random (x, y) offsets in +-[-63..63,
 * -31..31] from 0x1794A4, picks next delay +0x06 in [0..4] from 0x1794F0
 * (looping immediately if 0), and re-arms 0x199042. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Fn_199042(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x199042 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                              /* 0x199046 */
        wr32(m, obj, 0x00199042u);                       /* 0x19903A */
        return 0;
    }

    for (;;) {
        uint16_t w22 = (uint16_t)(rd16(m, obj + 0x22u) - 1u); /* 0x199048 */
        wr16(m, obj + 0x22u, w22);
        if ((int16_t)w22 <= 0) {                         /* 0x19904C */
            Obj_Free(m, obj);                            /* 0x19904E..0x199056 */
            return 0;                                    /* 0x19905A */
        }

        uint32_t a4 = rd32(m, obj + 0x08u);              /* 0x198FDC */
        r->a[4] = a4;
        wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u))); /* 0x198FE0..0x198FF0 */
        wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au))); /* 0x198FE4..0x198FF4 */

        uint32_t d1 = (uint8_t)Obj_RandRange(m, 0x0040u); /* 0x198FF8..0x199002 */
        d1 <<= 8;                                        /* 0x199004 */
        d1 = (d1 & 0xFFFFFF00u) | (uint8_t)Obj_RandRange(m, 0x0020u); /* 0x199006..0x19900E */
        d1 = (d1 << 16) | (d1 >> 16);                    /* 0x199010 swap D1 */

        (void)Obj_Spawn(m, obj, 0x00183308u, 9u, 2u, d1); /* 0x199012..0x199028 */

        uint16_t wait = Obj_RandRangeU(m, 5u);           /* 0x19902C..0x19902E */
        wr16(m, obj + 0x06u, wait);                      /* 0x199034 */
        r->d[0] = (uint32_t)wait;
        r->d[1] = d1;
        if (wait != 0u) {                                /* 0x199038 */
            wr32(m, obj, 0x00199042u);                   /* 0x19903A */
            return 0;
        }
    }
}
