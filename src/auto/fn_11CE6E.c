/* ROM 0x11CE6E: boss "target camera" steering tick (A0 = object).
 * Aims +0x20 at the target (A0+0x3A, A0+0x3C) with 0x006422, rotating the byte
 * heading by +/-2 per frame, then moves along it from the camera position
 * (0x11B262 = 0x0CBE12 with the camera A5+0x30D2/A5+0x30D6 as the base).
 * While the vertical error stays inside +/-0x0A it only re-seeds the wobble
 * rates +0x3A/+0x3C from 0x001D8A (0x11CE42) and re-arms 0x11CE6E. Otherwise
 * it counts +0x06 down, clearing mode bit 25 at the 0x46 mark, and when the
 * counter runs out it hands the frame count to the 0x0CBEB0 difficulty scale:
 * 0x50 - scale(0x3C) frames with the tick parked at 0x11CEEE, then mode bit 30
 * with 0x64 - scale(0x46) frames at 0x11CF18, then mode bit 28 with a 0x8C
 * countdown at 0x11CF34. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"    /* Obj_Atan2 (0x006422) */

#define MODE_L (F3_A5 + 0x30EEu)
#define CAM_X  (F3_A5 + 0x30D2u)
#define CAM_Y  (F3_A5 + 0x30D6u)
#define SIN_TBL 0x00027E40u
#define FNX_11CE6E 0x0011CE6Eu

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B262: move from the camera position along +0x20 with +0x42 as speed,
 * then re-latch the camera. Leaves D0 = angle>>8, D1 = speed*cos, D2 = speed*sin,
 * A3 = 0x27E40 (0x0CBE12). */
static void sub_11B262(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *d2_io)
{
    wr32(m, obj + 0x14u, rd32(m, CAM_X));               /* 0x11B262 */
    wr32(m, obj + 0x18u, rd32(m, CAM_Y));               /* 0x11B268 */
    uint32_t d0 = setw(*d0_io, (uint16_t)(rd16(m, obj + 0x20u) >> 8)); /* 0x11B26E */
    uint16_t speed = rd16(m, obj + 0x42u);              /* 0x0CBE12 */
    uint16_t ang = (uint16_t)d0;
    int16_t c = (int16_t)rd16(m, SIN_TBL + 0x80u + (uint32_t)ang * 2u);
    int16_t s = (int16_t)rd16(m, SIN_TBL + (uint32_t)ang * 2u);
    uint32_t p1 = (uint32_t)((int32_t)(int16_t)speed * (int32_t)c);
    uint32_t p2 = (uint32_t)((int32_t)(int16_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + p1);    /* 0x0CBE24 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + p2);    /* 0x0CBE2E */
    wr32(m, CAM_X, rd32(m, obj + 0x14u));               /* 0x11B27A */
    wr32(m, CAM_Y, rd32(m, obj + 0x18u));               /* 0x11B280 */
    *d0_io = d0;
    *d1_io = p1;
    *d2_io = p2;
}


/* ROM 0x006422 as an inline block: the routine's `exg.l d1,d0` means the
 * returned D0 keeps the HIGH word of the dy operand (D1) when exactly one of
 * the two operands is negative, and of the dx operand (D0) otherwise. */
static uint32_t atan2_regs(const f3mem_t *m, uint32_t d0_in, uint32_t d1_in)
{
    uint16_t dx = (uint16_t)d0_in;
    uint16_t dy = (uint16_t)d1_in;
    uint16_t ang = Obj_Atan2(m, dx, dy);
    int neg_dx = (int16_t)dx < 0;
    int neg_dy = (int16_t)dy < 0;
    uint32_t hi = (neg_dx != neg_dy) ? (d1_in & 0xFFFF0000u) : (d0_in & 0xFFFF0000u);
    return hi | ang;
}

/* ROM 0x0CBEB0: difficulty-scaled word A5+0x87E -> D0 = swap(d0 * (0x87E<<3 +
 * 0xC00) << 4), D1 = that scale word. */
static uint32_t sub_0CBEB0(const f3mem_t *m, uint32_t d0, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    *d1_io = setw(*d1_io, d1w);
    int32_t prod = (int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)d1w;
    uint32_t sh = (uint32_t)prod << 4;
    return (sh << 16) | (sh >> 16);
}

int Fn_11CE6E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];

    uint16_t dx = (uint16_t)(rd16(m, obj + 0x3Au) - rd16(m, obj + 0x14u)); /* 0x11CE6E */
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x3Cu) - rd16(m, obj + 0x18u)); /* 0x11CE76 */
    d0 = setw(d0, dx);                                  /* 0x11CE6E..0x11CE72 */
    d1 = setw(d1, dy);                                  /* 0x11CE76..0x11CE7A */
    d0 = atan2_regs(m, d0, d1);                         /* 0x11CE7E */
    d0 = setw(d0, (uint16_t)((uint8_t)d0 - rd8(m, obj + 0x20u))); /* 0x11CE84 */
    if ((int8_t)d0 >= 0)                                /* 0x11CE8A blt */
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + 2u)); /* 0x11CE8E */
    else
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) - 2u)); /* 0x11CE96 */

    sub_11B262(m, obj, &d0, &d1, &d2);                  /* 0x11CE9A */

    d0 = setw(d0, (uint16_t)(rd16(m, obj + 0x3Cu) - rd16(m, obj + 0x18u))); /* 0x11CEA0 */
    if ((int16_t)d0 > 0x000A || (int16_t)d0 < -0x000A)  /* 0x11CEA8 / 0x11CEB0 */
        goto lab_11CEBA;
    goto lab_11CE42;                                    /* 0x11CEB8 bra */

