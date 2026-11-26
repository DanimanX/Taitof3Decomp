/* ROM 0x10F9E0: multi-jointed/serpent enemy segment tick (A0 = object).
 * Toggles palette ramp (0x00183C) every 0x14/0x1E frames via +0x23/+0x4E.
 * On +0x24 <= 0 awards 0x1770 via 0x0D0E04 and spawns 0x117B44; on death or
 * A5-0x144B bit 5 spawns 0x0DC340, clears bit (+0x0C)->+2 in A5+0x1D5E, clears
 * +0x06, and calls Obj_StartDying (0x0CEE74). Otherwise when +0x06 != 2 runs
 * ObjScript_CF1E4 (if flags7 bit 1) and ObjScript_CF4B8 (0x10FBFE); writes
 * +0x58 into circular buffer (+0x0C)->+4[(+0x0C)], subtracts A5+0x4228 from
 * +0x3E, applies sprite/offset table 0x10FC90[+0x22] via 0x0CFA7C, appends to
 * collision lists (0x0D0DC2), flashes colour (0x0CBE34), and while on-screen
 * with bomb active homes (+0x3A, +0x3E) toward (0xA0, 0x74) via 0x0D0E52 +
 * 0x0D2D44(7), adds orbit offset 0x0CF898, and steps +0x58 by -8 mod 0x168. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
}

/* ROM 0x0D0DC2 with A1 register tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
}

int Fn_10F9E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    {
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x10F9E0 */
        wr8(m, obj + 0x23u, cnt);
        if (cnt == 0u) {
            uint8_t tog = (uint8_t)~rd8(m, obj + 0x4Eu);    /* 0x10F9E8 */
            wr8(m, obj + 0x4Eu, tog);
            if (tog == 0u) {
                wr8(m, obj + 0x23u, 0x14u);                 /* 0x10F9F0 */
                (void)Pal_QueueRamp(m, 0x00110050u, 0x00110064u, 0x566Cu, 0x0Fu, 5u); /* 0x10FA14 */
            } else {
                wr8(m, obj + 0x23u, 0x1Eu);                 /* 0x10FA22 */
                (void)Pal_QueueRamp(m, 0x00110064u, 0x00110050u, 0x566Cu, 0x19u, 5u); /* 0x10FA46 */
            }
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* 0x10FA50 */
        d0 = 0x00001770u;                                   /* 0x10FB86 */
        {
            uint8_t mode = rd8(m, obj + 0x35u);
            if (mode == 1u)
                a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
            else if (mode == 2u)
                a1 = rd32(m, obj + 0x36u);
        }
        Obj_0D0E04(m, obj, d0);                             /* 0x10FB8C */
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0);    /* 0x10FB92..0x10FBA8 */
        goto die_dying;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)            /* 0x10FA58 */
        goto die_dying;

    if (rd8(m, obj + 0x06u) != 2u) {                        /* 0x10FA62 */
        uint8_t f7 = rd8(m, obj + 0x07u);                   /* 0x10FA6C */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)                /* 0x10FA76 */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x10FA7C */
            if (ObjScript_CF4B8(m, obj, 0x0010FBFEu) < 0)   /* 0x10FA88 */
                return -1;
        }
    }

    {
        uint32_t desc = rd32(m, obj + 0x0Cu);               /* 0x10FA8E */
        uint16_t idx = rd16(m, desc);                       /* 0x10FA92 */
        wr16(m, rd32(m, desc + 4u) + (uint32_t)(int32_t)(int16_t)idx, rd16(m, obj + 0x58u)); /* 0x10FA94: ([4,A1],D0.w) */
        idx = (uint16_t)((idx + 2u) & 0x01FFu);             /* 0x10FA9C..0x10FA9E */
        wr16(m, desc, idx);                                 /* 0x10FAA2 */
    }

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u)); /* 0x10FAA4..0x10FAA8 */

    {
        uint16_t k = rd8(m, obj + 0x22u);                   /* 0x10FAAC..0x10FAAE */
        uint32_t rec = rd32(m, 0x0010FC90u + (uint32_t)k * 4u); /* 0x10FAB2 */
        d1 = rd32(m, rec); rec += 4u;                       /* 0x10FABA */
        uint16_t ox = rd16(m, rec); rec += 2u;              /* 0x10FABC */
        uint16_t oy = rd16(m, rec); rec += 2u;              /* 0x10FABE */
        uint32_t tbl = rd32(m, rec);                        /* 0x10FAC0 */
        uint16_t ang = rd16(m, obj + 0x58u);                /* 0x10FAC2 */
        Obj_0CFA7C(m, obj, tbl, ang);                       /* 0x10FAC6 */
        d0 = setw(d0, rd16(m, obj + 0x1Cu));                /* 0x0CFA86: low word = sprite pal/word */

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x10FACC */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - ox)); /* 0x10FAD2 */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x10FAD6 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - oy)); /* 0x10FADC */
    }

    add_coll_regs(m, obj, &a1);                             /* 0x10FAE0 */
    Obj_BlinkColour(m, obj);                                /* 0x10FAE6 */

    {
        int on_screen = ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
                         (int16_t)rd16(m, obj + 0x14u) >= -0x000A &&
                         (int16_t)rd16(m, obj + 0x14u) <= 0x011C);
        if (rd8(m, obj + 0x06u) == 2u) {                    /* 0x10FAEC */
            if (!on_screen) {                               /* 0x10FB16..0x10FB2C */
                wr8(m, obj + 0x06u, 1u);                    /* 0x10FB34 */
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            }
        } else {
            if (!on_screen) {                               /* 0x10FAF6..0x10FB0C */
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            }
            wr8(m, obj + 0x06u, 2u);                        /* 0x10FB3C */
        }
    }

    {
        int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - 0x00A0u); /* 0x10FB42..0x10FB46 */
        int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - 0x0074u); /* 0x10FB4A..0x10FB4E */
        uint16_t ang = Obj_Atan256(m, dx, dy);              /* 0x10FB52 */
        sub_0D2D44(m, ang, 7u, &d0, &d1);                   /* 0x10FB58..0x10FB5A */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);    /* 0x10FB60 */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);    /* 0x10FB64 */

        if (Obj_0CF898(m, obj, &d0, &d1) < 0)               /* 0x10FB68 */
            return -1;
        a1 = rd32(m, obj + 0x0Cu);                          /* 0x0CF898 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);    /* 0x10FB6E */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);    /* 0x10FB72 */

        int16_t v58 = (int16_t)(rd16(m, obj + 0x58u) - 8);  /* 0x10FB76 */
        if (v58 < 0)                                        /* 0x10FB7A */
            v58 = 0x0167;                                   /* 0x10FB7E */
        wr16(m, obj + 0x58u, (uint16_t)v58);
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

