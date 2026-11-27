/* ROM 0x0F922C: enemy object tick (A0 = obj) — a scripted stage enemy whose
 * first pass converts it from "intro" to "script" state.
 *
 * +0x06 != 2 (intro): add to the hazard lists (0x0D0DC2); if HP +0x24 is gone,
 * install the anim record 0xF93E0, set +0x06 = 2, play hooked sound 0x33 and
 * spawn the two pods 0xF9428 (offset +6/+0x16) and 0xF9108 (+0x11/+0x26) in
 * group 9.
 * Then: die (0xCEE74) when A5-0x144B bit 5 or +0x07 bit 3 is set; else apply
 * the layer scroll (A5+0x4224/0x4228) to (+0x3A, +0x3E), copy it into
 * (+0x14, +0x18), dispatch +0x06 through the table at 0xF9360 ([0]=0xF936C,
 * [1]=[2]=bare rts), step the script (0x0D0690) and, while the script timer
 * +0x4C is running, add the 0x0CF898 orbit offset and draw (0x0CBE34 +
 * 0x001282). When +0x4C reaches 0 it nudges the object (-9, +0x14), spawns
 * 0xF8C1E in group 5 and re-initialises the object as the 0x0F9AD4 type
 * (mover 0xF9D9E, script 0xF9DD6, anim 0xF9F86, +0x0C = 0x384, +0x06 = 1). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

/* ROM 0x0D0678 (Obj_SetAnim) also leaves D0 = the record's frame word and
 * A1 = the record + 8; both are observable at rts on the 0xF9A6C path. */
static void fn0f922c_setanim(const f3mem_t *m, uint32_t obj, uint32_t a1,
                             uint32_t *d0_io, uint32_t *a1_io)
{
    Obj_SetAnim(m, obj, a1);
    *d0_io = (*d0_io & 0xFFFF0000u) | rd16(m, a1 + 2u);   /* move.w (a1)+,d0 */
    *a1_io = a1 + 8u;
}

/* ROM 0x0D0690 (Obj_AnimStep) with the D0/A1 it leaves behind: with a running
 * timer nothing changes; when it expires D0 = the script's frame word and
 * A1 = the script cursor past the sprdef long. */
static void fn0f922c_animstep(const f3mem_t *m, uint32_t obj,
                              uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, t);
    if (t != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;            /* 0x0D069E adda.w #0xC */
    wr32(m, obj + 0x48u, a1);
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0w < 0) {
        uint16_t back = rd16(m, a1);                       /* 0x0D06C0 tst.w (a1) */
        if (back == 0u) {                                  /* 0x0D06DE rts */
            *d0_io = (*d0_io & 0xFFFF0000u) | d0w;
            *a1_io = a1;
            return;
        }
        a1 -= back;                                        /* 0x0D06C6 suba.w */
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0w);                         /* 0x0D06AC */
    }
    d0w = rd16(m, a1);                                     /* 0x0D06B0 / 0x0D06D0 */
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *d0_io = (*d0_io & 0xFFFF0000u) | d0w;
    *a1_io = a1;
}

/* ROM 0x001282 (Obj_DrawSprite) leaves A1 = the sprite cursor and D0/D1 as
 * the emitter returns them. */
