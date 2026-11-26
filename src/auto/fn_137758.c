/* ROM 0x137758: stage boss camera approach tick (A0 = object).
 * If A5+0x32D6 < 2, slews speed +0x56/+0x42 toward +0x54 via 0x11B2CA, steps
 * camera scroll A5+0x30D2/+0x30D6 along (+0x20)>>8 via 0x11B262 (0x0CBE12),
 * and counts down +0x06 (re-arming 0x137758 while > 0, else setting bit 4
 * of A5+0x30EE, copying A5+0x1D28 -> A5+0x30E2, setting +0x20 = 0x4000,
 * +0x06 = 0x64, and advancing to 0x137794). If A5+0x32D6 >= 2, clears
 * A5+0x3318, sets +0x54 = 4, +0x50 = 0x2E0, +0x52 = 0x80, +0x06 = 0x64, and
 * advances to 0x137868. */
#include "fn_auto.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B2CA: ease +0x56.w toward +0x54.b and store (+0x56 >> 8) into +0x42. */
static void sub_11B2CA(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    int8_t target = (int8_t)rd8(m, obj + 0x54u);             /* 0x11B2CA */
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);                /* 0x11B2CE */
    if (target > cur) {                                      /* 0x11B2D2..0x11B2D6 */
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u)); /* 0x11B2E4 */
    } else if (target < cur) {
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u)); /* 0x11B2DA */
    }
    uint16_t spd = (uint16_t)((int16_t)rd16(m, obj + 0x56u) >> 8); /* 0x11B2EA..0x11B2EE */
    wr16(m, obj + 0x42u, spd);                               /* 0x11B2F0 */
    *d0_io = setw(*d0_io, spd);
}

/* ROM 0x11B262: sync +0x14/+0x18 with A5+0x30D2/0x30D6, move along +0x20>>8, write back. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));          /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));          /* 0x11B268 */
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);    /* 0x11B26E..0x11B272 */
    r->d[0] = setw(r->d[0], ang);

    int16_t spd = (int16_t)rd16(m, obj + 0x42u);             /* 0x0CBE18 */
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
    r->d[1] = (uint32_t)((int32_t)spd * (int32_t)c);
    r->d[2] = (uint32_t)((int32_t)spd * (int32_t)s);
    r->a[3] = 0x00027E40u;
    Obj_MoveByAngle(m, obj, ang);                            /* 0x11B274 -> 0x0CBE12 */

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));          /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));          /* 0x11B280 */
}

int Fn_137758(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 2) {            /* 0x137758..0x13775E */
        wr16(m, F3_A5 + 0x3318u, 0u);                        /* 0x137842 */
        wr8(m, obj + 0x54u, 4u);                             /* 0x137846 */
        wr16(m, obj + 0x50u, 0x02E0u);                       /* 0x13784C */
        wr16(m, obj + 0x52u, 0x0080u);                       /* 0x137852 */
        wr16(m, obj + 0x06u, 0x0064u);                       /* 0x137858 (beq 137880 never taken) */
        wr32(m, obj, 0x00137868u);                           /* 0x137860 */
        return 0;
    }

    sub_11B2CA(m, obj, &r->d[0]);                            /* 0x137762 */
    sub_11B262(m, r, obj);                                   /* 0x137768 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x13776E */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x137772 */
        wr32(m, obj, 0x00137758u);                           /* 0x137750 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x08u)); /* 0x137774 {4:1} */
    wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u));      /* 0x13777A */
    wr16(m, obj + 0x20u, 0x4000u);                           /* 0x137780 */
    wr16(m, obj + 0x06u, 0x0064u);                           /* 0x137786 */
    wr32(m, obj, 0x00137794u);                               /* 0x13778C */
    return 0;
}
