/* ROM 0x19383C: segmented boss tail/chain follower tick (A0 = object).
 * Updates tail whip speed A5+0x4B50 (ramp +0x80 up to 0x1000 when A5+0x4B3A bit
 * 9 from MSB is set, playing sound 0x2C once via A5+0x4B96 bit 11; otherwise
 * decay -0x40 down to 0, or re-aim +0x20 toward A5+0x1D28 via 0x17AF66 clamped
 * to [0x78, 0x98]). Moves head relative to parent (+0x08) + (+0x16, +0x1A),
 * propagates (+0x50, +0x54, +0x20) down the 11-link +0x66 chain with sin/cos
 * offsets from 0x027E40 scaled by (A5+0x4B50 >> 6), and re-arms 0x19383C or
 * frees when A5+0x4B38 == -1. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17AF66: aim angle from obj (+0x50,+0x54) to target (+0x4C) + (0x18,0x10). */
static uint32_t sub_17AF66(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    int32_t target = (int32_t)rd32(m, obj + 0x4Cu);
    if (target <= 0)
        return d0;
    uint32_t a1 = (uint32_t)target;
    int16_t dx = (int16_t)(rd16(m, a1 + 0x14u) + 0x18 - rd16(m, obj + 0x50u));
    int16_t dy = (int16_t)(rd16(m, a1 + 0x18u) + 0x10 - rd16(m, obj + 0x54u));
    uint16_t ang = Obj_AtanByte(m, dx, dy);
    return (d0 & 0xFFFF0000u) | ang;
}

int Fn_19383C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    /* 0x19383C..0x1938CE: tail whip / aim state machine */
    if ((rd16(m, F3_A5 + 0x4B3Au) & 0x0040u) != 0u) {
        if ((rd16(m, F3_A5 + 0x4B96u) & 0x0010u) == 0u) { /* 0x193846 */
            wr16(m, F3_A5 + 0x4B96u, rd16(m, F3_A5 + 0x4B96u) | 0x0010u);
            Sound_PlayHooked(m, 0x002Cu);                /* 0x193856..0x193866 */
        }
        if ((int16_t)rd16(m, F3_A5 + 0x4B50u) >= 0x1000) { /* 0x19386A */
            wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) & (uint16_t)~0x0040u);
            wr16(m, F3_A5 + 0x4B96u, rd16(m, F3_A5 + 0x4B96u) & (uint16_t)~0x0010u);
        } else {
            wr16(m, F3_A5 + 0x4B50u, (uint16_t)(rd16(m, F3_A5 + 0x4B50u) + 0x0080u));
        }
    } else if ((int16_t)rd16(m, F3_A5 + 0x4B50u) > 0) { /* 0x19388E */
        wr16(m, F3_A5 + 0x4B50u, (uint16_t)(rd16(m, F3_A5 + 0x4B50u) - 0x0040u));
    } else {
        uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);         /* 0x1938A0 */
        wr32(m, obj + 0x4Cu, tgt);
        if (tgt == 0u)
            wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) & (uint16_t)~0x0040u);
        d0 = sub_17AF66(m, obj, d0);                     /* 0x1938B0 */
        int16_t ang = (int16_t)(uint16_t)d0;
        if (ang < 0x78)                                  /* 0x1938B6 */
            ang = 0x78;
        if (ang > 0x98)                                  /* 0x1938C2 */
            ang = 0x98;
        wr8(m, obj + 0x20u, (uint8_t)ang);               /* 0x1938CE */
    }

    /* 0x1938D2..0x1938F2: head position from parent (+0x08) + offset */
    uint32_t parent = rd32(m, obj + 0x08u);
    uint16_t hx = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t hy = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, hx);
    wr16(m, obj + 0x54u, hy);
    wr16(m, obj + 0x14u, hx);
    wr16(m, obj + 0x18u, hy);

    /* 0x1938F6..0x193958: propagate down 11-link +0x66 chain */
    uint32_t a3 = obj;
    uint32_t a4 = rd32(m, a3 + 0x66u);
    wr32(m, a4 + 0x50u, rd32(m, a3 + 0x50u));
    wr32(m, a4 + 0x54u, rd32(m, a3 + 0x54u));
    wr16(m, a4 + 0x20u, rd16(m, a3 + 0x20u));
    a3 = a4;

    int32_t scale = (int32_t)(int16_t)rd16(m, F3_A5 + 0x4B50u) >> 6; /* 0x193944 bfexts {0:10} */
    for (int i = 0; i < 10; i++) {
        a4 = rd32(m, a3 + 0x66u);
        wr32(m, a4 + 0x50u, rd32(m, a3 + 0x50u));
        wr32(m, a4 + 0x54u, rd32(m, a3 + 0x54u));
        wr16(m, a4 + 0x20u, rd16(m, a3 + 0x20u));
        uint32_t idx = rd8(m, a4 + 0x20u);               /* 0x193932 bfextu {0:8} */
        int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + idx * 2u);
        int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + idx * 2u);
        d0 = (uint32_t)((int32_t)cos_v * scale);         /* 0x19394A muls.w */
        d1 = (uint32_t)((int32_t)sin_v * scale);         /* 0x19394C muls.w */
        wr32(m, a4 + 0x50u, rd32(m, a4 + 0x50u) + d0);
        wr32(m, a4 + 0x54u, rd32(m, a4 + 0x54u) + d1);
        a3 = a4;
    }

    /* 0x19395C..0x193972: re-arm 0x19383C or free */
    if (rd16(m, F3_A5 + 0x4B38u) != 0xFFFFu) {
        wr32(m, obj, 0x0019383Cu);                       /* 0x193834 */
    } else {
        Obj_Free(m, obj);                                /* 0x193968 */
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = 0x00027E40u;
    return 0;
}
