/* ROM 0x1657EE: stage-clear/transition line-RAM fade & BG setup tick (A0 = obj).
 * Counts down +0x06; when <= 0, fills 232 words of line RAM at 0x626200 +
 * (A5-0x5D56) with inverted RGB fade mask built from (4, +0x44) clamped to 8,
 * then decrements +0x44 and re-arms (+0x06 = 8) while >= 0. When +0x44 < 0,
 * clears PF1/line RAM (0x005956), resets scroll/layer globals (A5+0x420C..
 * A5+0x4286, 0x40C26C..0x40C27C), fills 0x62B200 (0xB00D) and 0x62B400 (0x3008)
 * via 0x176198, queues palette 0x1663C4 -> 0x5900 (0x20), spawns 0x1121AE,
 * and transitions to 0x1659B0. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x176198: fill 0xE8 words (0x1D0 bytes) backwards from buf+(A5-0x5D56)+0x1D0. */
static void sub_176198(const f3mem_t *m, uint32_t buf, uint16_t val)
{
    uint32_t a0 = buf + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u) + 0x01D0u;
    for (int i = 0; i < 0xE8; i++) {
        a0 -= 2u;
        wr16(m, a0, val);
    }
}

int Fn_1657EE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x1657EE */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {
        wr32(m, obj, 0x001657EEu);                      /* 0x1657E6 */
        return 0;
    }

    /* 0x1657F4..0x16585C: fill 0x626200 + (A5-0x5D56) with fade mask */
    uint16_t d0w = 4u;
    uint16_t d1w = rd16(m, obj + 0x44u);
    d0 = setw(d0, d0w);
    d1 = setw(d1, d1w);
    {
        uint32_t dst = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        uint16_t a = d0w & 0x000Fu;
        if ((int16_t)a > 8)
            a = 8u;
        uint16_t b = d1w & 0x000Fu;
        if ((int16_t)b > 8)
            b = 8u;
        uint16_t hi = (uint16_t)(((a << 8) + (uint16_t)(8u - a)) << 4);
        uint16_t lo = (uint16_t)((b << 8) + (uint16_t)(8u - b));
        uint16_t mask = (uint16_t)(~(hi | lo));
        for (int16_t d7 = 0x00E7; d7 >= 0; d7--)
            wr16(m, dst + (uint32_t)d7 * 2u, mask);
    }

    int16_t step = (int16_t)(rd16(m, obj + 0x44u) - 1u); /* 0x165860 */
    wr16(m, obj + 0x44u, (uint16_t)step);
    if (step >= 0) {
        wr16(m, obj + 0x06u, 8u);                       /* 0x1657DE: beq never taken */
        wr32(m, obj, 0x001657EEu);
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;
    }

    /* 0x165868..0x1659B0: final reset, upload, spawn 0x1121AE, transition */
    Gfx_ClearPf1AndLineRam(m);                          /* 0x16586C */
    Mem_WritePair64(m, F3_A5 + 0x420Cu, F3_A5 - 0x5DA0u, 0u, 0u); /* 0x16588A */
    Mem_WritePair64(m, F3_A5 + 0x4214u, F3_A5 - 0x5D98u, 0u, 0u); /* 0x1658A8 */
    a1 = F3_A5 - 0x5D94u;
    Obj_16AB26(m, F3_A5 + 0x422Cu, 0x0000E000u, 0u);    /* 0x1658C2: D0=0xE000, D1=0 */
    Obj_16AB26(m, F3_A5 + 0x4234u, 0x0000E000u, 0u);    /* 0x1658DC */
    wr8(m, F3_A5 + 0x4285u, 4u);                        /* 0x1658EA */
    wr8(m, F3_A5 + 0x4286u, 4u);                        /* 0x1658F8 */
    wr32(m, 0x0040C26Cu, 0x00166626u);                  /* 0x165914 */
    wr32(m, 0x0040C270u, 0x001666C6u);
    wr32(m, 0x0040C274u, 0x00166676u);                  /* 0x165930 */
    wr32(m, 0x0040C278u, 0x001666E2u);
    sub_176198(m, 0x0062B200u, 0xB00Du);                /* 0x165940 (-0x4FF3) */
    sub_176198(m, 0x0062B400u, 0x3008u);                /* 0x165954 */
    wr16(m, F3_A5 + 0x425Eu, 0x03BFu);                  /* 0x16595E */
    wr16(m, F3_A5 + 0x4260u, 0x03BFu);                  /* 0x165964 */
    Pal_QueueUpload(m, 0x001663C4u, 0x5900u, 0x0020u);  /* 0x16597C */

    d0 = Obj_Spawn(m, obj, 0x001121AEu, 0u, 1u, 0u);    /* 0x16599E */
    d1 = 0x001666E2u;
    wr32(m, obj, 0x001659B0u);                          /* 0x1659A8 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
