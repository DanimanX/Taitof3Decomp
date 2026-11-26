/* ROM 0x1376B8: camera/scroll vertical follow phase (A0 = object).
 * If A5+0x32D6 < 1: tracks player (A5+0x30E2)->+0x18 + 0x10 against scroll
 * A5+0x30D6 to choose +0x54.b target (+5 or -5), runs 0x11B2CA twice, steps
 * scroll via 0x11B262, decrements +0x06 and re-arms 0x1376B8 while > 0.
 * When +0x06 <= 0 and A5+0x3318 <= 0xBB8, sets +0x20 = -0x8000, +0x54.b = 4,
 * and arms 0x137594. Otherwise sets A5+0x32D6 = 1 (when <= 0), clears
 * A5+0x3318, sets +0x20 = -0x8000, +0x54.b = 4, and arms 0x137726. */
#include "fn_auto.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B2CA: ease +0x56.w toward +0x54.b and store (+0x56 >> 8) into +0x42. */
static void sub_11B2CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int8_t target = (int8_t)rd8(m, obj + 0x54u);         /* 0x11B2CA */
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);            /* 0x11B2CE */
    if (target != cur) {                                 /* 0x11B2D2 beq */
        if (target > cur)                                /* 0x11B2D6 bgt */
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u)); /* 0x11B2E4 */
        else
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u)); /* 0x11B2DA */
    }
    int16_t d0w = (int16_t)rd16(m, obj + 0x56u) >> 8;    /* 0x11B2EA..0x11B2EE */
    wr16(m, obj + 0x42u, (uint16_t)d0w);                 /* 0x11B2F0 */
    r->d[0] = setw(r->d[0], (uint16_t)d0w);
}

/* ROM 0x11B262: step camera scroll A5+0x30D2/+0x30D6 along (+0x20)>>8. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));      /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));      /* 0x11B268 */
    uint16_t ang = rd16(m, obj + 0x20u) >> 8;            /* 0x11B26E..0x11B272 */
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    Obj_MoveByAngle(m, obj, ang);                        /* 0x11B274 -> 0x0CBE12 */
    r->d[0] = setw(r->d[0], ang);
    r->d[1] = (uint32_t)((int32_t)speed * (int32_t)cos_v);
    r->a[3] = 0x00027E40u;
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));      /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));      /* 0x11B280 */
}

int Fn_1376B8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 1)          /* 0x1376B8..0x1376BE */
        goto phase_end;

    {
        uint32_t a1 = rd32(m, F3_A5 + 0x30E2u);          /* 0x1376C2 */
        r->a[1] = a1;
        uint16_t d0w = (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u); /* 0x1376C6..0x1376CA */
        r->d[0] = setw(r->d[0], d0w);
        if ((int16_t)d0w >= (int16_t)rd16(m, F3_A5 + 0x30D6u)) /* 0x1376CE..0x1376D2 */
            wr8(m, obj + 0x54u, 0x05u);                  /* 0x1376E0 */
        else
            wr8(m, obj + 0x54u, 0xFBu);                  /* 0x1376D6 */
    }

    sub_11B2CA(m, r);                                    /* 0x1376E6 */
    sub_11B2CA(m, r);                                    /* 0x1376EC */
    sub_11B262(m, r);                                    /* 0x1376F2 */

    {
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x1376F8 */
        wr16(m, obj + 0x06u, cnt);
        if ((int16_t)cnt > 0) {                          /* 0x1376FC -> 0x1376B0 */
            wr32(m, obj + 0x00u, 0x001376B8u);
            return 0;
        }
    }

    if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x0BB8) {   /* 0x1376FE..0x137704 */
        wr16(m, obj + 0x20u, 0x8000u);                   /* 0x137580 */
        wr8(m, obj + 0x54u, 0x04u);                      /* 0x137586 */
        wr32(m, obj + 0x00u, 0x00137594u);               /* 0x13758C */
        return 0;
    }

    wr16(m, F3_A5 + 0x32D6u, 0x0001u);                   /* 0x137708 */

phase_end:
    wr16(m, F3_A5 + 0x3318u, 0x0000u);                   /* 0x13770E */
    wr16(m, obj + 0x20u, 0x8000u);                       /* 0x137712 */
    wr8(m, obj + 0x54u, 0x04u);                          /* 0x137718 */
    wr32(m, obj + 0x00u, 0x00137726u);                   /* 0x13771E */
    return 0;
}
