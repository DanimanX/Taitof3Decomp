#include "obj_ECF3E.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0690: script advance (6-byte frames) */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - 1u));
    if (rd16(m, obj + 0x4Cu) != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u);
    a1 += (uint32_t)(int32_t)(int16_t)0x000Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0w = rd16(m, a1); a1 += 2u;
    if ((int16_t)d0w < 0) {
        if (rd16(m, a1) == 0u)
            return;
        a1 = (uint32_t)(a1 - (uint32_t)(int32_t)(int16_t)rd16(m, a1));
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
        d0w = rd16(m, a1); a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)d0w);
        wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }
    wr16(m, obj + 0x4Cu, d0w);
    d0w = rd16(m, a1); a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0ECF3E */
int Obj_ECF3E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0ECF3E */
    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);
    d0 = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);

    sub_0D0690(m, obj);                     /* 0x0ECF4E */
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);             /* 0x0ECF58 */
        return 0;
    }

    Obj_DrawSprite(m, obj);                 /* 0x0ECF5E */
    return 0;
}
