#include "obj_CAF1A.h"
#include "../video/sprite.h"

/* ROM 0x0CB14A: screen-centred placement with a horizontal spread that scales
 * with the zoom byte (obj+0x1E) and the sprite height; spills the same value
 * scaled x4, negated, to A5+0x1D5C (fine scroll). */
static void sub_0CB14A(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0x00A0u);
    wr16(m, obj + 0x18u, 0x0074u);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)(rd8(m, obj + 0x1Eu) - 0x100u);   /* subi.w #0x100 */
    /* muls.w (A1),D0 ; asr.w #5,D0w (D0.w sign-extended into a 32-bit dest) */
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (uint16_t)((uint32_t)prod >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    d0 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(0u - d0);
    wr16(m, F3_A5 + 0x1D5Cu, d0);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x0CAF1A: one-shot zoom-in title card: reset zoom/colour, place, then
 * count down obj+0x06 by 0x30 and switch to 0x0CAF3E. */
int Obj_CAF1A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x1Fu, rd8(m, obj + 0x07u));
    wr8(m, obj + 0x1Eu, rd8(m, obj + 0x07u));
    sub_0CB14A(m, obj);
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 0x30u));
    if (rd8(m, obj + 0x06u) == 0u) {
        wr32(m, obj + 0x00u, 0x000CAF1Au);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x000CAF3Eu);
    return 0;
}
