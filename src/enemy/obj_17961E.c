#include "obj_17961E.h"

/* ---- copied verbatim from src/enemy/obj_17A8E0.c (same ROM addresses) ---- */

/* ROM 0x1798F8: index 4 tick: one of 16 direction frames {sprite.l, 2c.l,
 * 30.l} (12 bytes each) at +0x3A, picked by +0x20 with the coarse (>>4)
 * angle step. */
static void scr_dir16_box(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + (uint32_t)idx * 0x0Cu;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
}

/* ---- ROM 0x17961E (0x179A5C animation index 4 init): hold the first frame
 * for one extra tick, then run the same routine as the index 4 tick. The tick
 * jumps back to this init (bra.w 0x1798F8 at 0x179624), so the countdown is
 * re-pinned to 1 rather than reloaded from the record. */
int Obj_17961E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);                   /* move.w #$1, $3e(a0) */
    scr_dir16_box(m, obj);                      /* bra.w 0x1798F8 */
    return 0;
}
