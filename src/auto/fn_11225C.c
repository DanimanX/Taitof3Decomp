/* ROM 0x11225C: shot split/death (A0 = object). Subtracts the world scroll
 * (A5+0x422C) from the X long +0x14, spawns two children of group 0 (one scan
 * group) -- 0x1122C2 with the object's X as argument and 0x112368 with 0 --
 * plays hooked sound 0x16 and frees the object. D0 keeps the second spawn's
 * object (Obj_Free saves D0; 0x112368's spawn result is the last D0), A0 and
 * A1 are untouched. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_E8B72.h"

int Fn_11225C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x14u,                          /* ROM 0x11225C..0x112260 */
         rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x422Cu));
    r->d[0] = rd32(m, obj + 0x14u);               /* ROM 0x112264 move.l (0x14,A0),D0 */

    r->d[0] = Obj_Spawn(m, obj, 0x001122C2u, 0u, 1u, r->d[0]); /* ROM 0x112268..0x112278 */
    Sound_PlayHooked(m, 0x0016u);                 /* ROM 0x112282..0x11228C */
    r->d[0] = Obj_Spawn(m, obj, 0x00112368u, 0u, 1u, 0u);      /* ROM 0x112296..0x1122AA */
    Obj_Free(m, obj);                             /* ROM 0x1122B4..0x1122B6 */
    return 0;                                     /* ROM 0x1122C0 rts */
}
