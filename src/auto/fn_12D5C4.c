/* ROM 0x12D5C4: stage transition delay / GFX scroll ramp (A0 = object).
 * Decrements +0x06 and re-arms 0x12D5C4 while > 0. Once <= 0, fills 42 words
 * at 0x62777C/0x62617C + (A5-0x5D56), clears A5+0x30F3, plays sound 0x13,
 * configures A5+0x32A4/32A8, and runs the 0x12E422 ramp toward A5+0x32A2 =
 * 0x8000 (stepping via 0x12E442 until within 0x0300, then 0x12D62C -> 0x12D63C). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_12D5C4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t timer = (int16_t)(rd16(m, obj + 0x06u) - 1u);        /* 0x12D5C4 */
    wr16(m, obj + 0x06u, (uint16_t)timer);
    if (timer > 0) {                                             /* 0x12D5C8 bgt */
        wr32(m, obj + 0x00u, 0x0012D5C4u);                       /* 0x12D5BC */
        return 0;
    }

    int32_t off = (int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);    /* 0x12D5D0..0x12D5DA */
    uint32_t a1 = (uint32_t)(0x0062777Cu + off);                 /* 0x12D5CA */
    uint32_t a2 = (uint32_t)(0x0062617Cu + off);                 /* 0x12D5D4 */
    for (int i = 0; i < 42; i++) {                               /* 0x12D5DE..0x12D5EE dbf 0x29 */
        wr16(m, a1, 0xDDDDu);                                    /* -0x2223 */
        a1 += 2u;
        wr16(m, a2, 0x02FFu);
        a2 += 2u;
    }

    wr8(m, F3_A5 + 0x30F3u, 0u);                                 /* 0x12D5F2 sf */
    Sound_PlayHooked(m, 0x0013u);                                /* 0x12D5F6..0x12D606 */
    wr16(m, F3_A5 + 0x32A4u, 0u);                                /* 0x12D60A */
    wr16(m, F3_A5 + 0x32A8u, 2u);                                /* 0x12D610 */
    wr32(m, obj + 0x0Cu, 0x0012D62Cu);                           /* 0x12D61E */

    /* 0x12D626 jmp 0x12E422(D0w = 0x8000, D1w = -0x0300) */
    uint16_t d0w = 0x8000u;                                      /* 0x12D616 */
    uint16_t d1w = 0xFD00u;                                      /* 0x12D61A */
    wr16(m, obj + 0x20u, d0w);                                   /* 0x12E422 */
    wr16(m, obj + 0x42u, d1w);                                   /* 0x12E426 */
    if ((int16_t)d1w < 0)                                        /* 0x12E42A */
        d1w = (uint16_t)(-(int16_t)d1w);                         /* 0x12E430 */
    wr16(m, obj + 0x22u, d1w);                                   /* 0x12E432 */

    /* 0x12E44A */
    int16_t diff = (int16_t)(rd16(m, F3_A5 + 0x32A2u) - d0w);    /* 0x12E44A..0x12E44E */
    if (diff < 0)                                                /* 0x12E452 */
        diff = (int16_t)(-diff);                                 /* 0x12E458 */
    d0w = (uint16_t)diff;

    if (d0w >= d1w) {                                            /* 0x12E45A..0x12E45E bcc */
        wr32(m, obj + 0x00u, 0x0012E442u);                       /* 0x12E46C -> 0x12E43A */
    } else {
        wr16(m, F3_A5 + 0x32A2u, 0x8000u);                       /* 0x12E462 */
        a1 = rd32(m, obj + 0x0Cu);                               /* 0x12E46E -> 0x12D62C */
        wr16(m, obj + 0x06u, 0x001Eu);                           /* 0x12D62C */
        wr32(m, obj + 0x00u, 0x0012D63Cu);                       /* 0x12D634 */
    }
    r->d[0] = setw(r->d[0], d0w);
    r->d[1] = setw(r->d[1], d1w);
    r->a[1] = a1;
    return 0;
}
