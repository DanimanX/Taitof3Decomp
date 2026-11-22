#include "obj_118D7C.h"
#include "../objects/obj_pool.h"

/* ROM 0x001774: clear all 16 palette-ramp slots at A5-0x6650 */
static void sub_001774(const f3mem_t *m)
{
    uint32_t a0 = F3_A5 - 0x6650u;
    for (int i = 0; i < 16; i++) {
        wr16(m, a0, 0u);
        a0 += 6u;
    }
    wr16(m, F3_A5 - 0x5DE8u, 0u);
}

/* ROM 0x0017D6: build per-entry {start.w, delta.w} ramp records */
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

        uint32_t s = ((c0 >> 16) & 0xFFu) << 8;
        wr16(m, a2 + 0x00u, (uint16_t)s);
        uint32_t e = ((c1 >> 16) & 0xFFu) << 8;
        wr16(m, a2 + 0x06u, (uint16_t)(uint32_t)((int32_t)(e - s) / div));

        s = ((c0 >> 8) & 0xFFu) << 8;
        wr16(m, a2 + 0x02u, (uint16_t)s);
        e = ((c1 >> 8) & 0xFFu) << 8;
        wr16(m, a2 + 0x08u, (uint16_t)(uint32_t)((int32_t)(e - s) / div));

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
            sub_0017D6(m, a0, a1, d1, d2, d6);
            return;
        }
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
    }
}

/* Dispatch one of the 6 palette-fade wrappers in 0x118E0C..0x118E72 */
static int call_fade_wrapper(const f3mem_t *m, uint32_t fn,
                             uint32_t d0, uint32_t d1, uint32_t d2,
                             uint32_t d3, uint32_t d4)
{
    switch (fn) {
    case 0x00118E0Cu: /* 0x001866: from 0x190E to D0 */
        sub_001792(m, 0x0000190Eu, d0, d1, d2, d3);
        return 0;
    case 0x00118E20u: /* 0x0018E4: from D0 to 0x198E */
        sub_001792(m, d0, 0x0000198Eu, d1, d2, d3);
        return 0;
    case 0x00118E34u: /* 0x0018BA: from 0x198E to D0 */
        sub_001792(m, 0x0000198Eu, d0, d1, d2, d3);
        return 0;
    case 0x00118E48u: /* 0x001890: from D0 to 0x190E */
        sub_001792(m, d0, 0x0000190Eu, d1, d2, d3);
        return 0;
    case 0x00118E5Cu: /* 0x00183C: from D0 to D4 */
        sub_001792(m, d0, d4, d1, d2, d3);
        return 0;
    case 0x00118E72u: /* 0x00183C: from D4 to D0 */
        sub_001792(m, d4, d0, d1, d2, d3);
        return 0;
    default:
        return -1;
    }
}

/* ROM 0x118D7C */
int Obj_118D7C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, F3_A5 - 0x144Bu);
    if ((flags & 0x10u) != 0u && (flags & 0x08u) != 0u) {
        sub_001774(m);
        Obj_Free(m, obj);
        return 0;
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    if ((int16_t)cnt > 0) {
        wr16(m, obj + 0x06u, cnt);
        wr32(m, obj + 0x00u, 0x00118D7Cu);
        return 0;
    }

    uint32_t fn = rd32(m, rd32(m, obj + 0x3Au));
    if (fn != 0x00118E0Cu && fn != 0x00118E20u && fn != 0x00118E34u &&
        fn != 0x00118E48u && fn != 0x00118E5Cu && fn != 0x00118E72u) {
        return -1;
    }

    wr16(m, obj + 0x06u, cnt);
    wr16(m, obj + 0x3Eu, 0u);

    /* 0x118D9A */
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint32_t d0 = rd32(m, a1); a1 += 4u;
    uint32_t d1 = (uint32_t)(int32_t)(int16_t)rd16(m, a1); a1 += 2u;
    uint32_t d2 = (uint32_t)rd16(m, a1) << 2; a1 += 2u;
    d0 += d2;
    d1 += d2;
    d2 = (uint32_t)rd16(m, a1); a1 += 2u;
    uint32_t d3 = (uint32_t)rd16(m, a1); a1 += 2u;
    uint32_t d4 = rd32(m, a1);

    if (call_fade_wrapper(m, fn, d0, d1, d2, d3, d4) < 0)
        return -1;

    wr16(m, obj + 0x3Eu, (uint16_t)((rd16(m, obj + 0x3Eu) + 1u) & 3u));
    wr16(m, obj + 0x06u, (uint16_t)d2);
    wr32(m, obj + 0x00u, 0x00118DDCu);
    return 0;
}
