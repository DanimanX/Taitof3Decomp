#include "obj_FF3E4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0FF3E4: leader of a vertical chain of markers (children at +0x50 /
 * +0x54), scrolls, closes gaps, then draws every chain member. */
int Obj_FF3E4_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        /* 0x0FF49C */
        wr8(m, obj + 0x06u, 0x01u);
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0FF3EE: scroll */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    /* 0x0FF3FE: off the left edge */
    if ((int16_t)rd16(m, obj + 0x14u) < -0x10) {
        wr8(m, obj + 0x06u, 0x01u);
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0FF408: keep the chain spacing (spawn a new tail segment when the
     * next member is more than 0x19 px below) */
    {
        uint32_t next = rd32(m, obj + 0x54u);
        int need = 0;
        if ((int32_t)next < 0) {
            need = 1;
        } else {
            if (rd8(m, next + 0x06u) != 0u) {
                need = 0;
            } else {
                uint16_t dy = (uint16_t)(rd16(m, next + 0x18u) - rd16(m, obj + 0x18u));
                need = ((int16_t)dy > 0x19);
            }
        }
        if (need) {
            uint32_t nw = Obj_Spawn(m, obj, 0x000FF4A8u, 5u, 2u, 0u);
            if (nw != 0xFFFFFFFFu) {
                uint32_t prev = rd32(m, obj + 0x54u);
                wr8(m, nw + 0x06u, 0x01u);
                wr32(m, obj + 0x54u, nw);
                wr32(m, nw + 0x50u, obj);
                wr32(m, nw + 0x54u, prev);
                if ((int32_t)prev >= 0)
                    wr32(m, prev + 0x50u, nw);
            }
        }
    }

    /* 0x0FF46C: draw every member of the chain */
    uint32_t cur = rd32(m, obj + 0x54u);
    while ((int32_t)cur >= 0) {
        if (rd8(m, cur + 0x06u) == 0u) {
            wr32(m, cur + 0x14u, rd32(m, cur + 0x14u) - rd32(m, F3_A5 + 0x4224u));
            wr32(m, cur + 0x18u, rd32(m, cur + 0x18u) - rd32(m, F3_A5 + 0x4228u));
            Obj_DrawSprite(m, cur);
        }
        obj = cur;
        cur = rd32(m, obj + 0x54u);
    }
    return 0;
}
