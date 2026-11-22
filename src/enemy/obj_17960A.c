#include "obj_17960A.h"

/* ---- copied verbatim from src/enemy/obj_17A8E0.c (same ROM addresses) ---- */

/* ROM 0x1798BA: index 2 tick: one of 32 direction frames {sprite.l} at
 * +0x3A, picked by +0x20 with the fine (>>3) angle step. */
static void scr_dir32(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    uint32_t a1 = rd32(m, obj + 0x3Au) + (uint32_t)idx * 4u;
    wr32(m, obj + 0x10u, rd32(m, a1));
}

/* ---- ROM 0x17960A (0x179A5C animation index 2 init): hold the first frame
 * for one extra tick, then run the same routine as the index 2 tick. As with
 * index 1, the tick jumps back to this init (bra.w 0x1798BA at 0x179610), so
 * the countdown is re-pinned to 1 rather than reloaded from the record. */
int Obj_17960A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);                   /* move.w #$1, $3e(a0) */
    scr_dir32(m, obj);                          /* bra.w 0x1798BA */
    return 0;
}
