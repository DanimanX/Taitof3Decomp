/* ROM 0x137594: camera scroll approach tick (A0 = object).
 * If (int16_t)A5+0x32D6 >= 1, clears A5+0x3318, sets +0x20 = 0x8000 and
 * +0x54 = 4, and transitions to 0x137726. Otherwise eases +0x56/+0x42 via
 * 0x11B2CA, integrates camera position (A5+0x30D2, A5+0x30D6) via 0x11B262
 * (0x0CBE12), and re-arms 0x137594 while (int16_t)A5+0x30D2 > 0x334; on
 * completion clears +0x54, sets +0x06 = 0x0A, and transitions to 0x1375C6. */
#include "fn_auto.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B2CA: ease +0x56.w toward +0x54.b and store (+0x56 >> 8) into +0x42. */
static void sub_11B2CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int8_t target = (int8_t)rd8(m, obj + 0x54u);
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);
    if (target > cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));
    else if (target < cur)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));
    int16_t spd = ASR16(rd16(m, obj + 0x56u), 8);
    wr16(m, obj + 0x42u, (uint16_t)spd);
    r->d[0] = setw(r->d[0], (uint16_t)spd);
}

/* ROM 0x11B262: integrate A5+0x30D2/30D6 along +0x20.b at speed +0x42 via 0x0CBE12. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    r->d[0] = setw(r->d[0], ang);

    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + 2u * ang);
    int32_t vx = (int32_t)speed * (int32_t)c;
    int32_t vy = (int32_t)speed * (int32_t)s;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)vy);
    r->d[1] = (uint32_t)vx;
    r->d[2] = (uint32_t)vy;
    r->a[3] = 0x00027E40u;

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

int Fn_137594(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 1) {          /* 0x137594..0x13759A */
        wr16(m, F3_A5 + 0x3318u, 0u);                       /* 0x13770E */
        wr16(m, obj + 0x20u, 0x8000u);                      /* 0x137712 */
        wr8(m, obj + 0x54u, 4u);                            /* 0x137718 */
        wr32(m, obj, 0x00137726u);                          /* 0x13771E */
        return 0;
    }

    sub_11B2CA(m, r);                                       /* 0x13759E */
    sub_11B262(m, r);                                       /* 0x1375A4 */

    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x0334) {       /* 0x1375AA..0x1375B0 */
        wr32(m, obj, 0x00137594u);                          /* 0x13758C */
        return 0;
    }

    wr8(m, obj + 0x54u, 0u);                                /* 0x1375B2 */
    wr16(m, obj + 0x06u, 0x000Au);                          /* 0x1375B8 */
    wr32(m, obj, 0x001375C6u);                              /* 0x1375BE */
    return 0;
}
