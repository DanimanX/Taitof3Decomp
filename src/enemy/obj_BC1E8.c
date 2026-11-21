#include "obj_BC1E8.h"
#include "../objects/obj_pool.h"

/* ROM 0x0017D6: per-colour fade steps into the slot at A5-0x7F50 + d6 */
static void sub_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d6_slot, int32_t steps, uint16_t count)
{
    uint32_t a2 = F3_A5 - 0x7F50u + d6_slot;
    int16_t d7 = (int16_t)(count - 1u);
    do {
        uint32_t c0 = rd32(m, a0); a0 += 4u;
        uint32_t c1 = rd32(m, a1); a1 += 4u;
        static const unsigned sh[3] = { 16u, 8u, 0u };
        for (int k = 0; k < 3; k++) {
            int32_t d2 = (int32_t)(((c0 >> sh[k]) & 0xFFu) << 8);
            wr16(m, a2 + (uint32_t)k * 2u, (uint16_t)d2);
            int32_t d3 = (int32_t)(((c1 >> sh[k]) & 0xFFu) << 8);
            d3 = (d3 - d2) / steps;
            wr16(m, a2 + 6u + (uint32_t)k * 2u, (uint16_t)d3);
        }
        a2 += 0x0Cu;
    } while (--d7 != -1);
}

/* ROM 0x001866 Pal_FadeFromBlack -> 0x001792 (free slot in A5-0x6650 x16) */
static void pal_fade_001866(const f3mem_t *m, uint32_t target, uint32_t d0,
                            uint32_t d1, uint32_t d2)
{
    uint32_t a0 = 0x0000190Eu; /* lea (0x8E,PC) */
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    for (int i = 0; i < 16; i++) {
        if (rd16(m, a3) == 0u) {
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return;
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return;
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_0017D6(m, a0, target, d6, (int32_t)d1, (uint16_t)d2);
            return;
        }
        d6 += 0x180u;
        a3 += 6u;
    }
}

/* ROM 0x0BC1E8 */
int Obj_BC1E8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x44u) - 1u);
    wr16(m, obj + 0x44u, t);
    if ((int16_t)t < 0) {
        wr16(m, obj + 0x44u, 4u);
        uint32_t d0 = (uint32_t)rd16(m, obj + 0x48u) << 2;
        pal_fade_001866(m, 0x00026C40u + d0, 0x00001300u + d0, 0x0Au, 1u);
        uint16_t n = (uint16_t)(rd16(m, obj + 0x48u) + 1u);
        wr16(m, obj + 0x48u, n);
        if ((int16_t)n >= 0x20) {
            Obj_Free(m, obj);
            return 0;
        }
    }
    /* 0x0BC236 */
    t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BC1E8u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
