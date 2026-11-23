#include "obj_DC77A.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBE34: blinking colour override (obj+0x35, obj+0x1D, obj+0x4F) */

/* ROM 0x0CF7FE: arm the script when re-entering the screen */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x14F)
        return;
    f7 = (uint8_t)(f7 & ~0x80u);
    wr8(m, obj + 0x07u, f7);
    wr8(m, obj + 0x07u, (uint8_t)(f7 | 0x02u));
}

/* ROM 0x0DC77A: scrolling ground target (score drop on death) */
int Obj_DC77A_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        return -1;   /* 0x0DC7E8: spawns 0x117C28 with the caller's D0 high bytes (unknown here) */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    sub_0CF7FE(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    {   /* ROM 0x0D0DD8: append to the target list at A5+0x8FE */
        uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    }
    Obj_BlinkColour(m, obj);
    if (rd8(m, obj + 0x1Du) != 0u)
        Obj_DrawSprite(m, obj);
    return 0;
}
