#include "obj_17D72C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17D72C: the 0x17D66E object's flight step. While +0x24 is still
 * positive it runs the 0x44 behaviour script (0x178982) and, once the object
 * has left the 0x184A08 region, appends it to the hazard and sort lists,
 * advances the animation (swapping the angle in and out of +0x0C), draws and
 * spawns a 0x17A42A child every third frame. It becomes the 0x0CBB2C
 * explosion, or parks for another frame, depending on the alternate sound
 * mode. Once +0x24 has run out it hands over to 0x17C73E: the behaviour's
 * hit info is credited to the target and the object switches to the 0x81C
 * animation and sound 0x35. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* cmp2.w (A_base)/(A_base+2) against D0w: true when lo < D0w < hi (bcs to the
 * out-of-range handler). */
static int in_box(const f3mem_t *m, uint32_t base, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, base), hi = (int16_t)rd16(m, base + 2u);
    int16_t x = (int16_t)v;
    return x > lo && x < hi;
}

/* ROM 0x17AFD4: steer toward the object at +0x4C. Reads the stack frame:
 * (0x8,A6) = target, (0xC,A6)/(0xE,A6) = x/y, (0x10,A6) = the turn step.
 * The heading error's sign picks the direction of the step, which is added to
 * the byte angle +0x20. */
static void sub_17AFD4(const f3mem_t *m, uint32_t obj, uint32_t tgt,
                       uint16_t x, uint16_t y, uint16_t step)
{
    if ((int32_t)tgt <= 0)
        return;
    uint32_t a1 = tgt;
    (void)a1;
    /* 0x17AFF0..0x17B000: the target's +0x14/+0x18 (+0x18/+0x10) minus the
     * pushed x/y, then jsr 0x6422. obj_shared's Obj_AtanByte is the wrapper
     * that feeds 0x6422 its (first, second) axis in register order; the
     * libc-side Obj_Atan2 takes them the other way round. */
    uint16_t d0 = Obj_AtanByte(m,
        (int16_t)(uint16_t)((uint16_t)(rd16(m, a1 + 0x14u) + 0x0018u) - x),
        (int16_t)(uint16_t)((uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u) - y));

    /* 0x17B00A: sub.b (0x20,A0),D0b */
    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    uint16_t d1 = step;
    if (diff != 0) {
        if ((int8_t)diff < 0)
            d1 = (uint16_t)(0u - d1);
    } else {
        d1 = 0u;
    }
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d1));
}

/* ROM 0x17B02E: the object the target field (+0x4C) points at, or -1.
 * +0x35 == 1 forwards through the pointer at +0x36 (its +0x8 word), == 2 is
 * the pointer itself. */
static int32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u)
        return (int32_t)rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (mode == 2u)
        return (int32_t)(rd32(m, obj + 0x36u) & 0x00FFFFFFu);
    return -1;
}

/* ROM 0x17C73E: the +0x24 credit. The behaviour's +0x62 word is taken as a
 * 16-bit field offset and added to the target object's +0x48, then the 0x81C
 * animation ({9, frames 0x17C81E}) replaces the current one and sound 0x35 is
 * played before rejoining the 0x17C614 body. */
static void lab_17C73E(const f3mem_t *m, uint32_t obj)
{
    /* 0x17C73E: D1 = bfextu (0x62,A0){0:16} */
    uint32_t d1 = rd16(m, obj + 0x62u);

    /* 0x17C744: jsr 0x17B02E; tst.l D0; blt; add.l D1,(0x48,A1) */
    {
        int32_t tgt = sub_17B02E(m, obj);
        if (tgt >= 0)
            wr32(m, (uint32_t)tgt + 0x48u, rd32(m, (uint32_t)tgt + 0x48u) + d1);
    }

    /* 0x17C756: animation record 0x17C81C -- index 9 -> 0x179600 */
    {
        uint32_t a1 = 0x0017C81Cu;
        uint16_t d0w = rd16(m, a1); a1 += 2u;
        wr16(m, obj + 0x40u, d0w);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, d0w) < 0)
            return;                                  /* nothing else applied yet */
    }

    wr8(m, obj + 0x1Du, 0u);                         /* 0x17C772 clr.b */

    {
        int16_t cmd = 0x35;                          /* 0x17C776 */
        if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
            cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
        Sound_PlayCmd(m, cmd);
    }

    /* 0x17C78A: bra.w 0x17C614 -- script step, centre, animate, park */
    if (Obj_Script10(m, obj) < 0)
        return;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return;
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1c = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0c = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0c));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1c));
    }
    /* 0x17C64A: tst.w (0x3E,A0); blt.w 0x17C78E */
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        lab_17C78E(m, obj);
        return;
    }
    Obj_DrawSprite(m, obj);                          /* 0x17C652 */
    wr32(m, obj + 0x00u, 0x0017C660u);               /* 0x17C658 */
}

