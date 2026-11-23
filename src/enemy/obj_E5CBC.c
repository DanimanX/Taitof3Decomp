#include "obj_E5CBC.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0CEF0E */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEF44u);
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 1u) {
        uint32_t a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

/* ROM 0x0E5CBC */
int Obj_E5CBC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x00000064u);
            sub_0CEF0E(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) > 0 && (rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        uint32_t vx = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + vx);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + rd32(m, obj + 0x54u));
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        sub_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        sub_0D0E04(m, obj, 0x00000064u);

    uint32_t child = Obj_Spawn(m, obj, 0x000E6082u, 9u, 2u, 0u);
    if ((int32_t)child >= 0) {
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0014u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0014u));
        wr16(m, child + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
    }

    Obj_StartDying(m, obj);
    return 0;
}
