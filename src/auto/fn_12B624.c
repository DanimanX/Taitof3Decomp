/* ROM 0x12B624: "pod" object tick (A0 = object, A0+0x08 = the ship it rides
 * on). It copies the ship's live flag +0x59: reached zero -> free the pod
 * (0x0105EC). Otherwise it mirrors the ship's position/angle/speed, drifts
 * forward with 0x0CBE12 and (when A5+0x30EE bit from the MSB holds) rolls a
 * 0x0CC2BC probability to fire two 0x0CCD5C bullets; then the animation
 * table 0x12B7AC is stepped and the sprite is anchored by the offsets in
 * +0x10. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"
#include "../enemy/obj_11B06E.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x12B3E2: copy the anchor object's position into the object */
static void sub_12B3E2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
}

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % d1w);
}

int Fn_12B624(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x12B624 */
    uint8_t flag = rd8(m, a1 + 0x59u);                   /* 0x12B628 */
    wr8(m, obj + 0x59u, flag);                           /* 0x12B62C */
    if (flag != 0u)
        goto arm_next;                                   /* 0x12B62E bne 0x12B792 */

    /* 0x12B632: re-sync the position from the anchor object (A0+0x44) */
    sub_12B3E2(m, obj);                                  /* bsr.w 0x12B3E2 */

    /* 0x12B636: the pod's angle = the ship's angle + 0x20 (but its speed
     * field is 0x30 lower), both turning with A5+0x327E */
    a1 = rd32(m, obj + 0x08u);                           /* 0x12B636 */
    wr16(m, a1 + 0x42u, rd16(m, obj + 0x22u));           /* 0x12B63A */
    wr16(m, obj + 0x42u, rd16(m, obj + 0x22u));          /* 0x12B640 */
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 0x30u)); /* 0x12B646 */
    {
        uint16_t d0w = rd16(m, F3_A5 + 0x327Eu);         /* 0x12B64C */
        wr16(m, a1 + 0x42u, (uint16_t)(rd16(m, a1 + 0x42u) + d0w)); /* 0x12B650 */
        d0w = (uint16_t)(d0w >> 1);                      /* 0x12B654 asr.w #1 */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + d0w)); /* 0x12B656 */
    }

    /* 0x12B65A: move with the angle in the HIGH byte of the ship's +0x20 */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, a1 + 0x20u) >> 8)); /* 0x12B660 */

    /* 0x12B666: bftst (0x30EE,A5){0x1:0x1} -- bit 6 of the byte */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) == 0u)
        goto no_fire;                                    /* 0x12B66C beq 0x12B74E */

    if ((int16_t)rd16(m, obj + 0x3Au) < 0)               /* 0x12B670 blt 0x12B716 */
        goto anim;

    /* 0x12B678: one roll per live pod; D2w = 4 * the ship's +0x22 is taken
     * out of the on-screen budget A5+0x878 around the call */
    {
        uint16_t d2w = (uint16_t)(rd16(m, a1 + 0x22u) << 2);
        uint16_t d0w;
        wr16(m, F3_A5 + 0x0878u, (uint16_t)(rd16(m, F3_A5 + 0x0878u) - d2w));
        d0w = sub_0CC2BC(m, 0x001Eu);                    /* 0x12B686 jsr 0x0CC2BC */
        wr16(m, F3_A5 + 0x0878u, (uint16_t)(rd16(m, F3_A5 + 0x0878u) + d2w));
        /* tst.w at 0x12B690 is the ROM's own check of D0w; the real test is
         * 0x12B696 after the restore. */
        if (d0w != 0u)
            goto anim;                                   /* 0x12B698 bne 0x12B716 */
    }

    /* 0x12B69C..0x12B6F4: three bullets, fn 0x0CCD5C, group 9, 2 groups,
     * args 0x02108000 / -0x7000 / 0x7000 */
    (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x02108000u); /* 0x12B69C */
    (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0xFFFF9000u); /* 0x12B6BC */
    (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x00007000u); /* 0x12B6DC */

    /* 0x12B6F8: restart the firing pose, then sound 0x3B */
    wr16(m, obj + 0x3Au, 2u);
    wr16(m, obj + 0x3Eu, 0u);
    Sound_PlayHooked(m, 0x003Bu);                        /* 0x12B702/js r0x13F238 + 0x0039D8 */

anim:
    /* 0x12B716: one step of the {dur.w, sprdef.l} table at 0x12B7AC */
    Obj_AnimFrameStep(m, obj, 0x0012B7ACu);              /* lea (0x94,PC),A1 */
    if ((int16_t)rd16(m, obj + 0x3Au) == -6 &&           /* 0x12B722 */
        (int16_t)rd16(m, obj + 0x3Eu) == 10)             /* 0x12B72C */
        Sound_PlayHooked(m, 0x001Bu);                    /* 0x12B736 */
    goto common;                                         /* 0x12B74A bra 0x12B760 */

no_fire:
    /* 0x12B74E: hidden pose */
    wr16(m, obj + 0x3Au, (uint16_t)-0x0C);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x10u, 0x00042EF8u);

common:
    /* 0x12B760 */
    wr8(m, obj + 0x35u, 0u);                             /* sf (0x35,A0) */
    Obj_11B06E_Tick(m, obj);                             /* 0x12B764 jsr 0x11B06E */
    a1 = rd32(m, obj + 0x08u);                           /* 0x12B76A */
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x1Du));             /* 0x12B76E */
    a1 = rd32(m, obj + 0x10u);                           /* 0x12B774 */
    {
        uint16_t d0w = rd16(m, a1);                      /* 0x12B778 */
        a1 += 2u;
        d0w = (uint16_t)(d0w << 3);                      /* 0x12B77A */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w)); /* 0x12B77C */
        d0w = rd16(m, a1);                               /* 0x12B780 */
        d0w = (uint16_t)(d0w << 3);                      /* 0x12B782 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w)); /* 0x12B784 */
    }
    Obj_DrawSprite(m, obj);                              /* 0x12B788 */
    /* 0x12B78E bra.w 0x12B61C: re-arm this tick */
    wr32(m, obj + 0x00u, 0x0012B624u);
    return 0;

arm_next:
    /* 0x12B792: the ship is gone: free the pod */
    Obj_Free(m, obj);                                    /* 0x12B794 jsr 0x0105EC */
    return 0;                                            /* 0x12B79E rts */
}
