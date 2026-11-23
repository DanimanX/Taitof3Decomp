#include "obj_1250E4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../enemy/obj_11B06E.h"
#include "../enemy/obj_11B09A.h"
#include "../video/sprite.h"

/* ---- local copies of helpers that live in other routines' files -------- */

/* ROM 0x11AE62: drain A0+0x24 into A5+0x30DA; when the counter hits 0 credit
 * A1's +0x48 clock with D0 (0x186A0 here). D1w = 1 and D2 = (0x24,A0) are the
 * caller's registers, so the scaling is fixed at one step. */
static void sub_11AE62(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)       /* 0x11AE62 */
        return;
    int32_t d2 = (int32_t)rd32(m, obj + 0x24u);       /* 0x11AE6A */
    if (d2 == 0)
        return;
    wr32(m, obj + 0x24u, 0u);                         /* 0x11AE72 */

    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u) {
        d2 = (int32_t)((uint32_t)d2 >> 1);            /* 0x11AE8E asr.l D1, D1 = 1 */
        if (k != 1u && k != 5u)
            d2 = (int32_t)((uint32_t)d2 >> 1);        /* 0x11AEA4 */
    }
    wr32(m, F3_A5 + 0x30DAu, rd32(m, F3_A5 + 0x30DAu) + (uint32_t)d2);
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0)
        return;
    /* 0x11AEB2: jsr 0x0CBDDE */
    a1 = rd32(m, obj + 0x36u);
    uint8_t s = rd8(m, obj + 0x35u);
    if (s != 2u && s == 1u)
        a1 = rd32(m, a1 + 0x08u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x000186A0u);   /* 0x11AEB8 */
}

/* ROM 0x0CBB0C: park the warp target when bit 2 of A5+0x1D58 is set */
static void sub_0CBB0C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x04u) {
        wr8(m, obj + 0x35u, 0x01u);
        wr32(m, obj + 0x36u, rd32(m, F3_A5 + 0x1D54u));
        wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) - 0x4000u);
    }
}

/* ROM 0x0CC2BC: roll D0w against the on-screen scroll budget */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint32_t d1 = (uint32_t)(uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    d1 = (uint32_t)(uint16_t)d0w * d1;
    d1 = (d1 << 8) & 0xFFFFFFFFu;
    d1 = (d1 << 16) | (d1 >> 16);                       /* swap */
    uint16_t thr = (uint16_t)d1;
    if (thr == 0u)
        thr = 1u;
    uint32_t num = rd16(m, F3_A5 + 0x0878u);
    uint32_t d0 = ((num % thr) << 16) | ((num / thr) & 0xFFFFu);
    d0 = (d0 << 16) | (d0 >> 16);                       /* swap */
    return (uint16_t)d0;
}

/* ROM 0x0CC33A: angle from (-D0,-D1) to the scroll block's box centre */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0w, uint16_t d1w)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
    if (a1 == 0u)
        return 0x0080u;                                 /* 0x0CC342 */

    d0w = (uint16_t)(0u - d0w);
    uint16_t t = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu));
    d0w = (uint16_t)(d0w + (uint16_t)((int16_t)t >> 1));
    d0w = (uint16_t)(d0w + rd16(m, a1 + 0x14u));
    d0w = (uint16_t)(d0w - rd16(m, obj + 0x14u));

    d1w = (uint16_t)(0u - d1w);
    t = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u));
    d1w = (uint16_t)(d1w + (uint16_t)((int16_t)t >> 1));
    d1w = (uint16_t)(d1w + rd16(m, a1 + 0x18u));
    d1w = (uint16_t)(d1w - rd16(m, obj + 0x18u));

    return Obj_Atan2(m, d0w, d1w);                      /* 0x0CC37C */
}

/* 0x1250xx: every fan is `pea arg ; pea 9 ; pea 2 ; pea fn ; jsr 0x1057C` */
static void fan(const f3mem_t *m, uint32_t obj, uint32_t fn, uint32_t arg)
{
    (void)Obj_Spawn(m, obj, fn, 9u, 2u, arg);
}

