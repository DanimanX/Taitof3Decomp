/* ROM 0x137BB6: stage/camera scroll controller phase tick (A0 = object).
 * If (int16_t)(A5+0x32D6) >= 5, clears A5+0x3318, sets target speed +0x54=2,
 * target pos (+0x50,+0x52)=(0x138,0x78), timer +0x06=0x12C and transitions to
 * 0x137C5C. Otherwise steers heading (+0x20) via 0x11B288, ramps speed via
 * 0x11B2CA, steps camera scroll via 0x11B262, and counts down +0x06: re-arms
 * 0x137BB6 while > 0, else clears A5+0x30F0 bit 6, sets A5+0x30EF bit 0, sets
 * +0x06=0xC8 and transitions to 0x137BF2. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x006422 with exact D0 upper-word behaviour (exg.l d1,d0 on quadrant != 0,6). */
static uint32_t atan2_regs(const f3mem_t *m, uint32_t d0_in, uint32_t d1_in)
{
    int16_t x = (int16_t)(uint16_t)d0_in;
    int16_t y = (int16_t)(uint16_t)d1_in;
    uint16_t d2 = 0u;
    if (x < 0) d2 |= 2u;
    if (y < 0) d2 |= 4u;
    uint16_t ang = Obj_Atan2(m, (uint16_t)x, (uint16_t)y);
    uint32_t hi = (d2 != 0u && d2 != 6u) ? (d1_in & 0xFFFF0000u) : (d0_in & 0xFFFF0000u);
    return hi | (uint8_t)ang;
}

/* ROM 0x11B288: steer byte (+0x20) by at most +/-2 toward target (+0x50,+0x52). */
static void sub_11B288(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u)); /* 0x11B288 */
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u)); /* 0x11B290 */
    uint32_t d0 = atan2_regs(m, setw(*d0_io, dx), setw(*d1_io, dy));            /* 0x11B298 */
    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));        /* 0x11B29E */
    if (diff > 0) {
        if (diff > 2)
            diff = 2;
    } else if (diff < -2) {
        diff = -2;
    }
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));       /* 0x11B2C4 */
    *d0_io = (d0 & 0xFFFFFF00u) | (uint8_t)diff;
    *d1_io = setw(*d1_io, dy);
}

/* ROM 0x11B2CA: ramp speed word (+0x56) toward (+0x54) and update (+0x42). */
static void sub_11B2CA(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    uint8_t tgt = rd8(m, obj + 0x54u);                                         /* 0x11B2CA */
    uint8_t cur = rd8(m, obj + 0x56u);                                         /* 0x11B2CE */
    if ((int8_t)tgt < (int8_t)cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));      /* 0x11B2DA */
    else if ((int8_t)tgt > (int8_t)cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));      /* 0x11B2E4 */

    uint16_t spd = (uint16_t)((int16_t)rd16(m, obj + 0x56u) >> 8);             /* 0x11B2EA..0x11B2EE */
    wr16(m, obj + 0x42u, spd);                                                 /* 0x11B2F0 */
    *d0_io = setw(*d0_io, spd);
}

/* ROM 0x11B262: step camera scroll (A5+0x30D2, A5+0x30D6) along (+0x20)>>8 via 0x0CBE12. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));                            /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));                            /* 0x11B268 */
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);                      /* 0x11B26E..0x11B272 */
    r->d[0] = setw(r->d[0], ang);

    int16_t spd = (int16_t)rd16(m, obj + 0x42u);                               /* 0x0CBE18 */
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
    uint32_t dx = (uint32_t)((int32_t)spd * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)spd * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->d[1] = dx;
    r->d[2] = dy;
    r->a[3] = 0x00027E40u;

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));                            /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));                            /* 0x11B280 */
}

int Fn_137BB6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 5) {                              /* 0x137BB6..0x137BBC */
        wr16(m, F3_A5 + 0x3318u, 0u);                                          /* 0x137C36 */
        wr8(m, obj + 0x54u, 2u);                                               /* 0x137C3A */
        wr16(m, obj + 0x50u, 0x0138u);                                         /* 0x137C40 */
        wr16(m, obj + 0x52u, 0x0078u);                                         /* 0x137C46 */
        wr16(m, obj + 0x06u, 0x012Cu);                                         /* 0x137C4C (beq 137C74 never taken) */
        wr32(m, obj, 0x00137C5Cu);                                             /* 0x137C54 */
        return 0;
    }

    sub_11B288(m, obj, &r->d[0], &r->d[1]);                                    /* 0x137BC0 */
    sub_11B2CA(m, obj, &r->d[0]);                                              /* 0x137BC6 */
    sub_11B262(m, r, obj);                                                     /* 0x137BCC */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);                      /* 0x137BD2 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                                    /* 0x137BD6 */
        wr32(m, obj, 0x00137BB6u);                                             /* 0x137BAE */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F0u, (uint8_t)(rd8(m, F3_A5 + 0x30F0u) & ~0x40u));      /* 0x137BD8 {0x11:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x01u));       /* 0x137BDE {0x0F:1} */
    wr16(m, obj + 0x06u, 0x00C8u);                                             /* 0x137BE4 */
    wr32(m, obj, 0x00137BF2u);                                                 /* 0x137BEA */
    return 0;
}
