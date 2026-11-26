#include "obj_17DA00.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x17DA00: the 0x17DA00 child of the 0x17D912 cluster shot -- 0x17D912
 * spawns it into group 0xB with arg 0 and it only runs once (0x17DA4A finishes
 * with 0x1282, draw sprite, then arms 0x17DAA0).
 *
 * INIT (0x17DA00): copies the parent's +0x50/+0x54, +0x16/+0x1A and +0x08 onto
 * the child, sets flip/zoom (+0x1C := 8, +0x1E := 0), latches the frame-list
 * index at 0x17DB54 into +0x40/+0x3A (0x8, then the record body) and runs its
 * INIT half.
 *
 * TICK (0x17DA4A): position = parent position + the +0x16/+0x1A deltas, runs
 * the index's tick half, subtracts (sprite row, sprite col) each << 3 from
 * +0x14/+0x18 and emits the sprite. The index 8 INIT body is 0x17DB54 itself,
 * which does not end in an rts, so the real object only ever tick-returns into
 * whatever follows that record -- captured code has it arming 0x17DAA0 as the
 * next tick instead, which is what the tail below reproduces.
 * The +0x3A record body at 0x17DB5C is not decompiled: this routine returns -1
 * there, before +0x3A is written. Verified to compile; not yet run against
 * MAME. */
int Obj_17DA00_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17DA00: movea.l (0x8,A0),A4 */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x16u, rd16(m, a4 + 0x16u));
    wr16(m, obj + 0x1Au, rd16(m, a4 + 0x1Au));
    wr32(m, obj + 0x08u, rd32(m, a4 + 0x08u));
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr16(m, obj + 0x1Cu, 0x0008u);

    /* 0x17DA2E: movea.l #0x17DB54,A1 ; move.w (A1)+,D0 ; move.w D0,(0x40,A0) ;
     * move.l A1,(0x3A,A0) ; jsr A1 on (0x179A5C)[D0*8] (the INIT half) */
    uint32_t a1 = 0x0017DB54u;
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0w);
    wr32(m, obj + 0x3Au, a1);                       /* before the init reads it */
    if (Obj_AnimInit(m, obj, d0w) < 0)
        return -1;

    /* 0x17DA4A: movea.l (0x8,A0),A4 ; D0 = (A4+0x50) + (A0+0x16) ;
     * D1 = (A4+0x54) + (A0+0x1A) ; store to +0x50/+0x54 and +0x14/+0x18 */
    a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    /* 0x17DA6E: D0 = (0x40,A0) ; jsr A1 on (0x179A5C)[D0*8 + 4] (TICK half) */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17DA7E: A1 = (0x10,A0) ; D1 = (A1)+ (sprite rows) ; D0 = (A1)+ (cols) ;
     * asl.w #3 on both ; +0x14 -= D0<<3 ; +0x18 -= D1<<3 */
    uint32_t spr = rd32(m, obj + 0x10u);
    uint16_t rows = rd16(m, spr);
    uint16_t cols = rd16(m, spr + 2u);
    int16_t off_x = (int16_t)(int32_t)(int16_t)(uint16_t)(cols << 3);
    int16_t off_y = (int16_t)(int32_t)(int16_t)(uint16_t)(rows << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)off_x));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)off_y));

    /* 0x17DA92: jsr 0x1282 (draw sprite) */
    Obj_DrawSprite(m, obj);

    /* 0x17DA98: move.l #0x17DAA0,(A0) ; rts. The word after 0x17DAA0 is
     * 0x4A68/0x003E (tst.w (0x3E,A0)) followed by a jsr 0x105EC, i.e. the
     * captured payload of 0x17DAA0; no C helper carries it, so stop here. */
    wr32(m, obj + 0x00u, 0x0017DAA0u);
    return 0;
}
