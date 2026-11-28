#include "fn_auto.h"
/* ROM 0x0C2E80: clear the 0xE8-word line-RAM band at 0x62B000 with 0x300C
 * (via 0x176198), blit the compressed tilemap at 0x0066F9C to 0x61039C
 * (via 0x0053B2, flags 0), then set A0+0x06 = 0x59 and re-arm 0x0C2EC6.
 * Every register survives: A0 is pushed around the calls. */
#include "../video/sprite.h"

/* ROM 0x176198: fill the 0xE8 words (0x1D0 bytes) at base + (A5-0x5D56) with
 * val. The ROM writes them backwards with movem.l bursts; every byte of the
 * range gets the same value, so the order does not matter. */
static void fill_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)
        wr16(m, a0 + off, val);                            /* ROM 0x1761C4..0x1761FC */
}

int Fn_0C2E80(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7] - 4u;                            /* 0x0C2E80 movem.l {A0},-(SP) */
    wr32(m, sp, r->a[0]);

    fill_176198(m, 0x0062B000u, 0x300Cu);                  /* 0x0C2E84..0x0C2E94 */
    Gfx_DrawCompressedTilemap(m, 0x00066F9Cu, 0x0061039Cu, 0u); /* 0x0C2E98..0x0C2EAE */

    r->a[0] = rd32(m, sp);                                 /* 0x0C2EB2 movem.l (SP)+,{A0} */
    r->a[7] = sp + 4u;
    wr16(m, obj + 0x06u, 0x0059u);                         /* 0x0C2EB6 (beq 0x0C2ECC never taken) */
    wr32(m, obj, 0x000C2EC6u);                             /* 0x0C2EBE */
    return 0;                                              /* 0x0C2EC4 rts */
}