/* ROM 0x1250E4: phase-1 mid-boss. Two gates (the parent's +0x59 and the
 * A5+0x30EE bit field) choose between parking the first record and walking
 * the +0xC frame script; the tail always publishes to the hazard lists, draws,
 * and then fires the five fan volleys selected by A5+0x30EE. */
int Obj_1250E4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1250E4: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x1253A2 */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        if (rd8(m, a1 + 0x59u) != 0u) {
            Obj_Free(m, obj);                           /* 0x1253A2 -> 0x105EC */
            return 0;
        }
    }

    sub_11AE62(m, obj);                                 /* 0x1250F0 jsr 0x11AE62 */

    wr8(m, obj + 0x34u, 0x10u);                         /* 0x1250FE sf + bset #4 */

    /* 0x125108: bftst (0x30EE,A5){0:1} ; beq 0x12514A */
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x8000u) == 0u) {
        wr32(m, obj + 0x10u, 0x00039960u);              /* 0x12514A */
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        goto tail_12515A;
    }

    /* 0x125112: A1 = (0xC,A0) ; jsr 0x0CBD24 ; tst.w (A1,D0w) */
    {
        int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);
        if (timer > 0) {
            wr16(m, obj + 0x3Eu, (uint16_t)(timer - 1));
            goto tail_12515A;                           /* 0x125120 bgt */
        }
        if (timer < 0) {
            wr16(m, F3_A5 + 0x30EEu,
                 (uint16_t)(rd16(m, F3_A5 + 0x30EEu) & 0x3FFFu));   /* 0x12513A/41 */
            goto tail_12515A;
        }
        /* timer == 0: read the record's own duration */
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        int16_t dur = (int16_t)rd16(m, a1 + (uint32_t)(int16_t)rd16(m, obj + 0x3Au));
        if (dur == 0) {
            /* 0x125128: bftst (0x30EE,A5){1:1} ; beq 0x125178 */
            if ((rd16(m, F3_A5 + 0x30EEu) & 0x4000u) == 0u)
                wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));  /* 0x125132 */
            goto tail_125178;
        }
    }

tail_12515A:
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));   /* 0x12515A */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));   /* 0x125160 */
    wr8(m, obj + 0x35u, 0x00u);                                    /* 0x125166 */

    if (rd8(m, F3_A5 + 0x30F3u) == 0u)                             /* 0x12516A */
        sub_0CBB0C(m, obj);                                        /* 0x125172 */

