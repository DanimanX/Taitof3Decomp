#include "obj_111DF0.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x111D8E: retire the object */
static void sub_0D_MORPH(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x111DF0: bonus item sliding in from the right edge as the camera
 * scrolls; despawns once it falls behind the camera. */
int Obj_111DF0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x111DF0: (x16 + 0x70) <= camera x -> retire */
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x5Cu) + 0x70u);
    if ((int16_t)d0w <= (int16_t)rd16(m, F3_A5 - 0x5DA8u)) {
        sub_0D_MORPH(m, obj);
        return 0;
    }

    /* 0x111DFE: x = ((x - cam - 0x9F8000) divs 0x280) * 0x7A4 + 0x880000 */
    uint32_t d0 = rd32(m, obj + 0x5Cu);
    d0 -= rd32(m, F3_A5 - 0x5DA8u);
    d0 -= 0x9F8000u;
    int16_t q = (int16_t)((int32_t)d0 / 0x280);          /* divs.w, V ignored */
    d0 = (uint32_t)((int32_t)q * 0x7A4);                 /* muls.w */
    d0 += 0x880000u;
    wr32(m, obj + 0x14u, d0);

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x00111DF0u);
    return 0;
}
