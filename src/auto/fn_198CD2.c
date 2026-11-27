/* ROM 0x198CD2: object initialiser (A0 = object). Clears the common boss/enemy
 * fields via Obj_17A7FA (0x17A7FA), sets the animation index word at +0x40 to
 * 9, the flip/attr byte at +0x1D to 0xD0 (-0x50) and re-arms the object with
 * 0x198CEC. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_198CD2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                              /* 0x198CD2 -> 0x17A7FA */
    wr16(m, obj + 0x40u, 0x0009u);                   /* 0x198CD8 */
    wr8(m, obj + 0x1Du, 0xB0u);                      /* 0x198CDE move.b #-0x50 */
    wr32(m, obj + 0x00u, 0x00198CECu);               /* 0x198CE4 */
    return 0;
}
