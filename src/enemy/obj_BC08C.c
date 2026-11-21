#include "obj_BC08C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0017D6: build per-entry {start.w, delta.w} ramp records (A2 += 0x0C
 * each), one per long word of the A0/A1 colour lists */
static void sub_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
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

        /* 0x0017E6: +0x06 = blue step, +0x00 = blue start */
        uint32_t s = ((c0 >> 16) & 0xFFu) << 8;
        wr16(m, a2 + 0x00u, (uint16_t)s);
        uint32_t e = ((c1 >> 16) & 0xFFu) << 8;
        wr16(m, a2 + 0x06u, (uint16_t)(uint32_t)((int32_t)(e - s) / div));

        /* 0x0017FE: green */
        s = ((c0 >> 8) & 0xFFu) << 8;
        wr16(m, a2 + 0x02u, (uint16_t)s);
        e = ((c1 >> 8) & 0xFFu) << 8;
        wr16(m, a2 + 0x08u, (uint16_t)(uint32_t)((int32_t)(e - s) / div));

        /* 0x001818: red */
        s = (c0 & 0xFFu) << 8;
        wr16(m, a2 + 0x04u, (uint16_t)s);
        e = (c1 & 0xFFu) << 8;
        wr16(m, a2 + 0x0Au, (uint16_t)(uint32_t)((int32_t)(e - s) / div));

        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x001792: claim one of 16 palette-ramp slots and build the ramp */
static uint32_t sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                           uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    uint16_t d7 = 0x000Fu;

    for (;;) {
        if (rd16(m, a3) == 0u) {
            /* 0x0017A6 */
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return 0xFFFFFFFFu;
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return 0xFFFFFFFFu;
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_0017D6(m, a0, a1, d1, d2, d6);
            return a3;
        }
        /* 0x0017BC */
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
    return 0xFFFFFFFFu;   /* 0x0017C8 */
}

/* ROM 0x001866 Pal_FadeFromBlack(dst, pal_idx, frames, count) */
static void sub_001866(const f3mem_t *m, uint32_t dst, uint32_t pal_idx,
                       uint32_t frames, uint32_t count)
{
    (void)sub_001792(m, 0x0000190Eu, dst, pal_idx, frames, count);
}

/* ROM 0x001890 Pal_FadeToBlack(src, pal_idx, frames, count) */
static void sub_001890(const f3mem_t *m, uint32_t src, uint32_t pal_idx,
                       uint32_t frames, uint32_t count)
{
    (void)sub_001792(m, src, 0x0000190Eu, pal_idx, frames, count);
}

/* ROM 0xBC08C */
int Obj_BC08C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC08C: palette fade cycle masked to 0x1F */
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x60u) & 0x1Fu);

    if (d0w == 0x001Fu) {
        /* 0x0BC0A0 */
        sub_001866(m, 0x0002678Cu, 0x00000E4Cu, 0x00000007u, 0x00000001u);
        /* 0x0BC0C2 */
        sub_001866(m, 0x0002684Cu, 0x00000F0Cu, 0x0000000Fu, 0x00000001u);
    }
    if (d0w == 0x0017u) {
        /* 0x0BC0EC */
        sub_001890(m, 0x0002678Cu, 0x00000E4Cu, 0x00000017u, 0x00000001u);
    }
    if (d0w == 0x000Fu) {
        /* 0x0BC116 */
        sub_001890(m, 0x0002684Cu, 0x00000F0Cu, 0x0000000Fu, 0x00000001u);
    }

    /* 0x0BC13C */
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BC084 */
        wr32(m, obj, 0x000BC08Cu);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
