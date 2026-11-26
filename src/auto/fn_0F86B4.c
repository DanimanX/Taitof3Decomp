/* ROM 0x0F86B4: orbiting-turret / sub-part follower tick (A0 = object).
 * Reads parent A4 = +0x08; if parent +0x06 == 0 calls Obj_StartDying
 * (0x0CEE74). Otherwise syncs (+0x14, +0x18) = parent (+0x14, +0x18 + 1) and
 * +0x1D = parent +0x1D; if parent +0x58 != self +0x58 updates +0x58 and
 * installs behaviour (0x0F879E or 0x0F8762) + (+0x68).w via 0x0D0678. When
 * parent +0x06 == 2 adds 0x0CF898 D1 offset to +0x18 and steps script
 * (0x0D0690). Finally draws parent then self (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

int Fn_0F86B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];
    uint32_t a4 = rd32(m, obj + 0x08u);                 /* 0x0F86B4 */

    if (rd8(m, a4 + 0x06u) == 0u) {                     /* 0x0F86B8 */
        Obj_StartDying(m, obj);                         /* 0x0F86BC */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, a4 + 0x14u));          /* 0x0F86C2 */
    wr32(m, obj + 0x18u, rd32(m, a4 + 0x18u));          /* 0x0F86C8 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u)); /* 0x0F86CE */
    wr8(m, obj + 0x1Du, rd8(m, a4 + 0x1Du));            /* 0x0F86D2 */

    uint16_t d0w = rd16(m, a4 + 0x58u);                 /* 0x0F86D8 */
    d0 = setw(d0, d0w);
    if (d0w != rd16(m, obj + 0x58u)) {                  /* 0x0F86DC */
        a1 = 0x000F879Eu;                               /* 0x0F86E4 */
        wr16(m, obj + 0x58u, d0w);                      /* 0x0F86E8 */
        if (d0w != 0u)                                  /* 0x0F86EC */
            a1 = 0x000F8762u;                           /* 0x0F86F0 */
        a1 = (uint32_t)((int32_t)a1 + (int16_t)rd16(m, obj + 0x68u)); /* 0x0F86F4 */
        Obj_StartBehaviour(m, obj, a1);                 /* 0x0F86F8 */
        a1 += 8u;
    }

    if (rd8(m, a4 + 0x06u) == 2u) {                     /* 0x0F86FE */
        if (Obj_0CF898(m, obj, &d0, &d1) < 0)           /* 0x0F8708 */
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1); /* 0x0F870E */
        step_script_regs(m, obj, &d0, &a1);             /* 0x0F8712 */
    }

    Obj_DrawSprite(m, a4);                              /* 0x0F8718..0x0F871A */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0F8720..0x0F8722 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
