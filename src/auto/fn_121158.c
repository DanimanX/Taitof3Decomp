/* ROM 0x121158: boss/ship bubble/spark spawner tick (A0 = object, +0x08 =
 * parent). Frees self (0x0105EC) if parent->+0x59 != 0. Tracks A5+0x30D2/30D6
 * when A5+0x30D6 is within [0x9C, 0xC8] (cmp2.w at 0x121228), resetting +0x06
 * = 0x46 whenever position changes. Counts down +0x06 and probabilistically
 * spawns 0x118E88 particles with packed (x, y, side, D3.b) arg in D3, then
 * re-arms 0x121158. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

int Fn_121158(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x121158 */

    if (rd8(m, a1 + 0x59u) != 0u) {                      /* 0x12115C..0x121160 */
        Obj_Free(m, obj);                                /* 0x12121C (preserves D0,A0,A1) */
        r->a[1] = a1;
        return 0;
    }

    {
        uint16_t y = rd16(m, F3_A5 + 0x30D6u);           /* 0x121164 */
        d0 = setw(d0, y);
        /* 0x121168 cmp2.w (0xbc,PC),D0 -> bounds at 0x121228: [0x009C, 0x00C8] */
        if (y >= 0x009Cu && y <= 0x00C8u) {              /* 0x121170 bcs 0x12119E */
            int reset = 0;
            if (y != rd16(m, obj + 0x18u)) {             /* 0x121174..0x121178 */
                reset = 1;
            } else {
                uint16_t x = rd16(m, F3_A5 + 0x30D2u);   /* 0x12117C */
                d0 = setw(d0, x);
                if (x != rd16(m, obj + 0x14u))           /* 0x121180..0x121184 */
                    reset = 1;
            }
            if (reset) {
                wr16(m, obj + 0x06u, 0x0046u);           /* 0x12118C */
                wr16(m, obj + 0x14u, rd16(m, F3_A5 + 0x30D2u)); /* 0x121192 */
                wr16(m, obj + 0x18u, rd16(m, F3_A5 + 0x30D6u)); /* 0x121198 */
            }
        }
    }

    {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x12119E */
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t <= 0)                             /* 0x1211A2 */
            goto rearm;

        d0 = Rng_NextMem(m);                             /* 0x1211A6 */
        d0 = setw(d0, (uint16_t)(d0 & 0x003Fu));         /* 0x1211AC */
        if ((int16_t)(uint16_t)d0 > (int16_t)t)          /* 0x1211B0..0x1211B4 */
            goto rearm;
    }

    d0 = Rng_NextMem(m);                                 /* 0x1211B8 */
    {
        int16_t d0w = (int16_t)(((uint16_t)d0 & 0x007Fu) - 0x003Fu); /* 0x1211BE..0x1211C2 */
        int16_t d1w = d0w;                               /* 0x1211C6 */
        if (d1w >= 0)                                    /* 0x1211C8 */
            d1w = (int16_t)(-d1w);                       /* 0x1211CC */
        d1w >>= 3;                                       /* 0x1211CE asr.w #3,D1w */

        uint8_t d2b = (d0w >= 0) ? 0xFFu : 0x00u;        /* 0x1211D2 sge D2b */
        if (d0w < 0)                                     /* 0x1211D4 */
            d0w = (int16_t)(d0w + 0x0038);               /* 0x1211D8 */
        else
            d0w = (int16_t)(d0w - 0x0018);               /* 0x1211E0 */

        d0w = (int16_t)(d0w + (int16_t)rd16(m, obj + 0x14u)); /* 0x1211E4 */
        d1w = (int16_t)(d1w + (int16_t)rd16(m, obj + 0x18u) + 0x0016); /* 0x1211EC..0x1211F0 */

        uint32_t d3 = (r->d[3] & 0x000000FFu)
                    | (((uint32_t)(uint16_t)d0w & 0x03FFu) << 22)
                    | (((uint32_t)(uint16_t)d1w & 0x03FFu) << 12)
                    | (((uint32_t)d2b & 0x000Fu) << 8);
        r->d[3] = d3;
        r->d[2] = (r->d[2] & 0xFFFFFF00u) | d2b;
        d1 = setw(d1, (uint16_t)d1w);
        d0 = Obj_Spawn(m, obj, 0x00118E88u, 5u, 2u, d3); /* 0x1211FC..0x121212 */
    }

rearm:
    wr32(m, obj + 0x00u, 0x00121158u);                   /* 0x121216 -> 0x121150 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
