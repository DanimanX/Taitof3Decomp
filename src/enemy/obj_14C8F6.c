#include "obj_14C8F6.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x00523A: tilemap decompressor, the 0x0053B2 stream format but each
 * tile long is (v & 0xFE00FFFF) ^ ((attr & 0x1FF) << 16). `flags` is the
 * word at SP+0x3A (top 2 bits = mirror mode), `attr` the word at SP+0x38.
 * Shared with obj_13E150.c (declared there; no header is editable here). */
void Gfx_DrawTilemapAttr_00523A(const f3mem_t *m, uint32_t src, uint32_t dst,
                                uint16_t attr, uint16_t flags);
void Gfx_DrawTilemapAttr_00523A(const f3mem_t *m, uint32_t src, uint32_t dst,
                                uint16_t attr, uint16_t flags)
{
    uint32_t a0 = src;
    int16_t rows = (int16_t)((uint16_t)rd8(m, a0++) - 1u);
    int16_t cols = (int16_t)((uint16_t)rd8(m, a0++) - 1u);
    uint32_t mode = (uint32_t)(flags >> 14) & 3u;
    uint16_t d5_lo = (uint16_t)(dst & 0xFFFFu);
    uint16_t d6_lo = d5_lo;
    int16_t row_step = 0x0080;
    int16_t col_step = 4;
    if (mode == 1u) {
        d6_lo = (uint16_t)(d6_lo + (uint16_t)(cols << 2));
        col_step = -4;
    } else if (mode == 2u) {
        row_step = -0x0080;
        d5_lo = (uint16_t)(d5_lo + (uint16_t)(rows << 7));
    } else if (mode == 3u) {
        row_step = -0x0080;
        d5_lo = (uint16_t)(d5_lo + (uint16_t)(rows << 7));
        d6_lo = (uint16_t)(d6_lo + (uint16_t)(cols << 2));
        col_step = -4;
    }
    /* D3 = (flags word : 0) with bits 24..16 replaced by attr (bfins {7:9}) */
    uint32_t d3 = ((uint32_t)(flags & 0xFE00u) << 16) | ((uint32_t)(attr & 0x1FFu) << 16);
    int16_t run_cnt = 0;
    uint8_t run_op = 0;
    uint32_t run_val = 0;
    int16_t r = rows;
    do {
        uint16_t d4 = d6_lo;
        uint32_t row_base = (dst & 0xFFFFF000u) + (d5_lo & 0x0F80u);
        int16_t c = cols;
        do {
            uint16_t off = d4 & 0x007Cu;
            if (run_cnt == 0) {
                uint16_t tok = rd16(m, a0);
                a0 += 2u;
                if (tok == 0u) {
                    wr32(m, row_base + off, (rd32(m, a0) & 0xFE00FFFFu) ^ d3);
                    a0 += 4u;
                } else {
                    run_op = (uint8_t)(tok >> 8);
                    run_cnt = (int16_t)(tok & 0xFFu);
                    if (run_op == 1u) {
                        wr32(m, row_base + off, 0u);
                    } else {
                        run_val = (rd32(m, a0) & 0xFE00FFFFu) ^ d3;
                        a0 += 4u;
                        wr32(m, row_base + off, run_val);
                        if (run_op == 3u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val + 1u);
                        else if (run_op == 4u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val - 1u);
                    }
                }
            } else {
                run_cnt--;
                if (run_op == 1u) {
                    wr32(m, row_base + off, 0u);
                } else {
                    wr32(m, row_base + off, run_val);
                    if (run_op == 3u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val + 1u);
                    else if (run_op == 4u) run_val = (run_val & 0xFFFF0000u) | (uint16_t)(run_val - 1u);
                }
            }
            d4 = (uint16_t)(d4 + col_step);
            c--;
        } while (c >= 0);
        d5_lo = (uint16_t)(d5_lo + row_step);
        r--;
    } while (r >= 0);
}

/* ROM 0x14C8F6: init. Unpacks two tilemaps (0x6702C / 0x67112, attr 0xFE)
 * four times each into the 0x613000/0x613400 pf blocks, arms a 0xD7-frame
 * timer and installs tick 0x14C9FE. */
int Obj_14C8F6_Tick(const f3mem_t *m, uint32_t obj)
{
    for (uint32_t i = 0; i < 4u; i++) {
        Gfx_DrawTilemapAttr_00523A(m, 0x0006702Cu, 0x00613000u + i * 0x20u, 0x00FEu, 0u);
        Gfx_DrawTilemapAttr_00523A(m, 0x00067112u, 0x00613400u + i * 0x20u, 0x00FEu, 0u);
    }
    wr16(m, obj + 0x06u, 0x00D7u);
    wr32(m, obj + 0x00u, 0x0014C9FEu);
    return 0;
}