lab_11CEBA:
    if (rd16(m, obj + 0x06u) == 0x0046u)                /* 0x11CEBA cmpi/bne */
        wr32(m, MODE_L, rd32(m, MODE_L) & ~0x02000000u); /* 0x11CEC4 bfclr {6:1} */
    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11CECA subq.w */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {                                    /* 0x11CECE bgt */
            wr32(m, obj + 0x00u, FNX_11CE6E);           /* 0x11CE66 */
            goto out;
        }
    }

    d0 = setw(d0, 0x003Cu);                             /* 0x11CED0 */
    d0 = sub_0CBEB0(m, d0, &d1);                        /* 0x11CED4 */
    d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));       /* 0x11CEDA neg.w */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 + 0x0050u));  /* 0x11CEDC addi.w */
    wr16(m, obj + 0x06u, (uint16_t)d0);                 /* 0x11CEE0 */
    if ((uint16_t)d0 != 0u) {                           /* 0x11CEE4 beq */
        wr32(m, obj + 0x00u, 0x0011CEEEu);              /* 0x11CEE6 */
        goto out;
    }

    wr32(m, MODE_L, rd32(m, MODE_L) | 0x40000000u);     /* 0x11CEF4 bfset {1:1} */
    d0 = setw(d0, 0x0046u);                             /* 0x11CEFA */
    d0 = sub_0CBEB0(m, d0, &d1);                        /* 0x11CEFE */
    d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));       /* 0x11CF04 neg.w */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 + 0x0064u));  /* 0x11CF06 addi.w */
    wr16(m, obj + 0x06u, (uint16_t)d0);                 /* 0x11CF0A */
    if ((uint16_t)d0 != 0u) {                           /* 0x11CF0E beq */
        wr32(m, obj + 0x00u, 0x0011CF18u);              /* 0x11CF10 */
        goto out;
    }

    wr32(m, MODE_L, rd32(m, MODE_L) | 0x10000000u);     /* 0x11CF1E bfset {3:1} */
    wr16(m, obj + 0x06u, 0x008Cu);                      /* 0x11CF24 */
    /* 0x11CF2A beq after a non-zero constant: never taken. */
    wr32(m, obj + 0x00u, 0x0011CF34u);                  /* 0x11CF2C */
    goto out;

lab_11CE42:                                             /* 0x11CE42, shared tail */
    d0 = Rng_NextMem(m);                                /* 0x11CE42 jsr 0x001D8A */
    d0 = setw(d0, (uint16_t)((uint16_t)(d0 & 0x001Fu) + 0x00DCu));
    wr16(m, obj + 0x3Au, (uint16_t)d0);                 /* 0x11CE50 */
    d0 = Rng_NextMem(m);                                /* 0x11CE54 */
    d0 = setw(d0, (uint16_t)((uint16_t)(d0 & 0x007Fu) + 0x0040u));
    wr16(m, obj + 0x3Cu, (uint16_t)d0);                 /* 0x11CE62 */
    wr32(m, obj + 0x00u, FNX_11CE6E);                   /* 0x11CE66 */

out:
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    return 0;
}
