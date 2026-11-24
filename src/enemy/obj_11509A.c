#include "obj_11509A.h"
#include "../objects/obj_pool.h"

/* ROM 0x11509A */
int Obj_11509A_Tick(const f3mem_t *m, uint32_t obj) {
    /* Spawn: arg=0 (pea 0.w -> 0), ngroups=2, group=3, fn=(void*)0x1134B2 */
    Obj_Spawn(m, obj, 0x1134B2u, 3, 2, 0x0u);

    /* Clear multiple object flags */
    wr8(m, obj + 0x34u, 0x00u);  /* bit-clear at offset 0x34 */
    wr8(m, obj + 0x35u, 0x00u);  /* bit-clear at offset 0x35 */
    wr8(m, obj + 0x1Du, 0x00u);  /* is_active flag */
    wr8(m, obj + 0x4Fu, 0x00u);  /* additional flag */

    /* Set object type to 2 */
    wr16(m, obj + 0x22u, 0x02u);

    /* Set sprite size to 300 (0x12C) */
    wr16(m, obj + 0x6u, 0x12Cu);

    /* Load pointer from external table into obj+0x3A */
    wr32(m, obj + 0x3Au, rd32(m, F3_A5 + 0x1D28u));
    wr32(m, obj + 0x00u, 0x001150E0u);

    return 0;
}
