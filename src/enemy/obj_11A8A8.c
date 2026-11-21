#include "obj_11A8A8.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0017D6 */
static void sub_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d1, uint32_t d2, uint32_t d6)
{
    uint32_t a2 = (F3_A5 - 0x7F50u) + d6;
    int32_t div = (int32_t)d1;
    uint16_t d7 = (uint16_t)((uint16_t)d2 - 1u);

    /* 0017E2 */
    for (;;) {
        uint32_t c0 = rd32(m, a0);
        a0 += 4u;
        uint32_t c1 = rd32(m, a1);
        a1 += 4u;

        /* 0017E6 */
        uint32_t s = ((c0 >> 16) & 0xFFu) << 8;
        wr16(m, a2 + 0x00u, (uint16_t)s);
        uint32_t e = ((c1 >> 16) & 0xFFu) << 8;
        int32_t step = (int32_t)(e - s) / div;
        wr16(m, a2 + 0x06u, (uint16_t)(uint32_t)step);

        /* 0017FE */
        s = ((c0 >> 8) & 0xFFu) << 8;
        wr16(m, a2 + 0x02u, (uint16_t)s);
        e = ((c1 >> 8) & 0xFFu) << 8;
        step = (int32_t)(e - s) / div;
        wr16(m, a2 + 0x08u, (uint16_t)(uint32_t)step);

        /* 001818 */
        s = (c0 & 0xFFu) << 8;
        wr16(m, a2 + 0x04u, (uint16_t)s);
        e = (c1 & 0xFFu) << 8;
        step = (int32_t)(e - s) / div;
        wr16(m, a2 + 0x0Au, (uint16_t)(uint32_t)step);

        /* 001832 */
        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x001792 */
static uint32_t sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                           uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    uint16_t d7 = 0x000Fu;

    /* 0017A2 */
    for (;;) {
        if (rd16(m, a3) == 0u) {
            /* 0017A6 */
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return 0xFFFFFFFFu;
            /* 0017AC */
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return 0xFFFFFFFFu;
            /* 0017B0 */
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_0017D6(m, a0, a1, d1, d2, d6);
            return a3;
        }
        /* 0017BC */
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }

    /* 0017C8 */
    return 0xFFFFFFFFu;
}

/* ROM 0x0018BA: palette fade towards the colours at dst (A0 = 0x198E) */
static uint32_t sub_0018BA(const f3mem_t *m, uint32_t dst, uint32_t d0,
                           uint32_t d1, uint32_t d2)
{
    /* lea (0xba,PC),A0 at 0x18D2 -> 0x198E */
    return sub_001792(m, 0x0000198Eu, dst, d0, d1, d2);
}

/* ROM 0x11A8A8 */
int Obj_11A8A8_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, 0x00BE0000u);
    wr32(m, obj + 0x18u, 0x00BE0000u);
    wr32(m, obj + 0x10u, 0x00062DA0u);
    wr8(m, obj + 0x1Du, 0u);
    Obj_DrawSprite(m, obj);

    if ((int16_t)rd16(m, obj + 0x06u) >= 0xC8) {
        wr8(m, obj + 0x1Du, 0x3Au);
        Obj_Animate(m, obj, 0x0011A93Cu);   /* lea (0x60,PC) at 0x11A8DA */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u)));
        Obj_DrawSprite(m, obj);
        wr32(m, obj + 0x2Cu, rd32(m, obj + 0x2Cu) + 0x00010000u);
        /* addi.l #0x0,(0x30,A0): no change */
    }

    /* 0x11A90A */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u));
    if ((int16_t)rd16(m, obj + 0x2Cu) < 0x75) {
        wr32(m, obj + 0x00u, 0x0011A8A8u);
        return 0;
    }
    sub_0018BA(m, 0x0001F100u, 0x00004EC0u, 0x0000001Eu, 0x00000010u);

    /* 0x11A884 */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x2Cu, 0u);
    wr32(m, obj + 0x30u, 0xFFF80000u);
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x00u, 0x0011A8A8u);
    return 0;
}
