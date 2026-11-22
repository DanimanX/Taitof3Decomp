#include "obj_179572.h"

/* ROM 0x179572 (0x179A5C animation index 3 init): plain timed frame with a
 * hit box at +0x3A -- {duration.w, sprite.l, 2c.l, 30.l}. */
int Obj_179572_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));          /* move.w (a1)+, $3e(a0) */
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));     /* move.l (a1)+, $10(a0) */
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 6u));     /* move.l (a1)+, $2c(a0) */
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Au));  /* move.l (a1)+, $30(a0) */
    return 0;
}
