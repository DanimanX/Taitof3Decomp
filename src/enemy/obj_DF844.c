#include "obj_DF844.h"
#include "../objects/obj_common.h"

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

/* ROM 0x0DF844 */
int Obj_DF844_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DF844: obj+0x14 += obj+0x3A, obj+0x18 += obj+0x3E (16.16) */
    uint32_t d0 = rd32(m, obj + 0x3Au);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
    d0 = rd32(m, obj + 0x3Eu);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);

    /* 0x0DF854 */
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) != 0u)
        return 0;                                   /* 0x0DF862 rts */

    /* 0x0DF864: animation ended -> unlink obj from its parent's child list
     * (obj+0x50 prev, obj+0x54 next) and die. */
    uint32_t a1 = rd32(m, obj + 0x50u);
    uint32_t a2 = rd32(m, obj + 0x54u);
    wr32(m, a1 + 0x54u, a2);
    if ((int32_t)a2 >= 0)                           /* bmi -> 0x0CEE74 */
        wr32(m, a2 + 0x50u, a1);

    /* 0x0DF87A: bra 0x0CEE74 */
    Obj_StartDying(m, obj);
    return 0;
}
