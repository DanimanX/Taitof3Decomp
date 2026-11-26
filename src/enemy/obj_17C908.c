#include "obj_17C908.h"
#include "../video/sprite.h"
#include "../objects/obj_anim.h"

/* ROM 0x17C908: spawn entry of the 0x17D66E cluster child. It points +0x0C at
 * the turn table 0x17CAD2 (8-byte records {anim.l, +0x1C.w, dx.b, dy.b}
 * indexed by the parent's angle) and falls into the one-shot 0x17C910: follow the parent
 * (+0x50/+0x54/+0x16/+0x1A/+0x20 and its own +0x08), replay the parent's angle,
 * run that row's 0x179A5C INIT and add the row's signed dx/dy. In the same tick
 * 0x17C97A reapplies the offset over the parent position into +0x50/+0x54 and
 * +0x14/+0x18, runs the row's TICK, subtracts the sprite size from +0x14/+0x18
 * and hands over to 0x17C9D0. */

/* ROM 0x17C97A: motion + animation tick. Returns 0, or -1 through the 0x179A5C
 * dispatch (indices 6/7 have no translation) before any of its writes. */
int Obj_17C97A_Body(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);             /* parent */
    uint16_t d0w = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1w = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0w);
    wr16(m, obj + 0x54u, d1w);
    wr16(m, obj + 0x14u, d0w);
    wr16(m, obj + 0x18u, d1w);

    /* 0x17C99E: move.w (0x40,A0),D0w ; movea.l (0x4,0x179A5C,D0w*8),A1 ; jsr */
    uint16_t idx = rd16(m, obj + 0x40u);
    if (Obj_AnimTick(m, obj, idx) < 0)
        return -1;

    /* 0x17C9AE: +0x10 sprite def {w.w, h.w}; both ASL.w #3 and SUB.w wrap */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));

    Obj_DrawSprite(m, obj);                         /* 0x17C9C2 jsr 0x1282 */
    wr32(m, obj + 0x00u, 0x0017C9D0u);              /* 0x17C9C8 */
    return 0;
}

int Obj_17C908_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17C908: move.l #0x17CAD2,(0x0C,A0) */
    wr32(m, obj + 0x0Cu, 0x0017CAD2u);

    /* 0x17C910: follow the parent */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x16u, rd16(m, a4 + 0x16u));
    wr16(m, obj + 0x1Au, rd16(m, a4 + 0x1Au));
    wr16(m, obj + 0x20u, rd16(m, a4 + 0x20u));
    wr32(m, obj + 0x08u, rd32(m, a4 + 0x08u));

    /* 0x17C938: (uint8)((+0x20 + 8) >> 4) indexes the turn table */
    uint16_t d0w = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a3 = rd32(m, obj + 0x0Cu);             /* 0x17CAD2 */
    uint32_t a2 = a3 + (uint32_t)d0w * 8u;          /* lea (0,A1,D0.w*8) */

    /* 0x17C94A..0x17C952: 8-byte record {anim.l, +0x1C.w, dx.b, dy.b} */
    uint32_t a1 = rd32(m, a2);
    a2 += 4u;
    d0w = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0w);
    wr32(m, obj + 0x3Au, a1);

    /* 0x17C956: movea.l (0x0,0x179A5C,D0w*8),A1 ; jsr -- the INIT half; -1
     * before the record's dx/dy are applied */
    if (Obj_AnimInit(m, obj, d0w) < 0)
        return -1;

    /* 0x17C962: move.w (A2)+,(0x1C,A0) ; the two signed bytes into +0x16/+0x1A */
    wr16(m, obj + 0x1Cu, rd16(m, a2));
    a2 += 2u;
    int16_t bx = (int16_t)(int8_t)rd8(m, a2);
    int16_t by = (int16_t)(int8_t)rd8(m, a2 + 1u);
    wr16(m, obj + 0x16u, (uint16_t)((int16_t)rd16(m, obj + 0x16u) + bx));
    wr16(m, obj + 0x1Au, (uint16_t)((int16_t)rd16(m, obj + 0x1Au) + by));
    wr16(m, obj + 0x1Eu, 0u);                       /* 0x17C976 */

    return Obj_17C97A_Body(m, obj);                      /* 0x17C97A */
}
