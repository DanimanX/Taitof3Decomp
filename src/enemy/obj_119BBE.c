#include "obj_119BBE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* cmp2.w (abs).l,Dn; bcs -> outside the signed bounds {lo.w, hi.w} at addr */
static int out_of_bounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x119BBE: debris under gravity +0x42 */
int Obj_119BBE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x2Cu));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x30u));
    int32_t g = (int32_t)(int16_t)rd16(m, obj + 0x42u);
    wr32(m, obj + 0x30u, rd32(m, obj + 0x30u) + ((uint32_t)g << 7));
    Obj_Animate(m, obj, rd32(m, obj + 0x0Cu));
    Obj_DrawSprite(m, obj);
    if (out_of_bounds(m, 0x000CC112u, rd16(m, obj + 0x14u)) ||
        out_of_bounds(m, 0x000CC116u, rd16(m, obj + 0x18u))) {
        Obj_Free(m, obj);                    /* 0x119C0C */
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00119BBEu);       /* 0x119BB6 */
    return 0;
}
