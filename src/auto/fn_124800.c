/* ROM 0x124800: stage boss scroll/camera steering tick (A0 = object).
 * While A5+0x30DA > 0xC8, toggles A5+0x30EE bit 5 from MSB inversely to bit 0,
 * steps angle script 0x124B0C (+0x3A index, +0x3E timer), eases +0x20 toward
 * +0x22 by 1/16, advances global camera A5+0x30D2/0x30D6 along +0x20>>8 via
 * 0x11B262 (0x0CBE12), and re-arms 0x124800. When A5+0x30DA <= 0xC8, sets
 * +0x06 = 0x50, +0x0C = 0x124876, plays hooked sound 0x14, resets A5+0x30EE
 * to 0x0001 (bit 15 from MSB), clears A5+0x30F2, sets A5+0x30F3 = 0xFF, and
 * transitions to 0x124B68. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos[angle], D2 = speed * sin[angle]. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * angle);
    int16_t s = (int16_t)rd16(m, 0x027E40u + 2u * angle);
    uint32_t dx = (uint32_t)((int32_t)speed * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->d[1] = dx;
    r->d[2] = dy;
    r->a[3] = 0x00027E40u;
}

/* ROM 0x11B262: step camera scroll (A5+0x30D2, A5+0x30D6) along (+0x20)>>8. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));     /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));     /* 0x11B268 */
    uint16_t ang = rd16(m, obj + 0x20u) >> 8;            /* 0x11B26E..0x11B272 */
    r->d[0] = setw(r->d[0], ang);
    move_by_angle_regs(m, r, ang);                       /* 0x11B274 */
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));     /* 0x11B27A */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));     /* 0x11B280 */
}

int Fn_124800(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x00C8) {   /* 0x124800..0x124806 */
        wr16(m, obj + 0x06u, 0x0050u);                   /* 0x124862 */
        wr32(m, obj + 0x0Cu, 0x00124876u);               /* 0x124868 */
        Sound_PlayHooked(m, 0x0014u);                    /* 0x124B3A..0x124B4A */
        wr32(m, F3_A5 + 0x30EEu, 0u);                    /* 0x124B4E */
        wr8(m, F3_A5 + 0x30EFu, 0x01u);                  /* 0x124B52 bfset {15:1} */
        wr8(m, F3_A5 + 0x30F2u, 0u);                     /* 0x124B58 */
        wr8(m, F3_A5 + 0x30F3u, 0xFFu);                  /* 0x124B5C */
        wr32(m, obj, 0x00124B68u);                       /* 0x124B60 */
        return 0;
    }

    uint8_t flags = rd8(m, F3_A5 + 0x30EEu);             /* 0x12480A */
    if ((flags & 0x80u) == 0u)                           /* seq D0b; bfins {5:1} */
        flags |= 0x04u;
    else
        flags &= (uint8_t)~0x04u;
    wr8(m, F3_A5 + 0x30EEu, flags);

    uint32_t a1 = 0x00124B0Cu;                           /* 0x124818 */
    r->a[1] = a1;
    uint16_t d0w = rd16(m, obj + 0x3Au);                 /* 0x12481E */
    int16_t d1w = (int16_t)rd16(m, obj + 0x3Eu);         /* 0x124822 */
    wr16(m, obj + 0x3Eu, (uint16_t)(d1w + 1));           /* 0x124826 */
    if (d1w >= (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0w)) { /* 0x12482A */
        wr16(m, obj + 0x3Eu, 0u);                        /* 0x124832 */
        d0w = (uint16_t)(d0w + 4u);                      /* 0x124836 */
        if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0w) < 0) /* 0x124838 */
            d0w = 0u;
    }
    wr16(m, obj + 0x3Au, d0w);                           /* 0x124842 */
    wr16(m, obj + 0x22u, rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0w + 2u)); /* 0x124846 */

    int16_t diff = (int16_t)(rd16(m, obj + 0x22u) - rd16(m, obj + 0x20u)); /* 0x12484C */
    diff = (int16_t)(diff >> 4);                         /* 0x124854 asr.w #4 */
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + diff)); /* 0x124856 */

    sub_11B262(m, r);                                    /* 0x12485A */
    wr32(m, obj, 0x00124800u);                           /* 0x1247F8 */
    return 0;
}
