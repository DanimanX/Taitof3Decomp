#include "obj_E8CAE.h"
#include "obj_E8B72.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables scaled by D1w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t d0 = rd32(m, 0x000D2D64u + (uint32_t)((int32_t)(int16_t)d0w * 4));
    uint32_t d1 = rd32(m, 0x000D2ECCu + (uint32_t)((int32_t)(int16_t)d0w * 4));
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x0E8CAE */
int Obj_E8CAE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u));
    Obj_InitMoverB(m, obj, 0x000EABAAu);
    Obj_SetAnim(m, obj, 0x000E8D42u);

    uint32_t vx, vy;
    sub_0D2D44(m, 0x005Au, 0x000Au, &vx, &vy);
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    Obj_DrawSprite(m, obj);
    Sound_PlayHooked(m, 0x0032u);
    wr32(m, obj + 0x00u, 0x000E8D12u);
    return 0;
}
