/* ROM 0x0CEF62: allocate and clear a 512-byte slot buffer from A5+0x1D5E.
 * Uses the 4-bit occupancy mask at A5+0x1D5E to select a buffer pointer from
 * 0x0CEF98 and slot index from 0x0CEFD8, writes {0.w, idx.b, ptr.l} into
 * (+0x0C), marks the slot bit in A5+0x1D5E, and fills 128 longs with -1. */
#include "fn_auto.h"

int Fn_0CEF62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t mask = rd8(m, F3_A5 + 0x1D5Eu);            /* 0x0CEF62..0x0CEF64 */
    uint32_t a1 = rd32(m, 0x000CEF98u + (uint32_t)mask * 4u); /* 0x0CEF68 */
    uint16_t slot = rd16(m, 0x000CEFD8u + (uint32_t)mask * 2u); /* 0x0CEF70 */

    uint32_t a2 = rd32(m, obj + 0x0Cu);                 /* 0x0CEF78 */
    wr16(m, a2, 0u);                                    /* 0x0CEF7C */
    wr8(m, a2 + 2u, (uint8_t)slot);                     /* 0x0CEF7E */
    wr32(m, a2 + 4u, a1);                               /* 0x0CEF82 */

    wr8(m, F3_A5 + 0x1D5Eu,
        (uint8_t)(rd8(m, F3_A5 + 0x1D5Eu) | (1u << (slot & 7u)))); /* 0x0CEF86 */

    a2 = a1;                                            /* 0x0CEF8E */
    for (int i = 0; i < 0x80; i++) {                    /* 0x0CEF8A..0x0CEF92 */
        wr32(m, a2, 0xFFFFFFFFu);
        a2 += 4u;
    }

    r->d[0] = 0x0000FFFFu;
    r->d[1] = 0xFFFFFFFFu;
    r->a[1] = a1;
    return 0;
}
