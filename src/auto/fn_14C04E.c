#include "fn_auto.h"
/* ROM 0x14C04E: verified on 40 injected calls (capture_fn F3_INJECT, states Y27/Y04/Y14): only reached in stage phase 49 (script 0x14C63A at scroll 0x200000), which no run gets to: blend and queue 18 palettes across the gradient. */
#include "../video/palette.h"

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

/* ROM 0x16A286: set the second source and call the blend loop at 0x16A258. */
static void blend_palette(const f3mem_t *m, f3regs_t *r, uint32_t return_addr)
{
    wr32(m, r->a[7] - 4, return_addr);
    r->a[2] = 0x16A290;
    wr32(m, r->a[7] - 8, 0x16A28E);
    uint16_t weight = (uint16_t)r->d[0];
    uint16_t inverse = (uint16_t)~weight;
    r->d[1] = with_word(r->d[1], weight);
    r->d[0] = with_word(r->d[0], inverse);

    Pal_Blend64(m, r->a[0], r->a[1], r->a[2], weight);

    /* 0x16A260..0x16A280: the final iteration is byte zero. DIVU.W
     * packs the remainder above the quotient before LSR.W #8. */
    r->d[3] = ((uint32_t)rd8(m, r->a[2]) << 8) * weight;
    uint32_t sum = ((uint32_t)rd8(m, r->a[1]) << 8) * inverse + r->d[3];
    r->d[2] = ((sum % 0xFFFFu) << 16) | ((sum / 0xFFFFu) >> 8);
    r->d[7] = with_word(r->d[7], 0xFFFF);
}

int Fn_14C04E(const f3mem_t *m, f3regs_t *r)
{
    /* 0x14C04E..0x14C461 consists of 18 successive 0x3A-byte blocks.
     * The last two both use weight 0x8000. */
    for (uint32_t i = 0; i < 18; ++i) {
        uint32_t block = 0x14C04Eu + 0x3Au * i;
        uint32_t saved = r->a[7];
        for (unsigned j = 3; j > 0;)
            wr32(m, saved -= 4, r->a[--j]);
        for (unsigned j = 4; j > 0;)
            wr32(m, saved -= 4, r->d[--j]);
        r->a[7] = saved;

        uint32_t source = 0x232C0;
        uint32_t destination = 0x40B72Eu + 0x40u * i;
        wr32(m, saved - 4, source);
        wr32(m, saved - 8, destination);
        uint16_t weight = (uint16_t)(0x800u * (i < 16u ? i : 16u));
        r->d[0] = with_word(r->d[0], weight);
        r->a[0] = rd32(m, saved - 8);
        r->a[1] = rd32(m, saved - 4);
        blend_palette(m, r, block + 0x1E);

        FnAuto_QueuePalette(m, r, r->a[0], (uint16_t)(0x1D00u + 0x40u * i),
                            0x10, block + 0x32);
        for (unsigned j = 0; j < 4; ++j, saved += 4)
            r->d[j] = rd32(m, saved);
        for (unsigned j = 0; j < 3; ++j, saved += 4)
            r->a[j] = rd32(m, saved);
        r->a[7] = saved;
    }
    return 0;
}
