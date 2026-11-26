/* ROM 0x11EE42: boss arrival/intro deceleration tick (A0 = object).
 * Copies +0x14/+0x18 to A5+0x30D2/30D6, steps A5+0x30D2/30D6 using angle byte
 * +0x20 (sign-extended byte for X, 0x27E40[ang] for Y) scaled by +0x42, and
 * decrements +0x20 by 0x400. While +0x20 > 0, plays sound 0x25, copies A5+0x30D2/
 * 30D6 back to +0x14/+0x18, clears +0x20, decays +0x42 by 3/8, and counts down
 * +0x06. When +0x06 <= 0, clears A5+0x30F3, plays sound 0x13, sets A5+0x30F2 = 4,
 * clears bit 12 and sets bit 13 of A5+0x30EE; if A5+0x30D2 < 0xF0 transitions to
 * 0x11EEFE, else arms boss phase (A5+0x30EE bits, +0x06 = 0xA, +0x20 = -0x8000,
 * +0x42 = 0x80, +0x18 = 0) and transitions to 0x11EF66. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

int Fn_11EE42(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = 0x00027E40u;                           /* 0x11EE4E */

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));      /* 0x11EE42 */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));      /* 0x11EE48 */

    {
        uint16_t ang = rd8(m, obj + 0x20u);              /* 0x11EE54..0x11EE56 */
        int16_t sin_v = (int16_t)rd16(m, a1 + (uint32_t)ang * 2u); /* 0x11EE5A */
        int16_t spd = (int16_t)rd16(m, obj + 0x42u);

        int32_t dx_l = ((int32_t)(int16_t)(int8_t)ang * (int32_t)spd) >> 8; /* 0x11EE60..0x11EE66 */
        d0 = (uint32_t)dx_l;
        wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + (uint16_t)dx_l)); /* 0x11EE68 */

        uint32_t dy_l = (uint32_t)((int32_t)sin_v * (int32_t)spd) << 1; /* 0x11EE6C..0x11EE70 */
        dy_l = (dy_l << 16) | (dy_l >> 16);              /* 0x11EE72 swap D1 */
        d1 = dy_l;
        wr16(m, F3_A5 + 0x30D6u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + (uint16_t)dy_l)); /* 0x11EE74 */
    }

    {
        uint16_t v20 = (uint16_t)(rd16(m, obj + 0x20u) - 0x0400u); /* 0x11EE78 */
        wr16(m, obj + 0x20u, v20);
        if ((int16_t)v20 <= 0) {                         /* 0x11EE7E..0x11EE82 */
            wr32(m, obj + 0x00u, 0x0011EE42u);           /* 0x11EE3A */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    d0 = setw(d0, (uint16_t)Sound_PreCmdHook(m, 0x0025)); /* 0x11EE84..0x11EE94 */
    Sound_PlayCmd(m, (int16_t)d0);

    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));      /* 0x11EE98 */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));      /* 0x11EE9E */
    wr16(m, obj + 0x20u, 0u);                            /* 0x11EEA4 */

    {
        uint16_t d0w = rd16(m, obj + 0x42u);             /* 0x11EEAA */
        d0w >>= 2;                                       /* 0x11EEAE */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - d0w)); /* 0x11EEB0 */
        d0w >>= 1;                                       /* 0x11EEB4 */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - d0w)); /* 0x11EEB6 */
        d0 = setw(d0, d0w);
    }

    {
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11EEBA */
        wr16(m, obj + 0x06u, cnt);
        if ((int16_t)cnt > 0) {                          /* 0x11EEBE */
            wr32(m, obj + 0x00u, 0x0011EE42u);           /* 0x11EE3A */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    wr8(m, F3_A5 + 0x30F3u, 0u);                         /* 0x11EEC2 */
    d0 = setw(d0, (uint16_t)Sound_PreCmdHook(m, 0x0013)); /* 0x11EEC6..0x11EED6 */
    Sound_PlayCmd(m, (int16_t)d0);
    wr8(m, F3_A5 + 0x30F2u, 4u);                         /* 0x11EEDA */

    /* 0x11EEE0 bfclr (0x30ee,A5){0xc:0x1} -> 0x30EF bit 4 (0x08) */
    /* 0x11EEE6 bfset (0x30ee,A5){0xd:0x1} -> 0x30EF bit 5 (0x04) */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)((rd8(m, F3_A5 + 0x30EFu) & ~0x08u) | 0x04u));

    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x00F0) {    /* 0x11EEEC..0x11EEF2 */
        wr32(m, obj + 0x00u, 0x0011EEFEu);               /* 0x11EEF6 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x11EF2E {12:1} */
    wr16(m, obj + 0x06u, 0x000Au);                       /* 0x11EF34 */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x08u)); /* 0x11EF3A {4:1} */
    if (rd8(m, F3_A5 + 0x3260u) == 0u)                   /* 0x11EF40..0x11EF44 */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u)); /* 0x11EF48 {3:1} */

    wr16(m, obj + 0x20u, 0x8000u);                       /* 0x11EF4E */
    wr16(m, obj + 0x42u, 0x0080u);                       /* 0x11EF54 */
    wr32(m, obj + 0x18u, 0u);                            /* 0x11EF5A */
    wr32(m, obj + 0x00u, 0x0011EF66u);                   /* 0x11EF5E */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
