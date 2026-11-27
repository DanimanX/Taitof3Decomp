#include "fn_auto.h"
/* ROM 0x14B1BE: UNVERIFIED (no capture): Wait for the countdown, reset layer scroll, draw the background, and queue a palette fade. */
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x14B200..0x14B2CC, eight copies of the same stack call to 0x0053B2. */
static void draw_background(const f3mem_t *m, const f3regs_t *r, uint32_t sp)
{
    for (unsigned block = 0; block < 8; ++block) {
        uint32_t src = (block & 2u) != 0 ? 0x07677Cu : 0x0766B4u;
        uint32_t dst = 0x611000u + (block & 1u) * 0x400u + (block >> 1) * 0x20u;
        wr16(m, sp - 2u, 0u);
        wr32(m, sp - 6u, dst);
        wr32(m, sp - 10u, src);
        uint32_t save = sp - 14u;
        wr32(m, save, 0x14B216u + block * 0x1Au);
        for (unsigned i = 3; i > 0;)
            wr32(m, save -= 4u, r->a[--i]);
        for (unsigned i = 8; i > 0;)
            wr32(m, save -= 4u, r->d[--i]);
        /* 0x0053DC calls the table's first entry, 0x005352 + 8. */
        wr32(m, save - 4u, 0x0053DEu);
        Gfx_DrawCompressedTilemap(m, src, dst, rd32(m, sp - 2u));
    }
}

/* ROM 0x14B2D0 calls 0x001890, whose target palette is at 0x00190E. */
static void fade_to_black(const f3mem_t *m, f3regs_t *r, uint32_t sp)
{
    wr32(m, sp - 4u, 0x20u);
    wr32(m, sp - 8u, 0x180u);
    wr32(m, sp - 12u, 0x1400u);
    wr32(m, sp - 16u, 0x0230C0u);
    wr32(m, sp - 20u, 0x14B2EEu);
    wr32(m, sp - 24u, r->a[6]); /* LINK and MOVEM at 0x001890. */
    wr32(m, sp - 28u, r->a[1]);
    wr32(m, sp - 32u, r->a[0]);
    wr32(m, sp - 36u, r->d[2]);
    wr32(m, sp - 40u, r->d[1]);
    wr32(m, sp - 44u, 0x0018B0u);
    wr32(m, sp - 48u, r->a[3]); /* 0x001796 MOVEM */
    wr32(m, sp - 52u, r->a[2]);
    wr32(m, sp - 56u, 0x00190Eu);
    for (unsigned i = 7; i > 2; --i)
        wr32(m, sp - 60u - 4u * (7u - i), r->d[i]);
    wr32(m, sp - 80u, 0x20u);
    wr32(m, sp - 84u, 0x180u);
    wr32(m, sp - 88u, 0x1400u);

    uint32_t slot = Pal_QueueRamp(m, 0x0230C0u, 0x00190Eu, 0x1400u, 0x180u, 0x20u);
    if (slot != 0)
        wr32(m, sp - 92u, 0x0017B8u);

    /* 0x0017C8 clears A0 and subtracts one when no slot was found. */
    r->d[0] = slot != 0 ? slot : UINT32_MAX;
}

int Fn_14B1BE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t remaining = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(remaining - 1u));
    if ((int16_t)remaining > 1) {
        wr32(m, obj, 0x14B1BEu); /* 0x14B1C2 -> 0x14B1B6 */
        return 0;
    }

    uint32_t sp = r->a[7] - 4u;
    wr32(m, sp, obj); /* 0x14B1C4 MOVEM */
    r->a[0] = r->a[5] + 0x420Cu;
    r->a[1] = r->a[5] - 0x5DA0u;
    wr32(m, sp - 4u, 0u);
    wr32(m, sp - 8u, 0u);
    r->d[0] = 0u;
    r->d[1] = 0u;
    wr32(m, sp - 4u, 0x14B1E6u);
    Mem_WritePair64(m, r->a[0], r->a[1], r->d[0], r->d[1]);
    r->a[1] += 4u;

    r->a[0] = r->a[5] + 0x422Cu;
    wr32(m, sp - 4u, 0xFFF80000u);
    wr32(m, sp - 8u, 0u);
    r->d[1] = 0xFFF80000u;
    wr32(m, sp - 4u, 0x14B200u);
    Obj_16AB26(m, r->a[0], r->d[0], r->d[1]);
    r->a[0] += 4u;

    draw_background(m, r, sp);
    fade_to_black(m, r, sp);
    r->a[0] = rd32(m, sp);
    wr32(m, r->a[0], 0x14B2FEu);
    return 0;
}
