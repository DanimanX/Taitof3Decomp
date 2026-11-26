/* ROM 0x137794: stage boss approach/vertical-follow tick (A0 = object).
 * While A5+0x32D6 < 2, steers +0x54 toward player Y (+0x18 + 0x10 vs A5+0x30D6),
 * eases +0x56 -> +0x42 (0x11B2CA), advances shared scroll/pos along +0x20>>8
 * (0x11B262), and counts down +0x06 (re-arming 0x137794 while > 0, else clearing
 * +0x54, setting +0x06 = 0x0A, and advancing to 0x1377E8). When A5+0x32D6 >= 2,
 * clears A5+0x3318, sets +0x54 = 4, +0x50 = 0x2E0, +0x52 = 0x80, +0x06 = 0x64,
 * and advances to 0x137868. */
#include "fn_auto.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B2CA: ease +0x56.w toward +0x54.b and store (+0x56 >> 8) in +0x42.w. */
static void sub_11B2CA(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    int8_t target = (int8_t)rd8(m, obj + 0x54u);            /* 0x11B2CA */
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);               /* 0x11B2CE */
    if (target != cur) {                                    /* 0x11B2D2 */
        if (target > cur)                                   /* 0x11B2D6 */
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u)); /* 0x11B2E4 */
        else
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u)); /* 0x11B2DA */
    }
    int16_t spd = (int16_t)rd16(m, obj + 0x56u) >> 8;       /* 0x11B2EA..0x11B2EE */
    *d0_io = setw(*d0_io, (uint16_t)spd);
    wr16(m, obj + 0x42u, (uint16_t)spd);                    /* 0x11B2F0 */
}

/* ROM 0x11B262: sync +0x14/+0x18 with A5+0x30D2/0x30D6, move along +0x20>>8, write back. */
static void sub_11B262(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *d2_io)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));         /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));         /* 0x11B268 */
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);   /* 0x11B26E..0x11B272 */
    *d0_io = setw(*d0_io, ang);

    int16_t spd = (int16_t)rd16(m, obj + 0x42u);            /* 0x0CBE18 */
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + 2u * ang);
    *d1_io = (uint32_t)((int32_t)spd * (int32_t)c);
    *d2_io = (uint32_t)((int32_t)spd * (int32_t)s);
    Obj_MoveByAngle(m, obj, ang);                           /* 0x11B274 */

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));         /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));         /* 0x11B280 */
}

int Fn_137794(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t a1 = r->a[1];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 2) {           /* 0x137794..0x13779A */
        wr16(m, F3_A5 + 0x3318u, 0u);                       /* 0x137842 */
        wr8(m, obj + 0x54u, 4u);                            /* 0x137846 */
        wr16(m, obj + 0x50u, 0x02E0u);                      /* 0x13784C */
        wr16(m, obj + 0x52u, 0x0080u);                      /* 0x137852 */
        wr16(m, obj + 0x06u, 0x0064u);                      /* 0x137858 (beq 137880 never taken) */
        wr32(m, obj, 0x00137868u);                          /* 0x137860 */
        return 0;
    }

    a1 = rd32(m, F3_A5 + 0x30E2u);                          /* 0x13779E */
    uint16_t py = (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u); /* 0x1377A2..0x1377A6 */
    d0 = setw(d0, py);
    if ((int16_t)py >= (int16_t)rd16(m, F3_A5 + 0x30D6u))   /* 0x1377AA..0x1377AE */
        wr8(m, obj + 0x54u, 4u);                            /* 0x1377BC */
    else
        wr8(m, obj + 0x54u, 0xFCu);                         /* 0x1377B2 */

    sub_11B2CA(m, obj, &d0);                                /* 0x1377C2 */
    sub_11B262(m, obj, &d0, &d1, &d2);                      /* 0x1377C8 */

    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x1377CE */
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {                               /* 0x1377D2 */
        wr32(m, obj, 0x00137794u);                          /* 0x13778C */
    } else {
        wr8(m, obj + 0x54u, 0u);                            /* 0x1377D4 */
        wr16(m, obj + 0x06u, 0x000Au);                      /* 0x1377DA */
        wr32(m, obj, 0x001377E8u);                          /* 0x1377E0 */
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->a[1] = a1;
    r->a[3] = 0x00027E40u;
    return 0;
}
