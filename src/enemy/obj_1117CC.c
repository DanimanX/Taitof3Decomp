#include "obj_1117CC.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x1117CC: one animation step of the table at 0x111804 (decoded from
 * work/maincpu.bin); when the step ends, drift on the stage scroll velocity
 * and draw, else free.
 *   1117CC  lea (0x36,PC),A1  -> 0x111804
 *   1117D2  jsr 0x0CBD24
 *   1117D8  tst.w (0x0,A1,D0w) ; ble -> free -- the animation index is
 *           re-read from +0x3A after the step (0x0CBD24 leaves only its own
 *           loop's D0w there, while the golden capture shows the check using
 *           obj+0x3A).
 *   1117E0  subi.l #0xc000,(0x18,A0) */
int Obj_1117CC_Tick(const f3mem_t *m, uint32_t obj)
{
    const uint32_t a1 = 0x00111804u;

    /* 0x1117D2 */
    Obj_AnimFrameStep(m, obj, a1);
    int16_t off = (int16_t)rd16(m, obj + 0x3Au);

    /* 0x1117D8 */
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)off) <= 0) {
        Obj_Free(m, obj);                          /* 0x1117F8 */
        return 0;
    }

    /* 0x1117E0 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x0000C000u);
    Obj_SubScrollVel(m, obj);                      /* 0x1117E8 ROM 0x0CBE00 */
    Obj_DrawSprite(m, obj);                        /* 0x1117EE ROM 0x001282 */
    wr32(m, obj + 0x00u, 0x001117CCu);             /* 0x1117F4 ROM 0x1117C4 */
    return 0;
}
