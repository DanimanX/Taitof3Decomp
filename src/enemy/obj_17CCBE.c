#include "obj_17CCBE.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"

/* ROM 0x17CCBE: spawn half of the 0x17CC9E record. Copies the +0x50/+0x54
 * position, the +0x16/+0x1A velocity and the +0x08 parent from the parent
 * object, latches the frame-list index from the record's first word (+0x40,
 * body pointer +0x3A) and primes the list's INIT half, then resets the zoom
 * (+0x1E := -0xF10), colour (+0x1C := 0) and alpha (+0x1D := +0x0F), plays
 * sound 0x29 and falls into the tick at 0x17CD28. */
int Obj_17CCBE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17CCBE..0x17CCDA: inherit from the parent */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x16u, rd16(m, a4 + 0x16u));
    wr16(m, obj + 0x1Au, rd16(m, a4 + 0x1Au));
    wr32(m, obj + 0x08u, rd32(m, a4 + 0x08u));

    /* 0x17CCE0..0x17CCEC: movea.l #0x17CC9E,A1 ; move.w (A1)+,D0 ;
     * move.w D0,(0x40,A0) ; move.l A1,(0x3A,A0) */
    uint32_t a1 = 0x0017CC9Eu;
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0w);
    wr32(m, obj + 0x3Au, a1);

    /* 0x17CCF0: lea (0x179A5C).l,A1 ; movea.l (0x0,A1,D0w*8),A1 ; jsr (A1)
     * -- INIT half of the 0x179A5C entry indexed by +0x40 */
    if (Obj_AnimInit(m, obj, d0w) < 0)
        return -1;

    /* 0x17CCFC..0x17CD14 */
    wr16(m, obj + 0x1Eu, 0xF0F0u);              /* move.w #-0xF10 = 0xF0F0 */
    wr8(m, obj + 0x1Cu, 0x00u);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x0Fu));
    wr16(m, obj + 0x06u, rd16(m, obj + 0x0Cu));

    /* 0x17CD14: move.w #0x29,-(SP) ; jsr 0x13F238 ; jsr 0x39D8 ; lea (0x2,SP),SP */
    Sound_PlayHooked(m, 0x0029u);

    return Obj_17CD28_Body(m, obj);                /* falls into 0x17CD28 */
}

/* ROM 0x17CD28: grow step -- follow the parent, animate, centre, shrink the
 * +0x1E/+0x1F scale bytes by 0x10, re-centre by the scaled size, draw, arm
 * 0x17CDA6. -1 from an untranslated 0x179A5C index. */
int Obj_17CD28_Body(const f3mem_t *m, uint32_t obj)
{
    /* 0x17CD28: movea.l (0x8,A0),A4 ; add the parent velocity to +0x50/+0x54,
     * write back and mirror into +0x14/+0x18 */
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = rd16(m, a4 + 0x50u);
    uint16_t d1 = rd16(m, a4 + 0x54u);
    d0 = (uint16_t)(d0 + rd16(m, obj + 0x16u));
    d1 = (uint16_t)(d1 + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    /* 0x17CD4C: TICK half of the 0x179A5C entry (+0x4,A1,D0w*8) */
    uint16_t d0w = rd16(m, obj + 0x40u);
    if (Obj_AnimTick(m, obj, d0w) < 0)
        return -1;

    /* 0x17CD5C..0x17CD6C: (A1)+ = +0x50 correction, (A1)+ = +0x54 correction,
     * each <<3, subtracted from +0x14/+0x18 */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t e1 = rd16(m, a1);
    a1 += 2u;
    uint16_t e0 = rd16(m, a1);
    a1 += 2u;
    e0 = (uint16_t)(e0 << 3);
    e1 = (uint16_t)(e1 << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - e0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - e1));

    /* 0x17CD70: addi.b #-0x10,(0x1F,A0) ; addi.b #-0x10,(0x1E,A0) */
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 0x10u));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 0x10u));

    /* 0x17CD7C..0x17CD94: D1 = sprite word <<4 ; D0 = bfextu (0x1F,A0){0:8}
     * (byte at +0x1F) ; mulu.w ; lsr.w #8 ; lsr.w #1 (low word only) ;
     * add to +0x14 and +0x18 */
    a1 = rd32(m, obj + 0x10u);
    uint16_t s1 = rd16(m, a1);
    s1 = (uint16_t)(s1 << 4);
    uint16_t v0 = rd8(m, obj + 0x1Fu);
    uint32_t prod = (uint32_t)v0 * (uint32_t)s1;
    uint16_t p0 = (uint16_t)prod;
    p0 = (uint16_t)(p0 >> 8);
    p0 = (uint16_t)(p0 >> 1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + p0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + p0));

    /* 0x17CD98: jsr 0x1282 (draw sprite) */
    Obj_DrawSprite(m, obj);

    /* 0x17CD9E: move.l #0x17CDA6,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x0017CDA6u);
    return 0;
}
