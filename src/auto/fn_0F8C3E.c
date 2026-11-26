/* ROM 0x0F8C3E: attached-part sprite tick (A0 = part, +0x08 = parent).
 * Dies via 0x0CEE74 if parent state == 0. Otherwise tracks parent position
 * (+0x10, +1) and palette (+0x1D), updates anim via 0x0D0678 when parent +0x58
 * changes, runs 0x0CF898 orbit + 0x0D0690 script when parent state <= 2,
 * and draws parent then self (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

#define TBL_0F8CBA 0x000F8CBAu /* lea (0x44,PC),A1 at 0x0F8C74 */
#define TBL_0F8CF6 0x000F8CF6u /* lea (0x74,PC),A1 at 0x0F8C80 */

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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0F8C3E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                   /* 0x0F8C3E */
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, parent + 0x06u) == 0u) {                       /* 0x0F8C42 */
        Obj_StartDying(m, obj);                               /* 0x0F8C46 -> 0x0CEE74 */
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));            /* 0x0F8C4C */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u)); /* 0x0F8C52 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));            /* 0x0F8C58 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u)); /* 0x0F8C5E */
    wr8(m, obj + 0x1Du, rd8(m, parent + 0x1Du));              /* 0x0F8C62 */

    uint16_t p58 = rd16(m, parent + 0x58u);                   /* 0x0F8C68 */
    d0 = (d0 & 0xFFFF0000u) | p58;
    if (p58 != rd16(m, obj + 0x58u)) {                        /* 0x0F8C6C */
        wr16(m, obj + 0x58u, p58);
        a1 = (p58 == 0u) ? TBL_0F8CBA : TBL_0F8CF6;
        a1 = (uint32_t)((int32_t)a1 + (int16_t)rd16(m, obj + 0x68u));
        Obj_SetAnim(m, obj, a1);                              /* 0x0F8C88 -> 0x0D0678 */
        d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 2u);
    }

    if (rd8(m, parent + 0x06u) <= 2u) {                       /* 0x0F8C8E */
        if (Obj_0CF898(m, obj, &d0, &d1) < 0)                 /* 0x0F8C98 */
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);      /* 0x0F8C9E */
        uint16_t old_cnt = rd16(m, obj + 0x4Cu);
        Obj_StepScript(m, obj);                               /* 0x0F8CA2 -> 0x0D0690 */
        if (old_cnt == 1u) {
            uint32_t scr = rd32(m, obj + 0x48u);
            uint16_t w = rd16(m, scr);
            d0 = (d0 & 0xFFFF0000u) | (((int16_t)w < 0) ? w : rd16(m, scr + 2u));
        }
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[0] = parent;                                         /* 0x0F8CA8 */
    Obj_DrawSprite(m, parent);                                /* 0x0F8CAA */
    r->a[0] = obj;                                            /* 0x0F8CB0 */
    draw_sprite_regs(m, r);                                   /* 0x0F8CB2 -> 0x001282 */
    return 0;
}