die_dying:
    d0 = Obj_Spawn(m, obj, 0x000DC340u, 3u, 2u, 0u);        /* 0x10FBAC..0x10FBC4 */
    if ((int32_t)d0 >= 0) {                                 /* 0x10FBCA */
        uint32_t child = d0;
        wr32(m, child + 0x14u, rd32(m, obj + 0x3Au));       /* 0x10FBD0 */
        wr32(m, child + 0x18u, rd32(m, obj + 0x3Eu));       /* 0x10FBD6 */
        wr16(m, child + 0x58u, rd16(m, obj + 0x58u));       /* 0x10FBDC */
        wr16(m, child + 0x60u, rd16(m, obj + 0x60u));       /* 0x10FBE2 */
    }
    a1 = rd32(m, obj + 0x0Cu);                              /* 0x10FBE8 */
    {
        uint8_t bit = rd8(m, a1 + 2u);                      /* 0x10FBEC */
        d0 = (d0 & 0xFFFFFF00u) | bit;
        wr8(m, F3_A5 + 0x1D5Eu, (uint8_t)(rd8(m, F3_A5 + 0x1D5Eu) & ~(1u << (bit & 7u)))); /* 0x10FBF0 */
    }
    wr8(m, obj + 0x06u, 0u);                                /* 0x10FBF4 */
    Obj_StartDying(m, obj);                                 /* 0x10FBF8 -> 0x0CEE74 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
