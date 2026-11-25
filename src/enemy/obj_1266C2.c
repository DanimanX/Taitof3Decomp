#include "obj_1266C2.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x11B288: steer angle (+0x20.b) toward (+0x50, +0x52) by up to +-2 */
static void sub_11B288(const f3mem_t *m, uint32_t obj)
{
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, F3_A5 + 0x30D2u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, F3_A5 + 0x30D6u));
    int8_t diff = (int8_t)((uint8_t)Obj_Atan2(m, dx, dy) - rd8(m, obj + 0x20u));
    if (diff > 2)
        diff = 2;
    else if (diff < -2)
        diff = -2;
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));
}

/* ROM 0x11B2CA: ramp (+0x56.w) toward (+0x54.b) and store high byte in +0x42.w */
static void sub_11B2CA(const f3mem_t *m, uint32_t obj)
{
    int8_t target = (int8_t)rd8(m, obj + 0x54u);
    int8_t cur = (int8_t)rd8(m, obj + 0x56u);
    if (target != cur) {
        if (target > cur)
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x0020u));
        else
            wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x0010u));
    }
    wr16(m, obj + 0x42u, (uint16_t)((int16_t)rd16(m, obj + 0x56u) >> 8));
}

/* ROM 0x11B262: integrate shared position A5+0x30D2/+0x30D6 along (+0x20.w >> 8) */
static void sub_11B262(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

/* ROM 0x1266C2 */
int Obj_1266C2_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_11B288(m, obj);
    sub_11B2CA(m, obj);
    sub_11B262(m, obj);

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {
        wr32(m, obj + 0x00u, 0x001266C2u);
        return 0;
    }

    int16_t cmd = Sound_PreCmdHook(m, 0x0013);
    Sound_PlayCmd(m, cmd);
    wr8(m, obj + 0x54u, 8u);
    wr16(m, obj + 0x50u, 0x0040u);
    wr16(m, obj + 0x52u, 0x0040u);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x00126710u);
    return 0;
}
