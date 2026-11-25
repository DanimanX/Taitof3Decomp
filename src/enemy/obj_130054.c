#include "obj_130054.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17D6: emit the colour ramp from the record at A0 to the record at A1,
 * over D2 frames and D1 brightness steps, into the ramp buffer at
 * (A5-0x7F50) + D1's slot offset. */
static void sub_17D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
                     uint32_t d1, uint32_t d2, uint32_t d6)
{
    uint32_t a2 = (F3_A5 - 0x7F50u) + d6;
    int32_t div = (int32_t)d1;
    uint16_t d7 = (uint16_t)((uint16_t)d2 - 1u);

    for (;;) {
        uint32_t c0 = rd32(m, a0);
        a0 += 4u;
        uint32_t c1 = rd32(m, a1);
        a1 += 4u;
        for (int k = 0; k < 3; k++) {
            unsigned sh = 16u - 8u * (unsigned)k;
            uint32_t s = ((c0 >> sh) & 0xFFu) << 8;
            wr16(m, a2 + 2u * (uint32_t)k, (uint16_t)s);
            uint32_t e = ((c1 >> sh) & 0xFFu) << 8;
            int32_t step = (int32_t)(e - s) / div;
            wr16(m, a2 + 6u + 2u * (uint32_t)k, (uint16_t)(uint32_t)step);
        }
        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x1792: claim the first free slot of the A5-0x6650 ramp registry. */
static uint32_t sub_1792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                         uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    uint16_t d7 = 0x000Fu;

    for (;;) {
        if (rd16(m, a3) == 0u) {
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return 0u;
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return 0u;
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_17D6(m, a0, a1, d1, d2, d6);
            return a3;
        }
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            return 0u;
    }
}

/* ROM 0x183C: A0 = sub_1792(...), returning the slot pointer */
static uint32_t sub_183C(const f3mem_t *m, uint32_t a0, uint32_t a1,
                         uint32_t d0, uint32_t d1, uint32_t d2)
{
    return sub_1792(m, a0, a1, d0, d1, d2);
}

/* ROM 0x1660: single frame of the 0x32-entry queue at A5-0x6750
 * ({count.w, src.l, dst.w}? -- here {dst.w, src.l, count.w}); the "prepare"
 * variant used by the third pass of each phase. */
static void sub_1660(const f3mem_t *m, uint32_t a0, uint32_t d0, uint32_t d1)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t a2 = (F3_A5 - 0x6750u) + (uint32_t)n * 8u;
    wr16(m, a2, (uint16_t)d1);
    wr32(m, a2 + 2u, a0);
    wr16(m, a2 + 6u, (uint16_t)d0);
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x130054: palette-cycle driver. Two independent counters (+0x14 and
 * +0x18) walk a pair of colour records in A1 and hand 0x28-step and 0x3C-step
 * ramps to the ramp registry every frame. */
int Obj_130054_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x130054: either hold flag frees the driver */
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x02u) != 0u)
        goto free_self;
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x04u) != 0u)
        goto free_self;

    {
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        uint32_t a2, a3;

        /* 0x13006C: phase A -- counters +0x14 / +0xC, ramp sources 0 / +0xC */
        if (rd16(m, obj + 0x14u) == 0u) {
            a2 = a1;
            a3 = a1 + 0x0Cu;
            sub_183C(m, a2, a3, 0x00005178u, 0x00000028u, 0x00000001u);
            sub_183C(m, a2, a3, 0x000050ECu, 0x00000028u, 0x00000004u);
            sub_183C(m, a2, a3, 0x0000506Cu, 0x00000028u, 0x00000004u);
        } else if (rd16(m, obj + 0x14u) == 0x0028u) {
            a2 = a1 + 0x0Cu;
            a3 = a1;
            sub_183C(m, a2, a3, 0x00005178u, 0x00000028u, 0x00000001u);
            sub_183C(m, a2, a3, 0x000050ECu, 0x00000028u, 0x00000004u);
            sub_183C(m, a2, a3, 0x0000506Cu, 0x00000028u, 0x00000004u);
        }

        /* 0x130152 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
        if ((int16_t)rd16(m, obj + 0x14u) >= 0x50)
            wr16(m, obj + 0x14u, 0u);

        /* 0x130164: phase B -- counters +0x18 / +0x10, sources +0x18 / +0x1C */
        if (rd16(m, obj + 0x18u) == 0u) {
            a2 = a1 + 0x18u;
            a3 = a1 + 0x1Cu;
            sub_183C(m, a2, a3, 0x0000517Cu, 0x0000003Cu, 0x00000001u);
            sub_183C(m, a2, a3, 0x000050F8u, 0x0000003Cu, 0x00000001u);
            sub_183C(m, a2, a3, 0x00005078u, 0x0000003Cu, 0x00000001u);
        } else if (rd16(m, obj + 0x18u) == 0x003Cu) {
            a2 = a1 + 0x20u;
            sub_1660(m, a2, 0x00000001u, 0x0000517Cu);
            sub_1660(m, a2, 0x00000001u, 0x000050F8u);
            sub_1660(m, a2, 0x00000001u, 0x00005078u);
        }

        /* 0x130232 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u));
        if ((int16_t)rd16(m, obj + 0x18u) >= 0x41)
            wr16(m, obj + 0x18u, 0u);
    }

    /* 0x130244: latch 0x13004C (re-arm) */
    wr32(m, obj + 0x00u, 0x00130054u);
    return 0;

free_self:
    /* 0x130248 */
    Obj_Free(m, obj);
    return 0;
}
