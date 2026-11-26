/* ROM 0x1375FC: boss/stage camera scroll phase (A0 = object).
 * If (int16_t)(A5+0x32D6) >= 1: clears A5+0x3318, sets angle +0x20 = 0x8000,
 * target speed +0x54 = 4, and transitions to 0x137726.
 * Otherwise sets +0x54 to -3 or +3 depending on whether ((A5+0x30E2)->+0x18 +
 * 0x10) < (int16_t)(A5+0x30D6), slews +0x56 toward +0x54 via 0x11B2CA, steps
 * camera scroll A5+0x30D2/+0x30D6 via 0x11B262, sets bit ((+0x06 >> 6) & 3)
 * of A5+0x30EE when non-zero, and decrements +0x06. While +0x06 > 0 re-arms
 * 0x1375FC; otherwise resets +0x56=0x800, +0x54=0, +0x20=0, +0x06=0x50 and
 * transitions to 0x137678. */
#include "fn_auto.h"
#include "../player/player_move.h"

/* ROM 0x11B2CA: slew +0x56 toward (int8_t)+0x54 and update speed +0x42. */
static uint16_t sub_11B2CA(const f3mem_t *m, uint32_t obj)
{
    int8_t target = (int8_t)rd8(m, obj + 0x54u);               /* 0x11B2CA */
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);                  /* 0x11B2CE */
    if (target > cur)                                          /* 0x11B2D6 */
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u)); /* 0x11B2E4 */
    else if (target < cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u)); /* 0x11B2DA */

    int16_t spd = (int16_t)rd16(m, obj + 0x56u) >> 8;          /* 0x11B2EA..0x11B2EE */
    wr16(m, obj + 0x42u, (uint16_t)spd);                       /* 0x11B2F0 */
    return (uint16_t)spd;
}

/* ROM 0x11B262: step camera scroll A5+0x30D2/+0x30D6 along (+0x20)>>8. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));            /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));            /* 0x11B268 */
    uint16_t ang = rd16(m, obj + 0x20u) >> 8;                  /* 0x11B26E..0x11B272 */
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    Obj_MoveByAngle(m, obj, ang);                              /* 0x11B274 -> 0x0CBE12 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ang;
    r->d[1] = (uint32_t)((int32_t)speed * (int32_t)cos_v);
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));            /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));            /* 0x11B280 */
}

int Fn_1375FC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 1) {              /* 0x1375FC..0x137602 */
        wr16(m, F3_A5 + 0x3318u, 0x0000u);                     /* 0x13770E */
        wr16(m, obj + 0x20u, 0x8000u);                         /* 0x137712 */
        wr8(m, obj + 0x54u, 0x04u);                            /* 0x137718 */
        wr32(m, obj + 0x00u, 0x00137726u);                     /* 0x13771E */
        return 0;
    }

    uint32_t a1 = rd32(m, F3_A5 + 0x30E2u);                    /* 0x137606 */
    r->a[1] = a1;
    int16_t ref_y = (int16_t)(rd16(m, a1 + 0x18u) + 0x0010u);  /* 0x13760A..0x13760E */
    int16_t cam_y = (int16_t)rd16(m, F3_A5 + 0x30D6u);         /* 0x137612 */
    if (ref_y >= cam_y)                                        /* 0x137616 */
        wr8(m, obj + 0x54u, 0x03u);                            /* 0x137624 */
    else
        wr8(m, obj + 0x54u, 0xFDu);                            /* 0x13761A */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | sub_11B2CA(m, obj);    /* 0x13762A */
    sub_11B262(m, r, obj);                                     /* 0x137630 */

    uint32_t bit = (uint32_t)((rd16(m, obj + 0x06u) >> 6) & 3u); /* 0x137636..0x13763C */
    r->d[0] = bit;
    if (bit != 0u) {                                           /* 0x137642 */
        uint8_t cur = rd8(m, F3_A5 + 0x30EEu);
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(cur | (0x80u >> bit))); /* 0x13764C */
    }

    int16_t timer = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x137652 */
    wr16(m, obj + 0x06u, (uint16_t)timer);
    if (timer > 0) {                                           /* 0x137656 */
        wr32(m, obj + 0x00u, 0x001375FCu);                     /* 0x1375F4 */
        return 0;
    }

    wr16(m, obj + 0x56u, 0x0800u);                             /* 0x137658 */
    wr8(m, obj + 0x54u, 0x00u);                                /* 0x13765E */
    wr16(m, obj + 0x20u, 0x0000u);                             /* 0x137664 */
    wr16(m, obj + 0x06u, 0x0050u);                             /* 0x13766A */
    wr32(m, obj + 0x00u, 0x00137678u);                         /* 0x137670 */
    return 0;
}
