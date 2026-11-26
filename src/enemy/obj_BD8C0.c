#include "obj_BD8C0.h"
#include "obj_BD47E.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0BD8C0: one-shot init tick. Clears the pair blocks at A5+0x4214 /
 * A5-0x5D98 and A5+0x4234, resets the 0x62A400/0x628400 zoom ramp and the
 * 0x624400 scroll ramps (all rates zero, shared helpers in obj_BD47E.c) and
 * arms 0x0BD990. */

/* ROM 0x0BD8C0 */
int Obj_BD8C0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BD8C4..0x0BD8E2: jsr 0x16AB2C with D0 = D1 = 0 */
    Mem_WritePair64(m, F3_A5 + 0x4214u, F3_A5 - 0x5D98u, 0u, 0u);

    /* 0x0BD8E2..0x0BD8F6: jsr 0x16AB26 with D0 = D1 = 0 */
    Obj_16AB26(m, F3_A5 + 0x4234u, 0u, 0u);

    /* 0x0BD8FC..0x0BD92C: jsr 0x176832(0x62A400, 0x628400, 0, 0xE7, 0, 0, 2) */
    sub_176832(m, 0x0062A400u, 0x00628400u, 0x0000u, 0x00E7u, 0u, 0u, 0u, 0x0002u);

    /* 0x0BD930..0x0BD950: jsr 0x1768CA(0x624400, 0x74, 0, 0, 0) */
    sub_1768CA(m, 0x00624400u, 0x0074u, 0x0000u, 0u, 0u, 0u);

    /* 0x0BD95A..0x0BD97A: jsr 0x1768CA(0x624400, 0x74, 0xE7, 0, 0) */
    sub_1768CA(m, 0x00624400u, 0x0074u, 0x00E7u, 0u, 0u, 0u);

    /* 0x0BD984..0x0BD98E */
    wr32(m, obj + 0x00u, 0x000BD990u);
    return 0;
}
