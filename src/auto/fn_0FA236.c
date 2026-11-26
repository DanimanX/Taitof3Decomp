/* ROM 0x0FA236: child part tick (A0 = object).
 * Checks HP > 0 and bomb-clear, verifies parent (+0x08)->+0x06 != 0,
 * triggers opening/closing behaviour (0x0FA444 or 0x0FA504) when parent
 * reaches state 2, scrolls +0x14/+0x18 and moves Y by +0x42, checks
 * parent +0x0C Y bound, decrements +0x23, handles bomb-hit (0x0FA37E),
 * adds collision (0x0D0DC2), steps script, blinks, and draws. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0FA236(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0FA236 */
        d0 = 0x00000064u;                               /* 0x0FA32E */
        Obj_0D0E04(m, obj, d0);                         /* 0x0FA330 */
        goto death_fx;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0FA23E */
        goto death_fx;

    a1 = rd32(m, obj + 0x08u);                          /* 0x0FA248 */
    {
        uint8_t pstate = rd8(m, a1 + 0x06u);            /* 0x0FA24C */
        d0 = (d0 & 0xFFFFFF00u) | pstate;
        if (pstate == 0u) {                             /* 0x0FA250 */
            Obj_StartDying(m, obj);
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
        if (rd8(m, obj + 0x21u) == 0u && pstate == 2u) { /* 0x0FA256..0x0FA25E */
            wr8(m, obj + 0x21u, 1u);                    /* 0x0FA266 */
            wr8(m, obj + 0x23u, 0x0Fu);                 /* 0x0FA26C */
            wr16(m, obj + 0x42u, 2u);                   /* 0x0FA272 */
            a1 = 0x000FA444u;                           /* 0x0FA278 */
            if (rd8(m, obj + 0x20u) != 0u) {            /* 0x0FA27C */
                wr16(m, obj + 0x42u, 0xFFFEu);          /* 0x0FA284 */
                a1 = 0x000FA504u;                       /* 0x0FA28A */
            }
            Obj_StartBehaviour(m, obj, a1);             /* 0x0FA28E */
            a1 += 8u;
        }
    }

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x0FA294 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0FA298 */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0FA29C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0FA2A0 */
    {
        uint16_t dy = rd16(m, obj + 0x42u);             /* 0x0FA2A4 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + dy)); /* 0x0FA2A8 */
        a1 = rd32(m, obj + 0x08u);                      /* 0x0FA2AC */
        uint16_t lim = rd16(m, a1 + 0x0Cu);             /* 0x0FA2B0 */
        d0 = setw(d0, lim);
        int16_t cur_y = (int16_t)rd16(m, obj + 0x18u);
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x0FA2B4 */
            if ((int16_t)lim > cur_y) {                 /* 0x0FA2BC */
                Obj_StartDying(m, obj);
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            }
        } else {
            if ((int16_t)lim < cur_y) {                 /* 0x0FA2CA */
                Obj_StartDying(m, obj);
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            }
        }
    }

    if (rd8(m, obj + 0x23u) != 0u)                      /* 0x0FA2D4 */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u)); /* 0x0FA2DC */

    if (rd8(m, obj + 0x21u) != 0u) {                    /* 0x0FA2E0 */
        if (rd8(m, obj + 0x23u) == 0u && (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
            int16_t x = (int16_t)rd16(m, obj + 0x14u);
            if (x >= -0x000A && x <= 0x011C) {
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu)); /* 0x0FA308 */
                d0 = 0x00000064u;                       /* 0x0FA37E */
                Obj_RelayTimer(m, d0);                  /* 0x0FA380 */
                Obj_0CEF0E(m, obj);                     /* 0x0FA386 */
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            }
        }
        add_coll_regs(m, obj, &a1);                     /* 0x0FA314 */
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0FA31A */
    Obj_BlinkColour(m, obj);                            /* 0x0FA320 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0FA326 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_fx:
    d0 = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);    /* 0x0FA336 */
    if ((int32_t)d0 >= 0) {
        a1 = d0;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 8u));
        wr16(m, a1 + 0x58u, 0u);
        wr16(m, a1 + 0x60u, 0u);
    }
    Obj_StartDying(m, obj);
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
