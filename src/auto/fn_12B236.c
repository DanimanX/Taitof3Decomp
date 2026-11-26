/* ROM 0x12B236: ship-attached "body" object tick (A0 = object, A0+0x08 = the
 * ship). Copies the ship's +0x59 flag: zero -> free the object (0x0105EC).
 * Otherwise it re-syncs position from its anchor (A0+0x44, 0x12B3E2), eases
 * its angle +0x20 towards the anchor's (0x12B3F4), moves along it (0x0CBE12),
 * picks the body cell of its movement record (0x12B41E), publishes itself
 * through 0x11B06E and draws itself and the whole chain hanging off +0x44.
 * Then 0x12AB7C handles a hit, and A5+0x30EE bits (9 / 10 / 13 from the MSB)
 * roll the firing gate and spawn the gun/muzzle objects. */
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

/* ROM 0x12B3F4: angle +0x20 eases to the anchor's: D0w = ((anchor - own +
 * A5+0x327A) / 8) + own (divs.w), then A5+0x327A += A5+0x327C. */
static void sub_12B3F4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);                          /* 0x12B3F4 */
    uint16_t d1w = rd16(m, obj + 0x20u);                          /* 0x12B3F8 */
    uint16_t d0w = (uint16_t)(rd16(m, a1 + 0x20u) - d1w);         /* 0x12B3FC/0x12B400 */
    d0w = (uint16_t)(d0w + rd16(m, F3_A5 + 0x327Au));             /* 0x12B402 */
    /* 0x12B406 ext.l / 0x12B408 divs.w #8: quotient in D0w (RNG state clear) */
    {
        int32_t d0l = (int32_t)(int16_t)d0w;
        d0w = (uint16_t)(int16_t)(d0l / 8);
    }
    d0w = (uint16_t)(d0w + d1w);                                  /* 0x12B40C */
    wr16(m, obj + 0x20u, d0w);                                    /* 0x12B40E */
    wr16(m, F3_A5 + 0x327Au,
         (uint16_t)(rd16(m, F3_A5 + 0x327Au) + rd16(m, F3_A5 + 0x327Cu))); /* 0x12B414..0x12B418 */
}

/* ROM 0x12B41E: pick the {4 signed bytes, sprdef.l, flip.b, dx.w, dy.w}
 * cell of the movement record pointed at by +0x3A. D1w gets bit 27 of the
 * index, which makes the cell mirrored. */
static void sub_12B41E(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0w = rd16(m, obj + 0x20u);                          /* 0x12B41E */
    d0w = (uint16_t)(d0w >> 8);                                   /* 0x12B422 lsr.w #8 */
    d0w = (uint16_t)((d0w & 0xFF00u) | (uint8_t)((uint8_t)d0w + 4u)); /* 0x12B424 addq.b #4 */
    d0w = (uint16_t)(d0w >> 3);                                   /* 0x12B426 lsr.w #3 */
    {
        uint16_t d1w = (uint16_t)(d0w & 0x0010u) ? 0xFFFFu : 0x0000u; /* 0x12B428 bfexts {27:1} */
        uint32_t a2, a1;
        d0w = (uint16_t)(d0w & 0x000Fu);                          /* 0x12B42C andi.w #0xf */
        a1 = rd32(m, obj + 0x3Au);                                /* 0x12B430 */
        a2 = rd32(m, a1);                                         /* 0x12B434 movea.l (A1)+,A2 */
        a1 += 4u;
        a1 = a1 + (uint32_t)d0w * 8u;                             /* 0x12B436 lea (A1,D0w*8) */
        a2 = a2 + (uint32_t)d0w * 4u;                             /* 0x12B43A lea (A2,D0w*4) */
        wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a2));     /* 0x12B43E..0x12B442 */
        a2 += 1u;
        wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a2));     /* 0x12B446..0x12B44A */
        a2 += 1u;
        wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a2));     /* 0x12B44E..0x12B452 */
        a2 += 1u;
        wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a2));     /* 0x12B456..0x12B45A */
        wr32(m, obj + 0x10u, rd32(m, a1));                        /* 0x12B45E */
        a1 += 4u;
        wr8(m, obj + 0x1Cu, rd8(m, a1));                          /* 0x12B462 */
        a1 += 1u;
        wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1));     /* 0x12B466..0x12B46A */
        a1 += 1u;
        wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1));     /* 0x12B46E..0x12B472 */
        if (d1w != 0u) {                                          /* 0x12B476 beq 0x12B48A */
            wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x03u)); /* 0x12B47C */
            wr16(m, obj + 0x50u, (uint16_t)(-(uint16_t)rd16(m, obj + 0x50u)));
            wr16(m, obj + 0x52u, (uint16_t)(-(uint16_t)rd16(m, obj + 0x52u)));
        }
    }
}

/* ROM 0x12B48C: offset the sprite by the cell's {dy.w, dx.w} scaled by 8 */
static void sub_12B48C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);                           /* 0x12B48C */
    uint16_t d0w = rd16(m, a1);                                   /* 0x12B490 */
    a1 += 2u;
    d0w = (uint16_t)(d0w << 3);                                   /* 0x12B492 */
    d0w = (uint16_t)(d0w - rd16(m, obj + 0x52u));                 /* 0x12B494 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w)); /* 0x12B498 */
    d0w = rd16(m, a1);                                            /* 0x12B49C */
    d0w = (uint16_t)(d0w << 3);                                   /* 0x12B49E */
    d0w = (uint16_t)(d0w - rd16(m, obj + 0x50u));                 /* 0x12B4A0 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w)); /* 0x12B4A4 */
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