static void fn0f922c_draw(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0F922C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = r->a[1];

    /* 0x0F922C: one-shot intro pass */
    if (rd8(m, obj + 0x06u) != 2u) {
        Obj_0D0DC2(m, obj);                                /* 0x0F9236 */
        a1 = rd32(m, F3_A5 + 0x0906u);                     /* A1 left by 0x0D0DC2 */
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {          /* 0x0F923C */
            fn0f922c_setanim(m, obj, 0x000F93E0u, &d0, &a1); /* 0x0F9244 */
            wr8(m, obj + 0x06u, 2u);                       /* 0x0F924E */
            Sound_PlayHooked(m, 0x0033u);                  /* 0x0F9254 */
            d0 = Obj_Spawn(m, obj, 0x000F9428u, 9u, 2u, 0u); /* 0x0F9268 */
            if ((int32_t)d0 >= 0) {                        /* 0x0F9284 */
                a1 = d0;
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au)); /* 0x0F928C */
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0006u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu)); /* 0x0F9296 */
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0016u));
                d0 = Obj_Spawn(m, obj, 0x000F9108u, 9u, 2u, 0u); /* 0x0F92A2 */
                if ((int32_t)d0 >= 0) {                    /* 0x0F92BE */
                    a1 = d0;
                    wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au)); /* 0x0F92C6 */
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0011u));
                    wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu)); /* 0x0F92D2 */
                    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0026u));
                }
            }
        }
    }

    /* 0x0F92DE: stage-clear / bomb-clear death */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);                            /* 0x0F92E4 -> 0x0CEE74 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    d0 = rd32(m, F3_A5 + 0x4224u);                         /* 0x0F92EA */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);       /* 0x0F92EE */
    d0 = rd32(m, F3_A5 + 0x4228u);                         /* 0x0F92F2 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);       /* 0x0F92F6 */
    Obj_FlagOffscreen(m, obj);                             /* 0x0F92FA */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {             /* 0x0F9300 */
        Obj_StartDying(m, obj);                            /* 0x0F9306 -> 0x0CEE74 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));            /* 0x0F930C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));            /* 0x0F9312 */

    /* 0x0F9318..0x0F9324: state dispatch through the 3-entry table 0xF9360
     * ([0]=0x0F936C, [1] and [2]=0x0F938E, a bare rts). */
    d0 = (d0 & 0xFFFF0000u) | rd8(m, obj + 0x06u);
    if ((uint16_t)d0 == 0u) {
        Obj_ArmOnScreen(m, obj);                           /* 0x0F936C */
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {         /* 0x0F9372 btst #7 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
            wr8(m, obj + 0x06u, 1u);
        }
    }

    fn0f922c_animstep(m, obj, &d0, &a1);                   /* 0x0F9326 */

    if (rd16(m, obj + 0x4Cu) == 0u) {
        /* 0x0F9350: nudge, then start the 0x0F9AD4 behaviour (0x0F9A6C) */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 9u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x14u));
        d0 = Obj_Spawn(m, obj, 0x000F8C1Eu, 5u, 2u, 0u);   /* 0x0F9A6C */
        if ((int32_t)d0 < 0) {                             /* 0x0F9A88 */
            Obj_StartDying(m, obj);                        /* -> 0x0CEE74 */
            r->d[0] = d0;
            r->a[1] = a1;
            return 0;
        }
        a1 = 0x000F9D9Eu;
        Obj_InitMover(m, obj, a1);                         /* 0x0F9A90 */
        a1 = 0x000F9DD6u;
        Obj_SetScript(m, obj, a1);                         /* 0x0F9A9A */
        fn0f922c_setanim(m, obj, 0x000F9F86u, &d0, &a1);   /* 0x0F9AA4 */
        wr8(m, obj + 0x4Fu, 0x7Cu);                        /* 0x0F9AAE */
        wr8(m, obj + 0x1Du, 0x7Cu);                        /* 0x0F9AB4 */
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
        wr16(m, obj + 0x0Cu, 0x0384u);                     /* 0x0F9AC0 */
        wr8(m, obj + 0x06u, 1u);                           /* 0x0F9AC6 */
        wr32(m, obj, 0x000F9AD4u);                         /* 0x0F9ACC */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;                                          /* 0x0F9AD2 rts */
    }

    /* 0x0F9334: orbit offset then draw */
    {
        uint32_t od0 = 0u, od1 = 0u;
        if (Obj_0CF898(m, obj, &od0, &od1) < 0)
            return -1;
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + od0);  /* 0x0F933A */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + od1);  /* 0x0F933E */
    }
    Obj_HitFlash(m, obj);                                  /* 0x0F9342 */
    fn0f922c_draw(m, obj, r);                              /* 0x0F9348 */
    r->a[0] = obj;
    return 0;                                              /* 0x0F934E rts */
}
