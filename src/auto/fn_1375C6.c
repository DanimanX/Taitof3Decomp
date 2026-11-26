/* ROM 0x1375C6: stage/camera scroll controller intro phase tick (A0 = object).
 * If (int16_t)(A5+0x32D6) >= 1, clears A5+0x3318, sets +0x20=0x8000, +0x54=4,
 * and transitions to 0x137726. Otherwise ramps speed (+0x56 -> +0x42) via
 * 0x11B2CA, steps camera scroll via 0x11B262 (0x0CBE12), and counts down +0x06:
 * re-arms 0x1375C6 while > 0, else copies A5+0x1D28 to A5+0x30E2, sets
 * +0x20=0x4000, +0x06=0x12C, and transitions to 0x1375FC. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

int Fn_1375C6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 1) {                              /* 0x1375C6..0x1375CC */
        wr16(m, F3_A5 + 0x3318u, 0u);                                          /* 0x13770E */
        wr16(m, obj + 0x20u, 0x8000u);                                         /* 0x137712 */
        wr8(m, obj + 0x54u, 4u);                                               /* 0x137718 */
        wr32(m, obj, 0x00137726u);                                             /* 0x13771E */
        return 0;
    }

    sub_11B2CA(m, obj, &r->d[0]);                                              /* 0x1375D0 */
    sub_11B262(m, r, obj);                                                     /* 0x1375D6 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);                      /* 0x1375DC */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                                    /* 0x1375E0 */
        wr32(m, obj, 0x001375C6u);                                             /* 0x1375BE */
        return 0;
    }

    wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u));                        /* 0x1375E2 */
    wr16(m, obj + 0x20u, 0x4000u);                                             /* 0x1375E8 */
    wr16(m, obj + 0x06u, 0x012Cu);                                             /* 0x1375EE */
    wr32(m, obj, 0x001375FCu);                                                 /* 0x1375F4 */
    return 0;
}
