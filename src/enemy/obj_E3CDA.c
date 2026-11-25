#include "obj_E3CDA.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "obj_DBDA2.h"

/* ROM 0x0E3CDA: same shape as the 0x102FE6 enemy bullet. During the boss
 * intro (A5+0x1D58 bit1) with +0x14 in [-0x0A,0x11C] it runs the 0x0CBB2C
 * explosion tick. Otherwise a spent timer (+0x24 <= 0) or A5-0x144B bit5
 * spawns one 0x0DBDA2 shot in group 9 (count 2, arg 0) and starts dying;
 * alive it integrates +0x3A/+0x3E with the scroll velocity, flags itself
 * offscreen, syncs +0x14/+0x18, dies when the terrain probe at
 * (+0x14+8,+0x18+8) hits, else joins the hazard list and draws. */
int Obj_E3CDA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0E3CDA: btst.b #1,(0x1D58,A5) ; beq 0x0E3CF8 ;
     * cmpi.w #-0xA,(0x14,A0) ; blt ; cmpi.w #0x11C ; bgt ; jmp 0x0CBB2C */
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* 0x0E3CF8: tst.l (0x24,A0) ; ble.w 0x0E3D68 ;
     * btst.b #5,(A5-0x144B) ; bne.w 0x0E3D68 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u)) {
        /* 0x0E3D68: pea 0 ; move.w #2,-(SP) ; move.w #9,-(SP) ;
         * move.l #0xDBDA2,-(SP) ; jsr 0x1057C ; lea (0xC,SP),SP ;
         * bra.w 0x0CEE74 */
        (void)Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0E3D0A: move.l (0x50,A0),D0 ; sub.l (0x4224,A5),D0 ; add.l D0,(0x3A,A0) */
    uint32_t dx = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + dx);
    /* 0x0E3D16: move.l (0x54,A0),D0 ; sub.l (0x4228,A5),D0 ; add.l D0,(0x3E,A0) */
    uint32_t dy = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + dy);

    /* 0x0E3D22: jsr 0x0CF85A */
    Obj_FlagOffscreen(m, obj);

    /* 0x0E3D28: btst.b #3,(0x7,A0) ; bne.w 0x0CEE74 */
    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0E3D34: move.l (0x3A,A0),(0x14,A0) ; move.l (0x3E,A0),(0x18,A0) */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    /* 0x0E3D40: move.w (0x14,A0),D0 ; addq.w #8,D0 ; move.w (0x18,A0),D1 ;
     * addq.w #8,D1 ; jsr 0x0C6632 ; tst.b D0 ; bne.w 0x0CEE74 */
    int16_t tx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + 8u);
    int16_t ty = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + 8u);
    if (Terrain_TestPoint(m, tx, ty) != 0) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0E3D5A: jsr 0x0D0DCC ; jsr 0x001282 ; rts */
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
