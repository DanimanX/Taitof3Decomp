#include "obj_BC16C.h"
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


/* ROM 0xBC16C */
int Obj_BC16C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC16C: one palette step every 5 frames */
    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) - 1u));
    if ((int16_t)rd16(m, obj + 0x44u) < 0) {
        wr16(m, obj + 0x44u, 4u);

        /* 0x0BC17A */
        uint32_t d0 = (uint32_t)rd16(m, obj + 0x48u) << 2;
        uint32_t a1 = 0x00026840u + d0;
        uint32_t a2 = 0x00000F00u + d0;
        sub_001866(m, a1, a2, 0x0000000Cu, 0x00000001u);

        /* 0x0BC1AC */
        wr16(m, obj + 0x48u, (uint16_t)(rd16(m, obj + 0x48u) + 1u));
        if ((int16_t)rd16(m, obj + 0x48u) >= 0x15) {
            /* 0x0BC1C0: ramp finished */
            Obj_Free(m, obj);
            return 0;
        }
    }

    /* 0x0BC1BA */
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BC164 */
        wr32(m, obj, 0x000BC16Cu);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
