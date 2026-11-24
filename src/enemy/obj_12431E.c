#include "obj_12431E.h"
#include "../objects/obj_pool.h"

/* ROM 0x12431E */
int Obj_12431E_Tick(const f3mem_t *m, uint32_t obj) {
    /* A1 = *(F3_A5 + 0x30DE); *(A1) = 0x11B34E */
    wr32(m, rd32(m, F3_A5 + 0x30DEu), 0x0011B34Eu);

    /* Spawn: arg=0x20, ngroups=2, group=3, fn=(void*)0x13E9E8 */
    Obj_Spawn(m, obj, 0x13E9E8u, 3, 2, 0x20u);

    /* Clear active flag at +0x1D */
    wr8(m, obj + 0x1Du, 0x00u);

    /* Set sprite size to 200 (0xC8) */
    wr16(m, obj + 0x6u, 0xC8u);

    /* Set code pointer: init-only, next tick at ROM 0x124358 */
    wr32(m, obj + 0x0u, (uint32_t)0x124358);

    return 0;
}
