#include "obj_F754C.h"
#include "../objects/obj_common.h"

/* ROM 0x0D0DC2: append to the shootable list and the hazard list */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);                    /* move.l A0,(A1)+ */
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0F754C */
int Obj_F754C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);

    /* 0x0F7550 */
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, a1 + 0x06u) == 0x09u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0F7566 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr32(m, obj + 0x24u, 0x000F0000u);
        wr32(m, a1 + 0x24u, 0u);
        wr32(m, a1 + 0x36u, rd32(m, obj + 0x36u));
    }

    /* 0x0F7580 */
    uint8_t d0b = rd8(m, obj + 0x35u);
    if (d0b != 0u) {
        wr8(m, a1 + 0x35u, d0b);
        wr8(m, obj + 0x35u, 0u);
    }

    /* 0x0F7590 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0024u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0022u));
    if (rd8(m, a1 + 0x06u) >= 0x06u)
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));

    sub_0D0DC2(m, obj);
    return 0;
}