/* ROM 0x12AB7C: a hit was latched in +0x36 (and +0x35 != 2): charge the
 * victim's +0x48 to the hitter, give it a random +0x4C, mark it hit and play
 * sound 0x0F. The victim's style +0x22 must not be 0, 4, 5 or 6. */
static void sub_12AB7C(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x36u);                           /* 0x12AB80 */
    if (d0 == 0u)                                                 /* 0x12AB84 */
        return;
    wr32(m, obj + 0x36u, 0u);                                     /* 0x12AB88 */
    if (rd8(m, obj + 0x35u) == 2u)                                /* 0x12AB8C */
        return;
    {
        uint32_t a1 = d0;
        uint8_t style = rd8(m, a1 + 0x22u);
        if (style == 0u || style == 4u || style == 5u || style == 6u) /* 0x12AB98..0x12ABBC */
            return;
        {
            int32_t v = (int32_t)rd32(m, a1 + 0x48u);             /* 0x12ABC0 */
            if (v <= 0)                                           /* 0x12ABC4 ble */
                return;
            v = -v;                                               /* 0x12ABC8 neg.l */
            v >>= 3;                                              /* 0x12ABCA asr.l #3 */
            wr32(m, a1 + 0x48u, (uint32_t)v);                     /* 0x12ABCC */
            wr32(m, a1 + 0x4Cu, Rng_NextMem(m));                  /* 0x12ABD0 jsr 0x1D8A; ext.l */
            wr8(m, a1 + 0x1Cu, (uint8_t)(rd8(m, a1 + 0x1Cu) | 0x01u)); /* 0x12ABDC bset */
            Sound_PlayHooked(m, 0x000Fu);                         /* 0x12ABE2 */
        }
    }
}

int Fn_12B236(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x12B236 */
    uint8_t flag = rd8(m, a1 + 0x59u);                   /* 0x12B23A */
    wr8(m, obj + 0x59u, flag);                           /* 0x12B23E */
    if (flag != 0u)                                      /* 0x12B240 bne 0x12B378 */
        goto free_obj;

    sub_12B3E2(m, obj);                                  /* 0x12B244 */
    sub_12B3F4(m, obj);                                  /* 0x12B248 */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8)); /* 0x12B24C */
    sub_12B41E(m, obj);                                  /* 0x12B252 */

    wr8(m, obj + 0x35u, 0u);                             /* 0x12B256 sf */
    Obj_11B06E_Tick(m, obj);                             /* 0x12B25A */
    a1 = rd32(m, obj + 0x08u);                           /* 0x12B260 */
    a1 = rd32(m, a1 + 0x08u);                            /* 0x12B264 */
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x1Du));             /* 0x12B268 */

    /* 0x12B26E..0x12B288: draw the object and every anchor hanging off
     * +0x44 (the ROM pushes A0 and walks A0 = (0x44,A0) until zero) */
    {
        uint32_t owner = obj;
        uint32_t a0 = obj;
        for (;;) {
            sub_12B48C(m, a0);                           /* 0x12B272 */
            Obj_DrawSprite(m, a0);                       /* 0x12B276 */
            a0 = rd32(m, a0 + 0x44u);                    /* 0x12B27C */
            if (a0 == 0u)
                break;                                   /* 0x12B280 beq 0x12B288 */
        }
        obj = owner;                                     /* 0x12B288 pop A0 */
    }

    sub_12AB7C(m, obj);                                  /* 0x12B28C */

    /* 0x12B290: bftst (0x30EE,A5){0x9:0x1} -- bit 6 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) == 0u)
        goto after_fire;                                 /* 0x12B296 beq 0x12B328 */
    if (sub_0CC2BC(m, 0x001Au) != 0u)                    /* 0x12B29A */
        goto after_fire;                                 /* 0x12B2A6 bne 0x12B328 */

    /* 0x12B2AA: three gun objects 0x0CCDB6 (group 9, 2 groups). The arg is
     * +0x20. (0x12B2B4..0x12B2CA builds a byte pair from the record, but
     * 0x12B2CC reloads D1 with move.w (0x20,A0) before the push.) */
    {
        uint32_t d1 = (uint32_t)rd16(m, obj + 0x20u);    /* 0x12B2CC move.w */
        d1 &= 0xFFFFFF00u;                               /* 0x12B2D0 sf D1b */
        (void)Obj_Spawn(m, obj, 0x000CCDB6u, 9u, 2u, d1);            /* 0x12B2E2 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + 0x1000u);
        (void)Obj_Spawn(m, obj, 0x000CCDB6u, 9u, 2u, d1);            /* 0x12B300 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 - 0x2000u);
        (void)Obj_Spawn(m, obj, 0x000CCDB6u, 9u, 2u, d1);            /* 0x12B31E */
    }

after_fire:
    /* 0x12B328: bfclr (0x30EE,A5){0xA:0x1} -- bit 5 */
    {
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        if ((b & 0x20u) != 0u) {
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(b & (uint8_t)~0x20u));
            (void)Obj_Spawn(m, obj, 0x0012BB96u, 5u, 2u, 0u);        /* 0x12B344 */
        }
    }

    /* 0x12B34E: bftst (0x30EE,A5){0xD:0x1} -- bit 2 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x04u) != 0u) {
        uint32_t arg = rd32(m, obj + 0x3Au);             /* 0x12B358 */
        (void)Obj_Spawn(m, obj, 0x0012C126u, 5u, 2u, arg);           /* 0x12B36A */
    }

    /* 0x12B374 bra.w 0x12B22E */
    wr32(m, obj + 0x00u, 0x0012B236u);
    return 0;

free_obj:
    /* 0x12B378 */
    Obj_Free(m, obj);
    return 0;                                            /* 0x12B384 rts */
}
