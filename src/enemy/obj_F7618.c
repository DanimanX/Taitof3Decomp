#include "obj_F7618.h"
#include "../objects/obj_common.h"

/* ROM 0x0D0690: step the 12-byte animation record at +0x48 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - 1u));
    if (rd16(m, obj + 0x4Cu) != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u);
    a1 = (a1 + 0x0Cu) & 0x00FFFFFFu;                     /* adda.w #0xC */
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

    if (rd16(m, a1) == 0u)                               /* tst.w (A1) */
        return;

    a1 = ((a1 & 0xFFFF0000u) | (uint32_t)((uint16_t)a1 - rd16(m, a1))) & 0x00FFFFFFu;
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

/* ROM 0x0F7618 */
int Obj_F7618_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x06u, 0u);

    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t d0b = rd8(m, a1 + 0x06u);
    if (d0b == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (d0b < 0x04u || d0b == 0x09u) {
        /* 0x0F766C: rts */
        return 0;
    }

    /* 0x0F763A */
    wr8(m, obj + 0x06u, 0x01u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    uint32_t d0 = (uint32_t)rd8(m, a1 + 0x06u);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    a2 = (a2 + (uint32_t)(d0 * 4u)) & 0x00FFFFFFu;       /* lea (0,A1,D0.w*4),A1 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, a2)));
    a2 += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a2)));
    sub_0D0690(m, obj);
    return 0;
}
