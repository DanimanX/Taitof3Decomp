#include "obj_17A420.h"
#include "../objects/obj_anim.h"

/* ROM 0x1794A4: D0w = random-ish value in [-(d0-1)..d0-1] from the shared LCG
 * at A5+0x4B3C (same generator as 0x1794F0, different use) */
static uint16_t sub_1794A4(const f3mem_t *m, uint16_t d0)
{
    uint16_t r = d0;
    if (d0 != 0u) {
        int off = 0;
        while (off < 32 && !((uint32_t)d0 & (0x80000000u >> off)))
            off++;
        int16_t d1w = (int16_t)(off - 16);
        uint16_t d2w = rd16(m, 0x179530u + (uint32_t)((int32_t)d1w * 2));
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & d2w);
        if ((int16_t)r < (int16_t)v)
            v = (uint16_t)(v - r);
        r = v;
        if (rd8(m, F3_A5 + 0x4B3Cu) & 1u)
            r = (uint16_t)(0u - r);
    }
    uint32_t s = rd32(m, F3_A5 + 0x4B3Cu) * 5u - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (s << 16) | (s >> 16));
    return r;
}

/* ROM 0x17A420: spawn-wait tail with a global gate. When the +0x6 timer runs
 * out it advances the shared 0x4B24 counter (giving up if it is already above
 * 0x1E), starts animation index 0, installs the 0x17A504 behaviour record,
 * mirrors the parent's position/angle and scatters +0x20 by 0x1794A4. */
int Obj_17A420_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        /* 0x17A424 bgt.b -> 0x17A418, back into the wait body */
        wr32(m, obj + 0x00u, 0x0017A420u);
        return 0;
    }

    /* 0x17A426 bra.w -> 0x17A44A */
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        /* 0x17A450 bgt.w -> 0x179D18, outside this entry */
        return -1;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));

    /* The capture keeps +0x3A/+0x3C and +0x40 at their original values, so the
     * 0x17A42A animation block is not entered on this path: only the global
     * counter and the behaviour record change. */
    {
        uint16_t idx = rd16(m, obj + 0x40u);
        if (idx != 0u)
            return -1; /* any other index would run 0x17A42A, untested here */
    }

    wr32(m, obj + 0x44u, 0x0017A504u);         /* 0x17A458 */
    wr16(m, obj + 0x48u, rd16(m, 0x0017A508u));
    wr16(m, obj + 0x4Au, 0x0000u);

    /* 0x17A46C: mirror the parent, then rotate away from it */
    {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x50u, rd16(m, parent + 0x50u));
        wr16(m, obj + 0x54u, rd16(m, parent + 0x54u));
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, parent + 0x20u) - 0x80u));
    }
    wr16(m, obj + 0x1Cu, 0x0000u);
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)sub_1794A4(m, 0x0Au)));

    wr16(m, obj + 0x42u, 0x0002u);             /* 0x17A49E */
    /* 0x17A4A4 bra.w 0x179C66: only installs 0x179C6E and returns */
    wr32(m, obj + 0x00u, 0x00179C6Eu);
    return 0;
}
