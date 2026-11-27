#include "fn_auto.h"
#include "../video/palette.h"

/* ROM 0x14A9C0: blend and queue sixteen successive palette bands. */
static uint32_t f14a9c0_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

/* ROM 0x16A286: set blend source 0x16A290 and run the 0x16A258 loop. */
static void f14a9c0_blend(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    wr32(m, r->a[7] - 4u, ret);
    r->a[2] = 0x16A290u;
    wr32(m, r->a[7] - 8u, 0x16A28Eu);
    uint16_t weight = (uint16_t)r->d[0];
    uint16_t inverse = (uint16_t)~weight;
    r->d[1] = f14a9c0_word(r->d[1], weight);
    r->d[0] = f14a9c0_word(r->d[0], inverse);
    Pal_Blend64(m, r->a[0], r->a[1], r->a[2], weight);
    r->d[3] = ((uint32_t)rd8(m, r->a[2]) << 8) * weight;
    uint32_t sum = ((uint32_t)rd8(m, r->a[1]) << 8) * inverse + r->d[3];
    r->d[2] = ((sum % 0xFFFFu) << 16) | ((sum / 0xFFFFu) >> 8);
    r->d[7] = f14a9c0_word(r->d[7], 0xFFFFu);
}

int Fn_14A9C0(const f3mem_t *m, f3regs_t *r)
{
    for (uint32_t i = 0; i < 16u; ++i) {
        uint32_t block = 0x14A9C0u + 0x3Au * i;
        uint32_t sp = r->a[7];
        for (unsigned j = 3; j > 0;)
            wr32(m, sp -= 4u, r->a[--j]);
        for (unsigned j = 4; j > 0;)
            wr32(m, sp -= 4u, r->d[--j]);
        r->a[7] = sp;

        /* ROM pushes 0x234C0, then this band's destination, then pops A0/A1. */
        wr32(m, sp - 4u, 0x234C0u);
        wr32(m, sp - 8u, 0x40B3AEu + 0x40u * i);
        r->a[0] = rd32(m, sp - 8u);
        r->a[1] = rd32(m, sp - 4u);
        uint16_t weight = (uint16_t)(0xBA00u - 0x0600u * i);
        r->d[0] = f14a9c0_word(r->d[0], weight);
        f14a9c0_blend(m, r, block + 0x1Eu);

        FnAuto_QueuePalette(m, r, r->a[0],
                            (uint16_t)(0x1C00u + 0x40u * i),
                            0x10u, block + 0x32u);
        for (unsigned j = 0; j < 4; ++j, sp += 4u)
            r->d[j] = rd32(m, sp);
        for (unsigned j = 0; j < 3; ++j, sp += 4u)
            r->a[j] = rd32(m, sp);
        r->a[7] = sp;
    }
    return 0;
}
