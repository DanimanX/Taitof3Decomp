#include "obj_DBF70.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0690: step the 12-byte script record at +0x48 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - 1u));
    if (rd16(m, obj + 0x4Cu) != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u);
    a1 = (a1 + 0xCu) & 0x00FFFFFFu;                         /* adda.l: 24-bit bus */
    wr32(m, obj + 0x48u, a1);

    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0w >= 0) {
        wr16(m, obj + 0x4Cu, d0w);
        d0w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)d0w);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    if (rd16(m, a1) == 0u)
        return;

    a1 = ((a1 & 0xFFFF0000u) | (uint32_t)((uint16_t)a1 - rd16(m, a1))) & 0x00FFFFFFu; /* suba.w (A1),A1 */
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    d0w = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0DBF70 */
int Obj_DBF70_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);

    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);

    sub_0D0690(m, obj);

    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    Obj_DrawSprite(m, obj);
    return 0;
}
