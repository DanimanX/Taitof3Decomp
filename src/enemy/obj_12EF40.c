#include "obj_12EF40.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CBD24: step the animation table at A1 by the index/timer in
 * +0x3A/+0x3E; returns the frame offset (D0w) */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        uint16_t d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, d1);
        if ((int16_t)d1 <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* bfclr (0x30ee,A5){n:1}: returns the old bit */
static int bfclr_30ee(const f3mem_t *m, unsigned n)
{
    uint32_t a = F3_A5 + 0x30EEu + n / 8u;
    uint8_t mask = (uint8_t)(0x80u >> (n % 8u));
    uint8_t b = rd8(m, a);
    wr8(m, a, (uint8_t)(b & (uint8_t)~mask));
    return (b & mask) != 0u;
}

/* ROM 0x12F160 */
static void kill(const f3mem_t *m, uint32_t obj)
{
    wr8(m, rd32(m, obj + 0x0Cu) + 0x3Eu, 0u);
    Obj_Free(m, obj);
}

static int aborted(const f3mem_t *m) { return (rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u; }

/* animate with table a1; nonzero if still running (fn set to next) */
static int animate(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t next)
{
    uint16_t d0 = sub_0CBD24(m, obj, a1);
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) <= 0)
        return 0;
    uint32_t p = rd32(m, obj + 0x0Cu);
    wr8(m, p + 0x3Eu, rd32(m, obj + 0x10u) != 0x00028740u ? 0xFFu : 0x00u);
    wr32(m, obj + 0x00u, next);
    return 1;
}

/* ROM 0x12EF40 */
int Obj_12EF40_Tick(const f3mem_t *m, uint32_t obj)
{
    if (!bfclr_30ee(m, 9)) {
        wr32(m, obj + 0x00u, 0x0012EF40u);
        return 0;
    }
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);

    /* 0x12EF50 */
    if (aborted(m)) { kill(m, obj); return 0; }
    /* lea (0x21a,PC),A1 at 0x12EF5A -> 0x12F176 */
    if (animate(m, obj, 0x0012F176u, 0x0012EF86u))
        return 0;

    /* 0x12EF9E */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x44u, 0u);
    wr16(m, obj + 0x46u, 0u);
    wr16(m, obj + 0x48u, 0u);
    {
        uint32_t a1 = obj + 0x4Au;
        uint32_t a2 = 0x0012F22Eu;   /* lea (0x276,PC) at 0x12EFB6 */
        uint32_t a3 = rd32(m, obj + 0x0Cu);
        for (int i = 0; i < 4; i++) {
            uint16_t d2 = rd16(m, a2); a2 += 2u;
            if (rd8(m, a3 + 0x1Cu) & 0x01u)
                d2 = (uint16_t)((uint16_t)(0u - (uint16_t)(d2 - 0x4000u)) + 0x4000u);
            wr16(m, a1, d2); a1 += 2u;
            wr16(m, a1, rd16(m, a2)); a1 += 2u; a2 += 2u;
        }
    }

    /* 0x12EFE2 */
    if (aborted(m)) { kill(m, obj); return 0; }
    if (!bfclr_30ee(m, 10)) {
        wr32(m, obj + 0x00u, 0x0012EFFEu);
        return 0;
    }

    /* 0x12F08E */
    if (aborted(m)) { kill(m, obj); return 0; }
    {
        uint16_t c = (uint16_t)(rd16(m, obj + 0x48u) - 1u);
        wr16(m, obj + 0x48u, c);
        if ((int16_t)c >= 0) {
            wr32(m, obj + 0x00u, 0x0012F0A8u);
            return 0;
        }
    }

    /* 0x12F0FE */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    if (aborted(m)) { kill(m, obj); return 0; }
    /* lea (0xc6,PC),A1 at 0x12F110 -> 0x12F1D8 */
    if (animate(m, obj, 0x0012F1D8u, 0x0012F13Cu))
        return 0;

    /* 0x12F154 */
    wr8(m, rd32(m, obj + 0x0Cu) + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x0012EF40u);
    return 0;
}
