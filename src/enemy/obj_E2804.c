/* ROM 0x0E2804 */
#include "obj_E2804.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static void follow_parent(const f3mem_t *m, uint32_t obj, uint32_t p,
                          uint16_t dx, uint16_t dy)
{
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dx));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));
}

static void set_spr_hdr(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
}

/* ROM 0x0E2804: part attached to a parent object */
int Obj_E2804_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);

    if (rd8(m, p + 0x06u) == 0u) {
        wr8(m, obj + 0x06u, 0u);
        Obj_0CEF0E(m, obj);
        return 0;
    }

    if (rd8(m, obj + 0x06u) == 4u) {
        follow_parent(m, obj, p, 0x08u, 0x0Fu);
        Obj_StepScript(m, obj);
        Obj_DrawSprite(m, obj);
        Obj_DrawSprite(m, rd32(m, obj + 0x08u));
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        follow_parent(m, obj, p, 0x18u, 0x20u);
        switch (rd8(m, obj + 0x06u)) {
        case 1u:
            if (rd8(m, obj + 0x23u) == 0u) {
                Obj_AimAtPlayers(m, obj, 0x000E29F2u);
                if (rd8(m, obj + 0x23u) == 0u)
                    break;
            }
            {
                uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
                wr8(m, obj + 0x23u, t);
                if (t == 0u) {
                    Obj_StartBehaviour(m, obj, 0x000E2A34u);
                    wr8(m, obj + 0x06u, 2u);
                }
            }
            break;
        case 2u:
            Obj_StepScript(m, obj);
            if (rd16(m, obj + 0x4Cu) != 0u) {
                int16_t d1 = (int16_t)rd16(m, obj + 0x68u);
                if (d1 < 0) {
                    uint32_t c = Obj_Spawn(m, obj, 0x000E2C24u, 9u, 2u, 0u);
                    if ((int32_t)c < 0)
                        goto state2_next;
                    wr8(m, rd32(m, obj + 0x08u) + 0x06u, 3u);
                } else if (d1 == 0) {
                    wr8(m, rd32(m, obj + 0x08u) + 0x06u, 3u);
                }
                break;
            }
        state2_next:
            Obj_StartBehaviour(m, obj, 0x000E2B9Cu);
            wr8(m, obj + 0x06u, 3u);
            break;
        case 3u:
            Obj_StepScript(m, obj);
            if (rd16(m, obj + 0x4Cu) == 0u) {
                set_spr_hdr(m, obj, 0x000E2A2Au);
                wr8(m, obj + 0x06u, 1u);
            }
            break;
        default:
            return -1;
        }
        Obj_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
        Obj_DrawSprite(m, obj);
        Obj_DrawSprite(m, rd32(m, obj + 0x08u));
        return 0;
    }

    if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x190u);
    }

    uint32_t c = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
    if ((int32_t)c >= 0) {
        wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x20u));
        wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x1Au));
        wr16(m, c + 0x58u, 0u);
        wr16(m, c + 0x60u, 0u);
    }

    p = rd32(m, obj + 0x08u);
    follow_parent(m, obj, p, 0x08u, 0x0Fu);
    Obj_StartBehaviour(m, obj, 0x000E2BE4u);
    wr8(m, obj + 0x1Du, 0u);
    wr8(m, obj + 0x06u, 4u);
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    return 0;
}
