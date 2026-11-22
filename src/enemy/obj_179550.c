#include "obj_179550.h"

/* ROM 0x179550 (0x179A5C animation index 0 init): plain frame record at
 * +0x3A -- {duration.w, sprite.l} -- loaded into the countdown (+0x3E) and
 * the sprite pointer (+0x10). */
int Obj_179550_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));          /* move.w (a1)+, $3e(a0) */
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));     /* move.l (a1)+, $10(a0) */
    return 0;
}
