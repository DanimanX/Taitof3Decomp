#include "obj_12EE24.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CBDDE: A1 = A0+0x36, then follow +0x8 when +0x35 is 1 or 2 */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t s = rd8(m, obj + 0x35u);
    if (s == 1u || s == 2u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x0CBDA4: append A0 to list A5+0x8FE */
static void __attribute__((unused)) sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
}

/* ROM 0x0CBB0C: warp target when bit 2 of A5+0x1D58 is set */
static void __attribute__((unused)) sub_0CBB0C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x04u) {
        wr8(m, obj + 0x35u, 0x01u);
        wr32(m, obj + 0x36u, rd32(m, F3_A5 + 0x1D54u));
        wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) - 0x4000u);
    }
}

/* ROM 0x11AE62: A0+0x24 is added to the global A5+0x30DA counter; when it
 * reaches 0 A1 = A0+0x36 (+0x8 for states 1/2) is re-summed. D2 = A0+0x24,
 * scaled by D1w according to the target's kind. */
static void __attribute__((unused)) sub_11AE62(const f3mem_t *m, uint32_t obj, int32_t d2, uint16_t d1w, uint32_t d0)
{
    if (d2 != 0)
        return;
    wr32(m, obj + 0x24u, 0u);
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0 && k != 4) {
        d2 = d2 >> (d1w & 31u);
        if (k != 1 && k != 5)
            d2 = (int32_t)((uint32_t)d2 >> 1);
    }
    wr32(m, F3_A5 + 0x30DAu, rd32(m, F3_A5 + 0x30DAu) + (uint32_t)d2);
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0)
        return;
    a1 = sub_0CBDDE(m, obj);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x11C096: spread a shared hit counter over 8 line-RAM slots */
static void __attribute__((unused)) sub_11C096(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0)
        return;
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t s = rd8(m, obj + 0x35u);
    wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | s));
    if (rd8(m, obj + 0x34u) & 0x02u)
        return;

    uint16_t d0w = (uint16_t)(rd16(m, F3_A5 + 0x878u) - rd16(m, F3_A5 + 0x311Cu));
    wr16(m, F3_A5 + 0x311Cu, rd16(m, F3_A5 + 0x878u));
    if ((int16_t)d0w < 2) {
        uint32_t d0 = rd32(m, F3_A5 + 0x3116u);
        wr32(m, F3_A5 + 0x3116u, obj);
        if (d0 == obj) {
            wr16(m, F3_A5 + 0x311Au, (uint16_t)(rd16(m, F3_A5 + 0x311Au) + 1u));
            if ((int16_t)rd16(m, F3_A5 + 0x311Au) <= 3)
                return;
        }
    }

    wr16(m, F3_A5 + 0x311Au, 0u);
    uint32_t a2 = 0x0040B0F6u;
    int16_t d0w2;
    for (d0w2 = 7; d0w2 >= 0; d0w2--) {
        uint8_t b = rd8(m, a2);
        wr8(m, a2, (uint8_t)(b | 0x80u));
        if (!(b & 0x80u))
            break;
        a2 += 4u;
    }
    if (d0w2 < 0)
        return;

    a1 = rd32(m, obj + 0x36u);
    int16_t x0, y0;
    if (rd8(m, a1 + 0x22u) == 6u || rd8(m, obj + 0x35u) == 2u) {
        uint16_t r = (uint16_t)Rng_NextMem(m);
        uint16_t q1, q2;
        uint16_t d = (uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u));
        if (d != 0) {
            uint32_t v = ((uint32_t)r / d);
            if (v <= 0xFFFFu) q1 = (uint16_t)((uint32_t)r % d);
            else q1 = r;   /* divu.w overflow: D1 keeps its original high word */
        } else {
            q1 = r;
        }
        y0 = (int16_t)(uint16_t)(rd16(m, obj + 0x30u) + q1 + rd16(m, obj + 0x18u));

        r = (uint16_t)Rng_NextMem(m);
        d = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
        if (d != 0) {
            uint32_t v = ((uint32_t)r / d);
            q2 = (v <= 0xFFFFu) ? (uint16_t)((uint32_t)r % d) : r;
        } else {
            q2 = r;
        }
        x0 = (int16_t)(uint16_t)(rd16(m, obj + 0x2Cu) + q2 + rd16(m, obj + 0x14u));
    } else {
        int16_t t0 = (int16_t)(uint16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x14u));
        int16_t t1 = (int16_t)(uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x14u));
        if (t0 < t1) t0 = t1;
        int16_t t2 = (int16_t)(uint16_t)(rd16(m, obj + 0x2Eu) + rd16(m, obj + 0x14u));
        int16_t t3 = (int16_t)(uint16_t)(rd16(m, a1 + 0x2Eu) + rd16(m, a1 + 0x14u));
        if (t2 > t3) t2 = t3;
        x0 = (int16_t)((uint16_t)((uint16_t)t0 + (uint16_t)t2) >> 1);   /* asr.w */

        t0 = (int16_t)(uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x18u));
        t1 = (int16_t)(uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x18u));
        if (t0 < t1) t0 = t1;
        t2 = (int16_t)(uint16_t)(rd16(m, obj + 0x32u) + rd16(m, obj + 0x18u));
        t3 = (int16_t)(uint16_t)(rd16(m, a1 + 0x32u) + rd16(m, a1 + 0x18u));
        if (t2 > t3) t2 = t3;
        y0 = (int16_t)((uint16_t)((uint16_t)t0 + (uint16_t)t2) >> 1);
    }

    x0 = (int16_t)(uint16_t)(x0 - rd16(m, F3_A5 + 0x30D2u));
    y0 = (int16_t)(uint16_t)(y0 - rd16(m, F3_A5 + 0x30D6u));
    uint16_t w = rd16(m, a2);
    w = (uint16_t)((w & ~0x07FFu) | ((uint16_t)x0 & 0x07FFu));
    w = (uint16_t)((w & 0x03FFu) | (((uint16_t)y0 << 11) & 0xFC00u));
    wr16(m, a2, w);
}

/* ROM 0x12EE24 */
int Obj_12EE24_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x02u) {
        Obj_Free(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 + 0x30F2u) & 0x04u) {
        Obj_Free(m, obj);
        return 0;
    }

    /* draw the whole player sprite chain */
    uint32_t a0 = rd32(m, F3_A5 + 0x32AAu);
    for (;;) {
        a0 = rd32(m, a0 + 0x66u);
        if (rd32(m, a0 + 0x66u) == 0)
            break;
        Obj_DrawSprite(m, a0);
    }

    sub_11AE62(m, obj, 0x186A0, 2, 0x18600);
    wr32(m, obj + 0x00u, 0x0012EE24u);
    return 0;
}