/* ROM 0x17D72C */
int Obj_17D72C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D72C: btst.b #4,(-0x144C,A5); beq; clr.l (0x24,A0) */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        wr32(m, obj + 0x24u, 0u);

    /* 0x17D73A: tst.l (0x24,A0); ble.w 0x17C73E */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        lab_17C73E(m, obj);
        return 0;
    }

    /* 0x17D742: jsr 0x178982 */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* 0x17D748..0x17D764: cmp2.w (0x184A08)/(0x184A0C); bcs 0x17C78E */
    if (!in_box(m, 0x00184A08u, rd16(m, obj + 0x50u)) ||
        !in_box(m, 0x00184A0Cu, rd16(m, obj + 0x54u))) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17D768: append to the hazard list (A5+0x906) and the sort list
     * (A5+0x8FE) */
    {
        uint32_t p = rd32(m, F3_A5 + 0x906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x906u, p + 4u);
    }
    {
        uint32_t p = rd32(m, F3_A5 + 0x8FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x8FEu, p + 4u);
    }

    /* 0x17D77C: centre = the position */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17D788: swap the angle (+0x20) and the angle record (+0x0C) */
    {
        uint16_t d0w = rd16(m, obj + 0x20u);
        wr16(m, obj + 0x20u, rd16(m, obj + 0x0Cu));
        wr16(m, obj + 0x0Cu, d0w);
    }

    /* 0x17D796: cmpi.l #0x17D8E2,(0x44,A0); bne; push 0x400, y, x, target
     *          -> jsr 0x17AFD4 (the steering entry of the behaviour) */
    if (rd32(m, obj + 0x44u) == 0x0017D8E2u) {
        sub_17AFD4(m, obj, rd32(m, obj + 0x4Cu),
                   rd16(m, obj + 0x50u), rd16(m, obj + 0x54u), 0x0400u);
    }

    /* 0x17D7BC: the 0x179A5C TICK half for the index in +0x40 */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17D7CC: centre -= the frame size (<<3) */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1c = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0c = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0c));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1c));
    }

    /* 0x17D7E0: swap the angle back */
    {
        uint16_t d0w = rd16(m, obj + 0x20u);
        wr16(m, obj + 0x20u, rd16(m, obj + 0x0Cu));
        wr16(m, obj + 0x0Cu, d0w);
    }

    Obj_DrawSprite(m, obj);                          /* 0x17D7EE jsr 0x1282 */

    /* 0x17D7F4: every third frame (unless the behaviour is 0x17D8D8) spawn a
     * 0x17A42A child with arg 0, group 5, two groups */
    if (rd32(m, obj + 0x44u) != 0x0017D8D8u) {
        wr16(m, obj + 0x64u, (uint16_t)(rd16(m, obj + 0x64u) + 1u));
        if ((int16_t)rd16(m, obj + 0x64u) >= 3) {
            Obj_Spawn(m, obj, 0x0017A42Au, 5u, 2u, 0u);
            wr16(m, obj + 0x64u, 0u);
        }
    }

    /* 0x17D830: btst.b #1,(0x1D58,A5); beq -> 0x17D852 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        /* 0x17D838: (0x1D50,A5) += the +0x62 hit offset; +0x1D = +0x22;
         *          jmp 0x0CBB2C */
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + rd16(m, obj + 0x62u));
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
        Obj_ExplodeTick(m, obj);
        return 0;
    }

    /* 0x17D852: bra.w 0x17D724 -> move.l #0x17D72C,(A0) */
    wr32(m, obj + 0x00u, 0x0017D72Cu);
    return 0;
}