tail_125178:
    /* 0x125178: jsr 0x11C096 -- propagate the hit flag to the parent */
    if (rd8(m, obj + 0x35u) != 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | rd8(m, obj + 0x35u)));
        if ((rd8(m, obj + 0x34u) & 0x02u) == 0u)
            wr8(m, obj + 0x35u, 0x00u);
    }

    (void)Obj_11B06E_Tick(m, obj);                                 /* 0x12517E */

    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du));     /* 0x125184 */

    (void)Obj_11B09A_Tick(m, obj);                                 /* 0x12518E */

    /* 0x125194: D0w = 0xE, D1w = 0xA ; bfclr (0x30EE,A5){2:1} */
    {
        uint16_t d0w = 0x000Eu, d1w = 0x000Au;
        wr16(m, F3_A5 + 0x30EEu,
             (uint16_t)(rd16(m, F3_A5 + 0x30EEu) & 0xBFFFu));
        if ((rd16(m, F3_A5 + 0x30EEu) & 0x2000u) != 0u) {
            /* 0x1251A6..0x125276: six fans around the block angle */
            uint16_t ang = sub_0CC33A(m, obj, d0w, d1w);
            uint32_t arg = 0x0E0A0000u;
            arg = (arg & 0xFFFF0000u) | (uint32_t)(uint16_t)(ang << 8);
            arg = (arg & 0xFFFFFF00u) | 0x28u;                 /* move.b #0x28,D1 */
            fan(m, obj, 0x00125B0Cu, arg);                     /* 0x1251CA */
            arg = (arg & 0xFFFF0000u) | 0xB01Eu;               /* 0x1251E4 */
            fan(m, obj, 0x00125B0Cu, arg);
            arg = (arg & 0xFFFF0000u) | 0x981Eu;               /* 0x125202 */
            fan(m, obj, 0x00125B0Cu, arg);
            arg = (arg & 0xFFFF0000u) | 0x801Eu;               /* 0x125220 */
            fan(m, obj, 0x00125B0Cu, arg);
            arg = (arg & 0xFFFF0000u) | 0x681Eu;               /* 0x12523E */
            fan(m, obj, 0x00125B0Cu, arg);
            arg = (arg & 0xFFFF0000u) | 0x501Eu;               /* 0x12525C */
            fan(m, obj, 0x00125B0Cu, arg);
        }
    }

    /* 0x12527A: bfclr {4:1} ; three fans, stepping +0x2000 then -0x4000 */
    wr16(m, F3_A5 + 0x30EEu, (uint16_t)(rd16(m, F3_A5 + 0x30EEu) & 0xEFFFu));
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x1000u) != 0u) {
        uint16_t ang = sub_0CC33A(m, obj, 0x000Eu, 0x000Au);   /* 0x125284 */
        uint32_t d1 = 0x0E0A0000u;
        d1 = (d1 & 0xFFFF0000u) | (uint32_t)(uint16_t)(ang << 8);
        d1 = (d1 & 0xFFFFFF00u) | 0x14u;                       /* move.b #0x14,D1 */
        fan(m, obj, 0x00125B0Cu, d1);                          /* 0x1252A8 */
        d1 = (uint16_t)((uint16_t)d1 + 0x2000u);               /* 0x1252B2 */
        fan(m, obj, 0x00125B0Cu, d1);
        d1 = (uint16_t)((uint16_t)d1 - 0x4000u);               /* 0x1252D0 */
        fan(m, obj, 0x00125B0Cu, d1);
    }

    /* 0x1252EE: bfclr {14:1} ; roll 0xF ; one 0x0CD74E fan */
    wr16(m, F3_A5 + 0x30EEu, (uint16_t)(rd16(m, F3_A5 + 0x30EEu) & 0xFFFDu));
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0002u) != 0u &&
        sub_0CC2BC(m, 0x000Fu) == 0u) {                        /* 0x1252F8 */
        uint16_t ang = sub_0CC33A(m, obj, 0x000Eu, 0x000Au);   /* 0x125310 */
        uint32_t d1 = 0x0E0A0000u;
        d1 = (d1 & 0xFFFF0000u) |
             (uint32_t)(uint16_t)((uint16_t)(rd16(m, F3_A5 - 0x1456u) & 0x3F00u) - 0x2000u);
        d1 = (uint16_t)((uint16_t)d1 + (uint16_t)(ang << 8));  /* 0x12531E..0x12532A */
        fan(m, obj, 0x000CD74Eu, d1);                          /* 0x125336 */
    }

    /* 0x125346: bfclr {13:1} ; roll 0x6 ; one 0x0CCDB6 fan */
    wr16(m, F3_A5 + 0x30EEu, (uint16_t)(rd16(m, F3_A5 + 0x30EEu) & 0xFFFBu));
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0004u) != 0u &&
        sub_0CC2BC(m, 0x0006u) == 0u) {                        /* 0x125350 */
        uint16_t ang = sub_0CC33A(m, obj, 0x000Eu, 0x000Au);   /* 0x125360 */
        uint32_t d1 = 0x0E0A0000u;
        d1 = (d1 & 0xFFFF0000u) |
             (uint32_t)(uint16_t)((uint16_t)(rd16(m, F3_A5 - 0x1456u) & 0x3F00u) - 0x2000u);
        d1 = (uint16_t)((uint16_t)d1 + (uint16_t)(ang << 8));
        fan(m, obj, 0x000CCDB6u, d1);                          /* 0x12538E */
    }

    wr32(m, obj + 0x00u, 0x001250E4u);                         /* 0x12539E bra 0x1250DC */
    return 0;
}
