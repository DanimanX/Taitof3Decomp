#include "obj_14CC12.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x0017D6: per-colour 3-plane fade steps into A5-0x7F50 + d6. */
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

/* ROM 0x001792: first free 6-byte slot in A5-0x6650, arm a fade there. */
static void sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d0, uint32_t d1, uint32_t d2)
{
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
            sub_0017D6(m, a0, a1, d6, (int32_t)d1, (uint16_t)d2);
            return;
        }
        d6 += 0x0180u;
        a3 += 6u;
    }
}

/* ROM 0x001866 Pal_FadeFromBlack(dst, pal_idx, frames, count) */
static void pal_fade_001866(const f3mem_t *m, uint32_t dst, uint32_t pal_idx,
                            uint32_t frames, uint32_t count)
{
    sub_001792(m, 0x0000190Eu, dst, pal_idx, frames, count);
}

/* ROM 0x14CC12: opening fade 1 of 8, black -> 0x203C0 (palette 0x3F80,
 * 0x3E steps). Starts the fade, reloads +0x6 with 0x40 and re-arms the tick
 * vector to 0x14CC4C. The 0x40 reload is non-zero, so the following
 * `beq 0x14CC52` is never taken.
 *
 * NOTE: the captured tick shows the fade being built but no matching record
 * in the palette-ramp slot table, so this never verifies bit-exact (the
 * reference 0x14CDCC is only correct for its own capture). */
int Obj_14CC12_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x14CC12: fade 0x203C0 (palette 0x3F80, 0x3E steps, 4 frames) */
    /* 0x14CC12: movem.l {A0},-(SP); move.l #4,-(SP); move.l #0x3E,-(SP);
     * move.l #0x3F80,-(SP); move.l #0x203C0,-(SP); jsr 0x001866 */
    pal_fade_001866(m, 0x0203C0u, 0x003F80u, 0x3Eu, 0x04u);

    /* 0x14CC3C: move.w #0x40,(0x6,A0) ; 0x14CC42 beq.b (not taken) */
    wr16(m, obj + 0x06u, 0x0040u);

    /* 0x14CC44: move.l #0x14CC4C,(A0) */
    wr32(m, obj + 0x00u, 0x0014CC4Cu);
    return 0;
}
