/* ROM 0x12AC6C: player-attached rotating/aiming pod tick (A0 = object).
 * Syncs +0x59 from parent (+0x08); if non-zero, frees self (0x0105EC).
 * Otherwise copies position (+0x14/+0x18) from +0x44 (0x12B3E2), advances
 * +0x20 by A5+0x327A (and A5+0x327A by A5+0x327C), maps high byte through
 * (x - 0x50)*3/8 + 0x36, moves by angle via Obj_MoveByAngle (0x0CBE12),
 * selects hitbox/sprite table (0x12C8EA / 0x12C90E / 0x12C8C6) by angle
 * range [0x33..0x39], loads hitboxes and frame via +0x3A (0..3, +0x2000),
 * registers collision & palette via Obj_11B06E (0x11B06E), syncs +0x4F from
 * grandparent +0x1D, and if A5+0x30EE bit 4 is set, periodically spawns
 * 0x0CCE0E (0x0CC2BC + 0x01057C). Re-arms 0x12AC6C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

static uint32_t setb(uint32_t reg, uint8_t b)
{
    return (reg & 0xFFFFFF00u) | b;
}

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static int8_t asr8(int8_t v, unsigned n)
{
    return (int8_t)((v < 0) ? ~(uint8_t)((uint8_t)~(uint8_t)v >> n)
                            : ((uint8_t)v >> n));
}

/* ROM 0x0CC2BC: scaled modulo of A5+0x0878 by ((0x180 - A5+0x087E) * d0w) >> 8. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    prod = (prod << 16) | (prod >> 16);
    if ((uint16_t)prod == 0u)
        prod = 1u;
    *d1_io = prod;

    uint32_t d0 = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint16_t div = (uint16_t)prod;
    if (div != 0u) {
        uint32_t q = d0 / div;
        if (q <= 0xFFFFu) {
            uint32_t rem = d0 % div;
            d0 = ((rem & 0xFFFFu) << 16) | (q & 0xFFFFu);
        }
    }
    *d0_io = (d0 << 16) | (d0 >> 16);
}

int Fn_12AC6C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x12AC6C */

    uint8_t f59 = rd8(m, a1 + 0x59u);                   /* 0x12AC70 */
    wr8(m, obj + 0x59u, f59);
    if (f59 != 0u) {                                    /* 0x12AC76 */
        Obj_Free(m, obj);                               /* 0x12ADAA..0x12ADB2 */
        goto done;
    }

    a1 = rd32(m, obj + 0x44u);                          /* 0x12AC7A..0x12B3E2 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));          /* 0x12B3E6 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));          /* 0x12B3EC */

    uint16_t ang = (uint16_t)(rd16(m, a1 + 0x20u) + rd16(m, F3_A5 + 0x327Au)); /* 0x12AC82..0x12AC86 */
    wr16(m, obj + 0x20u, ang);                          /* 0x12AC8A */
    uint16_t w = (uint16_t)(ang >> 8);                  /* 0x12AC8E */
    wr16(m, F3_A5 + 0x327Au, (uint16_t)(rd16(m, F3_A5 + 0x327Au) + rd16(m, F3_A5 + 0x327Cu))); /* 0x12AC90..0x12AC94 */

    int8_t b0 = (int8_t)((uint8_t)w - 0x50u);           /* 0x12AC98 */
    int8_t b1 = b0;                                     /* 0x12AC9C */
    b0 = asr8(b0, 2u);                                  /* 0x12AC9E */
    b1 = asr8(b1, 3u);                                  /* 0x12ACA0 */
    b0 = (int8_t)(b0 + b1);                             /* 0x12ACA2 */
    uint8_t mv_ang = (uint8_t)((uint8_t)b0 + 0x36u);    /* 0x12ACA4 */
    d0 = setb(setw(d0, w), mv_ang);
    Obj_MoveByAngle(m, obj, (uint16_t)d0);              /* 0x12ACA8 */
    d1 = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x42u)
                  * (int32_t)(int16_t)rd16(m, 0x027EC0u + (uint32_t)(int16_t)(uint16_t)d0 * 2u));

    if ((int8_t)mv_ang < 0x33) {                        /* 0x12ACAE..0x12ACB2 */
        a1 = 0x0012C90Eu;                               /* 0x12ACCE */
        wr16(m, obj + 0x06u, 0x1221u);                  /* 0x12ACD4 */
    } else if ((int8_t)mv_ang > 0x39) {                 /* 0x12ACB6..0x12ACBA */
        a1 = 0x0012C8C6u;                               /* 0x12ACDE */
        wr16(m, obj + 0x06u, 0x0C22u);                  /* 0x12ACE4 */
    } else {
        a1 = 0x0012C8EAu;                               /* 0x12ACBE */
        wr16(m, obj + 0x06u, 0x0C1Fu);                  /* 0x12ACC4 */
    }

    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12ACEE..0x12ACF2 */
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12ACF6..0x12ACFA */
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12ACFE..0x12AD02 */
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12AD06..0x12AD0A */

    int16_t frame = (int16_t)rd16(m, obj + 0x3Au);      /* 0x12AD0E */
    a1 += (uint32_t)((int32_t)frame * 8);               /* 0x12AD12 */
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;        /* 0x12AD16 */
    wr8(m, obj + 0x1Cu, rd8(m, a1++));                  /* 0x12AD1A */
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++)); /* 0x12AD1E..0x12AD22 */
    int16_t off52 = (int16_t)(int8_t)rd8(m, a1++);      /* 0x12AD26..0x12AD28 */
    d0 = setw(d0, (uint16_t)off52);
    wr16(m, obj + 0x52u, (uint16_t)off52);              /* 0x12AD2A */

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x2000u); /* 0x12AD2E */
    if ((int16_t)rd16(m, obj + 0x3Au) >= 4)             /* 0x12AD36..0x12AD3C */
        wr16(m, obj + 0x3Au, 0u);                       /* 0x12AD40 */

    wr8(m, obj + 0x35u, 0u);                            /* 0x12AD44 */
    Obj_11B06E(m, obj);                                 /* 0x12AD48 */

    a1 = rd32(m, obj + 0x08u);                          /* 0x12AD4E */
    a1 = rd32(m, a1 + 0x08u);                           /* 0x12AD52 */
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x1Du));            /* 0x12AD56 */

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x10u) != 0u) {      /* 0x12AD5C..0x12AD62 */
        sub_0CC2BC(m, 8u, &d0, &d1);                    /* 0x12AD66..0x12AD6A */
        if ((uint16_t)d0 == 0u) {                       /* 0x12AD70..0x12AD72 */
            d0 = (d0 << 16) | (d0 >> 16);               /* 0x12AD76 */
            d0 = setw(d0, (uint16_t)(d0 & 7u));         /* 0x12AD78 */
            if ((int16_t)(uint16_t)d0 < 4) {            /* 0x12AD7C..0x12AD80 */
                d1 = (rd32(m, obj + 0x06u) & 0xFFFF0000u) | 0x8000u; /* 0x12AD84..0x12AD88 */
                d0 = Obj_Spawn(m, obj, 0x000CCE0Eu, 9u, 2u, d1); /* 0x12AD8C..0x12AD9C */
            }
        }
    }

    wr32(m, obj + 0x00u, 0x0012AC6Cu);                  /* 0x12ADA6 -> 0x12AC64 */

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
