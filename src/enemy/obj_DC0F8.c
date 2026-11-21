#include "obj_DC0F8.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0690: step the animation sequence at obj+0x48 (12-byte entries,
 * duration.w, flip.w, sprdef.l, tail.l; negative duration = loop back by the
 * following word, or hold when it is 0). */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    /* 0x0D0690 */
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    /* 0x0D069A */
    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;   /* adda.w #0xC */
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        /* 0x0D06C0 */
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 = (uint32_t)(a1 - 2u - (uint32_t)(int32_t)back); /* suba.w (A1),A1 */
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;                                     /* 0x0D06DE */
    }

    /* 0x0D06AC */
    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0DC0F8 */
int Obj_DC0F8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC0F8: obj+0x14 += obj+0x50 */
    uint32_t d0 = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);

    /* 0x0DC100: obj+0x18 += obj+0x54 - scroll_y(A5+0x4228) */
    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);

    /* 0x0DC10C */
    sub_0D0690(m, obj);

    /* 0x0DC112: animation held at 0 -> object dies */
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0DC11C */
    Obj_DrawSprite(m, obj);
    return 0;                                       /* 0x0DC122 rts */
}
