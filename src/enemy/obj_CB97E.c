#include "obj_CB97E.h"
#include "../objects/obj_common.h"

/* ROM 0x0055C8: copy a {cols.w, rows.w, tile.w...} word tilemap to dst,
 * tiles ^= mode << 8; mode bits pick the scan direction (table 0x5616). */
static void sub_0055C8(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t mode)
{
    uint16_t d0 = rd16(m, a0); a0 += 2u;
    uint16_t d1 = rd16(m, a0); a0 += 2u;
    uint16_t d5 = (uint16_t)(mode << 8);
    uint16_t d4 = (uint16_t)(mode << 1);
    d4 = (uint16_t)((d4 & 0xFF00u) | (uint8_t)(d4 << 6));
    d4 = (uint16_t)(d4 >> 5);
    uint32_t fn = rd32(m, 0x00005616u + (uint32_t)(int32_t)(int16_t)d4);
    uint16_t d2, d3;
    switch (fn) {
    case 0x5626u: d2 = 0x0002u; d3 = 0x0080u; break;
    case 0x5630u: d2 = 0xFFFEu; d3 = 0x0080u;
        a1 += (uint32_t)(int32_t)(int16_t)d1 - 2u; break;
    case 0x563Eu: d2 = 0x0002u; d3 = 0xFF80u;
        a1 += (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)(d0 - 1u) << 7); break;
    case 0x5650u: d2 = 0xFFFEu; d3 = 0xFF80u;
        a1 += (uint32_t)(int32_t)(int16_t)d1 - 2u;
        a1 += (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)(d0 - 1u) << 7); break;
    default: return; /* mode with bits beyond the 4-entry table: not used here */
    }
    do {
        uint32_t a2 = a1;
        uint16_t n = d1;
        do {
            wr16(m, a1, (uint16_t)(rd16(m, a0) ^ d5)); a0 += 2u;
            a1 += (uint32_t)(int32_t)(int16_t)d2;
            n = (uint16_t)(n - 1u);
        } while ((int16_t)n > 0);
        a1 = a2 + (uint32_t)(int32_t)(int16_t)d3;
        d0 = (uint16_t)(d0 - 1u);
    } while ((int16_t)d0 > 0);
}

/* ROM 0x005A86: copy character tiles into char RAM 0x61E000 + D0*0x20.
 * a0 -> {count-1.w, tile.w...}; each tile = 8 longs from 0x1120A + tile*0x20,
 * stored with the two words swapped. */
static void sub_005A86(const f3mem_t *m, uint16_t d0w, uint32_t a0)
{
    uint32_t a1 = 0x0061E000u + (uint32_t)(int32_t)(int16_t)(uint16_t)(d0w << 5);
    uint32_t a2 = 0x0001120Au;
    uint16_t n = rd16(m, a0); a0 += 2u;
    for (;;) {
        uint16_t d1w = (uint16_t)(rd16(m, a0) << 5); a0 += 2u;
        uint32_t a3 = a2 + (uint32_t)(int32_t)(int16_t)d1w;
        for (int i = 0; i < 8; i++, a3 += 4u, a1 += 4u) {
            uint32_t d2 = rd32(m, a3);
            wr32(m, a1, (d2 << 16) | (d2 >> 16));
        }
        if (n-- == 0u)
            break;
    }
}

/* ROM 0x0CB97E */
int Obj_CB97E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    uint32_t a3 = 0x0061C000u;
    a3 += (uint32_t)(int32_t)(int16_t)rd16(m, a2 + (uint32_t)((int32_t)(int16_t)rd16(m, a1 + 0x22u) * 2));
    a3 += (uint32_t)((int32_t)(int16_t)rd16(m, a1 + 0x60u) * 4);
    wr32(m, obj + 0x0Cu, a3);
    sub_0055C8(m, 0x00010B74u, a3, 0u);
    wr16(m, obj + 0x3Au, (uint16_t)(int16_t)-0x7E);
    wr16(m, obj + 0x3Eu, 0u);
    /* 0x0CB9C0 */
    Obj_Animate(m, obj, 0x000CBACCu);
    sub_005A86(m, (uint16_t)rd8(m, 0x00010B79u), rd32(m, obj + 0x10u));
    wr32(m, obj + 0x00u, 0x000CB9EEu);
    return 0;
}
