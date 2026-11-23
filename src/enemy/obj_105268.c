#include "obj_105268.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_common.h"

/* ROM 0x0CBE34 */

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x105268: hit box that follows the parent at +0x08 */
int Obj_105268_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr32(m, obj + 0x24u, 0x000F0000u);
        wr32(m, a1 + 0x24u, 0u);
        wr8(m, a1 + 0x35u, rd8(m, obj + 0x35u));
        wr32(m, a1 + 0x36u, rd32(m, obj + 0x36u));
    }
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    a1 = rd32(m, obj + 0x08u);
    wr8(m, a1 + 0x22u, rd8(m, obj + 0x1Du));
    return 0;
}
