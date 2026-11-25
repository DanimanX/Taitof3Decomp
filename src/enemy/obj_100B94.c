#include "obj_100B94.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables scaled by D1w (0..50) */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    int32_t idx = (int32_t)(int16_t)d0w * 4;
    uint32_t d0 = rd32(m, (uint32_t)(0x000D2D64 + idx));
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2ECC + idx));
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
    return 0;
}

/* ROM 0x100B94 */
int Obj_100B94_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t speed = rd16(m, obj + 0x60u);
    if (speed > 50u)
        return -1;

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0018u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u));
    Obj_0CEDE4(m, obj, 0x001086FCu);
    wr8(m, obj + 0x1Du, 0xCAu); /* -0x36 */

    /* ROM 0x0E6098 */
    Obj_StartBehaviour(m, obj, 0x000E612Au);

    uint32_t d0, d1;
    if (sub_0D2D44(m, rd16(m, obj + 0x58u), speed, &d0, &d1) < 0)
        return -1;
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    Obj_DrawSprite(m, obj);

    int16_t cmd = Sound_PreCmdHook(m, 0x001B);
    Sound_PlayCmd(m, cmd);

    wr32(m, obj + 0x00u, 0x000E60EAu);
    return 0;
}
