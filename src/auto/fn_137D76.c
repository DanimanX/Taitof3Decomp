/* ROM 0x137D76: camera scroll-track phase tick (A0 = object).
 * Steers +0x20.b toward (+0x50, +0x52) from (A5+0x30D2, A5+0x30D6) via 0x11B288
 * (0x006422), ramps speed +0x56/+0x42 toward +0x54 via 0x11B2CA, and steps
 * scroll via 0x11B262. Decrements +0x06 and re-arms 0x137D76 while > 0; on 0
 * sets A5+0x30EE{0x14:1} (A5+0x30F0 bit 3), sets +0x06 = 0x1E0, and arms
 * 0x137DA4. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B288: steer heading +0x20.b toward (+0x50, +0x52) from scroll. */
static void sub_11B288(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t dx = (int16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u)); /* 0x11B288..0x11B28C */
    int16_t dy = (int16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u)); /* 0x11B290..0x11B294 */
    uint16_t ang = Obj_AtanByte(m, dx, dy);                                  /* 0x11B298 -> 0x006422 */

    int8_t diff = (int8_t)((uint8_t)ang - rd8(m, obj + 0x20u));              /* 0x11B29E */
    if (diff > 0) {                                                          /* 0x11B2A2..0x11B2A4 */
        if (diff > 2)                                                        /* 0x11B2A8..0x11B2AC */
            diff = 2;                                                        /* 0x11B2B0 */
    } else if (diff < -2) {                                                  /* 0x11B2B8..0x11B2BC */
        diff = -2;                                                           /* 0x11B2C0 */
    }
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));     /* 0x11B2C4 */
    r->d[0] = setw(r->d[0], (uint16_t)((ang & 0xFF00u) | (uint8_t)diff));
}

/* ROM 0x11B2CA: ramp speed +0x56.w toward +0x54.b and update +0x42.w. */
static void sub_11B2CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int8_t target = (int8_t)rd8(m, obj + 0x54u);                             /* 0x11B2CA */
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);                                /* 0x11B2CE */
    if (target > cur)                                                        /* 0x11B2D2..0x11B2D6 */
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));    /* 0x11B2E4 */
    else if (target < cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));    /* 0x11B2DA */

    uint16_t d0w = (uint16_t)((int16_t)rd16(m, obj + 0x56u) >> 8);           /* 0x11B2EA..0x11B2EE */
    wr16(m, obj + 0x42u, d0w);                                               /* 0x11B2F0 */
    r->d[0] = setw(r->d[0], d0w);
}

/* ROM 0x11B262: step camera scroll A5+0x30D2/+0x30D6 along (+0x20)>>8. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));                          /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));                          /* 0x11B268 */
    uint16_t ang = rd16(m, obj + 0x20u) >> 8;                                /* 0x11B26E..0x11B272 */
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    Obj_MoveByAngle(m, obj, ang);                                            /* 0x11B274 -> 0x0CBE12 */
    r->d[0] = setw(r->d[0], ang);
    r->d[1] = (uint32_t)((int32_t)speed * (int32_t)c);
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));                          /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));                          /* 0x11B280 */
}

int Fn_137D76(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11B288(m, r);                                                        /* 0x137D76 */
    sub_11B2CA(m, r);                                                        /* 0x137D7C */
    sub_11B262(m, r);                                                        /* 0x137D82 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);                    /* 0x137D88 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                                  /* 0x137D8C -> 0x137D6E */
        wr32(m, obj, 0x00137D76u);
        return 0;
    }

    wr8(m, F3_A5 + 0x30F0u, (uint8_t)(rd8(m, F3_A5 + 0x30F0u) | 0x08u));     /* 0x137D8E bfset {0x14:1} */
    wr16(m, obj + 0x06u, 0x01E0u);                                           /* 0x137D94 (0x137D9A beq folded) */
    wr32(m, obj, 0x00137DA4u);                                               /* 0x137D9C */
    return 0;
}
