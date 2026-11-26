/* ROM 0x0F8C1E: attached-part init (A0 = object).
 * Seeds from 0x0F872A (0x0CEC4C), installs behaviour 0x0F8CBA (0x0D0678),
 * clears +0x58.w, and arms tick 0x0F8C3E. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F8C1E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_SeedFromParams(m, obj, 0x000F872Au);                 /* 0x0F8C1E..0x0F8C22 */
    Obj_StartBehaviour(m, obj, 0x000F8CBAu);                 /* 0x0F8C28..0x0F8C2C */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x000F8CBCu);
    r->a[1] = 0x000F8CC2u;

    wr16(m, obj + 0x58u, 0u);                                /* 0x0F8C32 */
    wr32(m, obj, 0x000F8C3Eu);                               /* 0x0F8C36 */
    return 0;
}
