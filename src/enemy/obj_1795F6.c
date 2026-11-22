#include "obj_1795F6.h"

/* ---- copied verbatim from src/enemy/obj_17A8E0.c (same ROM addresses) ---- */

/* ROM 0x179880: one of 16 direction frames {sprite.l} at +0x3A, picked by
 * +0x20 (0x20 added before the >>4 makes the index wrap like the byte). */
static void scr_dir16(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 0x20u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + (uint32_t)idx * 4u;
    wr32(m, obj + 0x10u, rd32(m, a1));
}

/* ---- ROM 0x1795F6 (0x179A5C animation index 1 init): hold the first frame
 * for one extra tick, then run the same routine as the index 1 tick. Unlike
 * index 0's tick (0x179664, which reloads the record's own duration through
 * bsr 0x179550), the tick jumps straight back to this init: the bra.w
 * 0x179880 at 0x1795FC re-pins the countdown to 1. */
int Obj_1795F6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);                   /* move.w #$1, $3e(a0) */
    scr_dir16(m, obj);                          /* bra.w 0x179880 */
    return 0;
}
