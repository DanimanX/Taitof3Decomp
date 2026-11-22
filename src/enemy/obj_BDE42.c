#include "obj_BDE42.h"

/* ROM 0x0053B2: run-length tilemap blit into playfield RAM.
 * src = {cols.b, rows.b, stream...}; stream words: 0 -> next long is a tile;
 * 0x01nn -> nn+1 empty cells; 0x02nn tile -> nn+1 copies; 0x03nn tile ->
 * nn+1 cells, tile+1 each; 0x04nn tile -> tile-1 each. Tiles ^= (mode & 0xFE00)<<16.
 * mode bits 15-14 pick the scan direction (table 0x5352). The destination is a
 * 32x32-long page: inner offset wraps in 0x7C, column offset in 0xF80.
 * dry = 1 only validates the stream (returns -1 on an unknown op, no writes). */
static int sub_0053B2(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t modew, int dry)
{
    uint32_t a0 = src;
    uint16_t d7hi = (uint16_t)(rd8(m, a0) - 1u);
    uint16_t d7lo = (uint16_t)(rd8(m, a0 + 1u) - 1u);
    a0 += 2u;
    uint32_t d7 = ((uint32_t)d7hi << 16) | d7lo;
    uint16_t dst_lo = (uint16_t)dst;
    unsigned mode = (unsigned)(modew >> 14) & 3u;
    uint16_t d5hi = (mode < 2u) ? 0x0080u : 0xFF80u;
    uint16_t d5w = dst_lo;
    if (mode >= 2u)
        d5w = (uint16_t)(d5w + (uint16_t)((d7 >> 9) & 0xFFFu));
    uint16_t d6hi = dst_lo;
    if (mode & 1u)
        d6hi = (uint16_t)(d6hi + (uint16_t)(d7lo << 2));
    uint16_t d6lo = (mode & 1u) ? 0xFFFCu : 0x0004u;
    uint32_t d3 = ((uint32_t)modew & 0xFE00u) << 16;
    uint32_t a1 = dst;

    enum { FETCH, CLR, FILL, INC, DEC } st = FETCH;
    uint16_t d0 = 0u;
    uint32_t d2 = 0u;

    uint16_t outer = d7hi;
    for (;;) {
        uint16_t d4w = d6hi;
        d5w &= 0x0F80u;
        a1 = (a1 & 0xFFFFF000u) + (uint32_t)(int32_t)(int16_t)d5w;
        uint16_t inner = d7lo;
        for (;;) {
            d4w &= 0x007Cu;
            uint32_t cell = a1 + (uint32_t)(int32_t)(int16_t)d4w;
            if (st == FETCH) {
                uint16_t w = rd16(m, a0); a0 += 2u;
                if (w == 0u) {
                    d2 = rd32(m, a0) ^ d3; a0 += 4u;
                    if (!dry) wr32(m, cell, d2);
                    goto next;
                }
                d0 = (uint16_t)(w & 0xFFu);
                switch (w >> 8) {
                case 1: st = CLR; break;
                case 2: st = FILL; d2 = rd32(m, a0) ^ d3; a0 += 4u; break;
                case 3: st = INC;  d2 = rd32(m, a0) ^ d3; a0 += 4u; break;
                case 4: st = DEC;  d2 = rd32(m, a0) ^ d3; a0 += 4u; break;
                default: return -1; /* jump table 0x5438 index out of range */
                }
            }
            if (!dry) wr32(m, cell, st == CLR ? 0u : d2);
            if (st == INC) d2 = (d2 & 0xFFFF0000u) | (uint16_t)(d2 + 1u);
            if (st == DEC) d2 = (d2 & 0xFFFF0000u) | (uint16_t)(d2 - 1u);
            if (d0-- == 0u) st = FETCH;
        next:
            d4w = (uint16_t)(d4w + d6lo);
            if (inner-- == 0u) break;
        }
        d5w = (uint16_t)(d5w + d5hi);
        if (outer-- == 0u) break;
    }
    return 0;
}

/* Validate then blit (no side effects if the stream has an unknown op). */
static int blit_0053B2(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t modew)
{
    if (sub_0053B2(m, src, dst, modew, 1) < 0)
        return -1;
    return sub_0053B2(m, src, dst, modew, 0);
}

/* ROM 0x0BDE42 */
int Obj_BDE42_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x000BDE42u);
        return 0;
    }
    if (blit_0053B2(m, 0x00090A2Au, 0x006133CCu, 0x0000u) < 0)
        return -1;
    if (blit_0053B2(m, 0x00090A2Au, 0x0061338Cu, 0x0000u) < 0)
        return -1;
    wr16(m, obj + 0x60u, 0x001Fu);
    wr32(m, obj + 0x00u, 0x000BDE92u);
    return 0;
}
