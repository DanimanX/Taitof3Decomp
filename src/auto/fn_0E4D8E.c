/* ROM 0x0E4D8E: 4-frame pulsing mine/pod enemy tick (A0 = object).
 * On +0x06 == 3, HP <= 0 (awards 0x12C via 0x0D0E04), or A5-0x144B bit 5
 * clears +0x06 and calls Obj_StartDying (0x0CEE74). Otherwise scrolls (+0x14,
 * +0x18) by (A5+0x4224, A5+0x4228), when +0x06 != 2 increments +0x21 and
 * loads frame (+0x21 >> 5) & 3 from table 0x0E4E8E via 0x0CFA86, adds to
 * collision lists (0x0D0DC2), blinks colour (0x0CBE34), and on bomb-hit
 * (A5+0x1D58 bit 1, X in [-10, 284]) awards 0x12C via 0x0D0E30, sets sprite
 * 0x05DA8C at (-5, +3), and tail-calls Obj_ExplodeTick (0x0CBB2C). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CFA86: load {pal.w, spr.l, hit.l} from A1 into +0x1C/+0x10/+0x68. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t *a1_io, uint32_t *d0_io)
{
    uint32_t a1 = *a1_io;
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
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

int Fn_0E4D8E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) == 3u) {                    /* 0x0E4D8E */
        Obj_StartDying(m, obj);                         /* 0x0E4D94 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0E4D9A */
        d0 = 0x0000012Cu;                               /* 0x0E4E1A */
        Obj_0D0E04(m, obj, d0);                         /* 0x0E4E20 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0E4E26 */
        Obj_StartDying(m, obj);                         /* 0x0E4E2A */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {      /* 0x0E4DA2 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0E4E26 */
        Obj_StartDying(m, obj);                         /* 0x0E4E2A */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x0E4DAC */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0E4DB0 */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0E4DB4 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0E4DB8 */

    if (rd8(m, obj + 0x06u) != 2u) {                    /* 0x0E4DBC */
        uint8_t d0b = (uint8_t)(rd8(m, obj + 0x21u) + 1u); /* 0x0E4DC6..0x0E4DCA */
        wr8(m, obj + 0x21u, d0b);                       /* 0x0E4DCC */
        uint16_t idx = (uint16_t)((d0b >> 5) & 3u);     /* 0x0E4DD0..0x0E4DD2 */
        d0 = setw(d0, idx);
        wr8(m, obj + 0x23u, (uint8_t)idx);              /* 0x0E4DD6 */
        a1 = rd32(m, 0x000E4E8Eu + (uint32_t)idx * 4u); /* 0x0E4DDA */
        sub_0CFA86(m, obj, &a1, &d0);                   /* 0x0E4DE2 */
    }

    add_coll_regs(m, obj, &a1);                         /* 0x0E4DE8 */
    Obj_BlinkColour(m, obj);                            /* 0x0E4DEE */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0E4DF4 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0E4E0C */
            d0 = 0x0000012Cu;                           /* 0x0E4E30 */
            Obj_RelayTimer(m, d0);                      /* 0x0E4E36 */
            wr8(m, obj + 0x06u, 0u);                    /* 0x0E4E3C */
            wr32(m, obj + 0x10u, 0x0005DA8Cu);          /* 0x0E4E40 */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 5u)); /* 0x0E4E48 */
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 3u)); /* 0x0E4E4C */
            Obj_ExplodeTick(m, obj);                    /* 0x0E4E50 */
        }
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
