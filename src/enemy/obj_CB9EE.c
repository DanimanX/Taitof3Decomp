#include "obj_CB9EE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

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

/* ROM 0x0CB9FE: animate (script 0xCBAE6), upload the frame's chars, or free
 * and blank the two map cells at obj+0xC when the script ends. */
static int lab_0CB9FE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x000CBAE6u;
    Obj_AnimFrameStep(m, obj, a1);
    uint16_t d0 = rd16(m, obj + 0x3Au);   /* final offset (+0x3A), as in ROM */
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) < 0) {
        /* 0x0CBA36 */
        uint32_t cell = rd32(m, obj + 0x0Cu);
        wr32(m, cell, 0u);
        wr32(m, cell + 0x80u, 0u);
        Obj_Free(m, obj);
        return 0;
    }
    sub_005A86(m, (uint16_t)rd8(m, 0x00010B79u), rd32(m, obj + 0x10u));
    wr32(m, obj + 0x00u, 0x000CBA34u);
    return 0;
}

/* ROM 0x0CB9EE */
int Obj_CB9EE_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x80u) == 0u) {
        /* 0x0CB9C0 */
        Obj_Animate(m, obj, 0x000CBACCu);
        sub_005A86(m, (uint16_t)rd8(m, 0x00010B79u), rd32(m, obj + 0x10u));
        wr32(m, obj + 0x00u, 0x000CB9EEu);
        return 0;
    }
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    return lab_0CB9FE(m, obj);
}
