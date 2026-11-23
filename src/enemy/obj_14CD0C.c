#include "obj_14CD0C.h"
#include "../objects/obj_pool.h"

/* ROM 0x0017D6: per-colour fade steps into the slot at A5-0x7F50 + d6
 * (same helper shape as obj_BC1E8 / obj_C6866). */
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

/* ROM 0x001792: take the first free 6-byte slot in A5-0x6650 and arm a fade
 * there (frames at +2, step count at +4, palette index at +0) via 0x0017D6. */
static uint32_t sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                           uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    for (int i = 0; i < 16; i++) {
        if (rd16(m, a3) == 0u) {
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return 0xFFFFFFFFu;
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return 0xFFFFFFFFu;
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_0017D6(m, a0, a1, d6, (int32_t)d1, (uint16_t)d2);
            return a3;
        }
        d6 += 0x0180u;
        a3 += 6u;
    }
    return 0xFFFFFFFFu;
}

/* ROM 0x001866 Pal_FadeFromBlack(dst, pal_idx, frames, count) */
static void pal_fade_001866(const f3mem_t *m, uint32_t dst, uint32_t pal_idx,
                            uint32_t frames, uint32_t count)
{
    (void)sub_001792(m, 0x0000190Eu, dst, pal_idx, frames, count);
}

/* ROM 0x14CD0C: opening fade 1 of 3, black -> 0x20400 (palette 0x3FC0,
 * 0x3E steps). Countdown +0x6; when it just expires (subq #1 gives 0, so
 * the bgt falls through) the fade is started and the counter reloads 0x40.
 * 0x40 is non-zero, so the following `beq 0x14CD52` is never taken and the
 * vector is always re-armed to 0x14CD44. Every captured tick is the plain
 * decrement. */
int Obj_14CD0C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0014CD0Cu);              /* 0x14CD04 */
        return 0;
    }
    pal_fade_001866(m, 0x00020400u, 0x00003FC0u, 0x3Eu, 1u); /* 0x14CD12 */
    wr16(m, obj + 0x06u, 0x0040u);                      /* 0x14CD3C */
    wr32(m, obj + 0x00u, 0x0014CD44u);                  /* 0x14CD44 */
    return 0;
}
