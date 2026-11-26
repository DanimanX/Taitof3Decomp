/* ROM 0x13C652: boss/enemy launch-pad standby tick (A0 = object, A0+0x08 = parent).
 * Frees self (0x13C9F8) if parent+0x59 != 0. Updates hit-flash colour (0x11C32C)
 * and draws sprite (0x11B09A). Waits for A5+0x30EE bitfield {18:1} (A5+0x30F0
 * bit 5, 0x20) and +0x06 countdown to hit 0; then plays sound 0x3A, seeds HP/
 * dmg/hitboxes (selecting +0x30/+0x32 from +0x1C bit 1), sets velocity (+0x50,
 * +0x52) = (0x400, 0x300), and transitions to 0x13C6DE. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
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
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x11B09A with exact D0/D1/A1 register tracking. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);

    if (y < 0x00E8 && x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        r->a[1] = a1;
        if ((int16_t)(h + y) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            r->a[1] = a1;
            if ((int16_t)(w + x) > 0)
                draw_sprite_regs(m, r);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t d0w = rd16(m, obj + 0x56u);
    r->d[0] = setw(r->d[0], d0w);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
}

int Fn_13C652(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);             /* 0x13C652 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                 /* 0x13C656..0x13C65A */
        Obj_Free(m, obj);                           /* 0x13C9F8 */
        return 0;
    }

    Obj_11C32C(m, obj);                             /* 0x13C65E */
    sub_11B09A(m, r);                               /* 0x13C664 */

    if ((rd8(m, F3_A5 + 0x30F0u) & 0x20u) == 0u) {  /* 0x13C66A..0x13C670 */
        wr32(m, obj, 0x0013C652u);                  /* 0x13C64A */
        return 0;
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x13C672 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                         /* 0x13C676 */
        wr32(m, obj, 0x0013C652u);                  /* 0x13C64A */
        return 0;
    }

    Sound_PlayHooked(m, 0x003Au);                   /* 0x13C678..0x13C688 */
    wr16(m, obj + 0x28u, 0x0001u);                  /* 0x13C68C */
    wr16(m, obj + 0x24u, 0x0014u);                  /* 0x13C692 */
    wr16(m, obj + 0x2Cu, 0x0010u);                  /* 0x13C698 */
    wr16(m, obj + 0x2Eu, 0x0047u);                  /* 0x13C69E */

    if ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u) {      /* 0x13C6A4..0x13C6AA */
        wr16(m, obj + 0x30u, 0x0003u);              /* 0x13C6AE */
        wr16(m, obj + 0x32u, 0x000Eu);              /* 0x13C6B4 */
    } else {
        wr16(m, obj + 0x30u, 0x0011u);              /* 0x13C6BE */
        wr16(m, obj + 0x32u, 0x001Cu);              /* 0x13C6C4 */
    }

    wr16(m, obj + 0x50u, 0x0400u);                  /* 0x13C6CA */
    wr16(m, obj + 0x52u, 0x0300u);                  /* 0x13C6D0 */
    wr32(m, obj, 0x0013C6DEu);                      /* 0x13C6D6 */
    return 0;                                       /* 0x13C6DC */
}
