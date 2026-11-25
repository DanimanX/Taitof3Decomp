#include "obj_11BD48.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"

/* ROM 0x0CC3C6: heading from A0 to A1 (this body is the copy already verified
 * in src/objects/obj_shared.c as Obj_Atan2). */
static uint16_t sub_0CC3C6(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu));
    uint16_t d1;
    d0 = (uint16_t)(d0 - rd16(m, a0 + 0x2Cu));
    d0 = (uint16_t)(d0 - rd16(m, a0 + 0x2Eu));
    d0 = (uint16_t)((int16_t)d0 >> 1);                       /* asr.w #1 */
    d0 = (uint16_t)(d0 + rd16(m, a1 + 0x14u));
    d0 = (uint16_t)(d0 - rd16(m, a0 + 0x14u));

    d1 = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u));
    d1 = (uint16_t)(d1 - rd16(m, a0 + 0x30u));
    d1 = (uint16_t)(d1 - rd16(m, a0 + 0x32u));
    d1 = (uint16_t)((int16_t)d1 >> 1);                       /* asr.w #1 */
    d1 = (uint16_t)(d1 + rd16(m, a1 + 0x18u));
    d1 = (uint16_t)(d1 - rd16(m, a0 + 0x18u));

    return Obj_Atan2(m, d0, d1);                             /* jsr 0x6422 */
}

/* ROM 0x11BD48: one frame of a homing turret. +0x3E steps the +0x3A frame
 * index, +0x20 turns toward the target (A0+0x0C) by the whole error of the
 * heading, then moves along the 0x27E40 table at speed +0x42.
 * 0x0CC3C6 = sub_0CC3C6, 0x0CBE12 = Obj_MoveByAngle, 0x0105EC = Obj_Free. */
int Obj_11BD48_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11BD48 tst.w (0x22,A0) / blt.w 0x11BD86 */
    if ((int16_t)rd16(m, obj + 0x22u) < 0)
        goto release;
    /* 0x11BD50 btst.b #4,(-0x144B,A5) / bne.w 0x11BD86 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto release;

    /* 0x11BD5A move.b (0x3E,A0),D0b / add.b D0b,(0x3A,A0) */
    wr8(m, obj + 0x3Au, (uint8_t)(rd8(m, obj + 0x3Au) + rd8(m, obj + 0x3Eu)));

    /* 0x11BD62 movea.l (0xC,A0),A1 / jsr 0xCC3C6 */
    {
        uint16_t d0 = sub_0CC3C6(m, obj, rd32(m, obj + 0x0Cu));
        /* 0x11BD6C sub.b (0x20,A0),D0b / ext.w / asl.w #4 / add.w D0w,(0x20,A0) */
        uint8_t b = (uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
        int16_t t = (int16_t)(int8_t)b;
        t = (int16_t)(t << 4);
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)t));
    }

    /* 0x11BD78 move.w (0x20,A0),D0w / lsr.w #8 / jsr 0xCBE12 */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));

    /* 0x11BD84 bra.b 0x11BD40: re-arm with itself */
    wr32(m, obj + 0x00u, 0x0011BD48u);
    return 0;

release:
    /* 0x11BD86 move.w #2,(0x6,A0); beq never taken, so +0x00 is not written */
    wr16(m, obj + 0x06u, 0x0002u);
    /* 0x11BD9C */
    Obj_Free(m, obj);
    return 0;
}
