/* ROM 0x0D8922: spinning/flipping enemy tick (A0 = obj).
 * Steps {dur.w, sprdef.l} anim table 0x0D8A28 or 0x0D8A6C (selected by +0x21 bit 6)
 * via 0x0CBD24, applies scroll velocity (0x0CBE00), appends to collision lists
 * (0x0CBDA4), flashes hit colour (0x0CBE34), draws sprite (0x001282), and on
 * table end flips direction and switches to 0x0D87F2. On HP <= 0 or bomb-clear,
 * credits timer to score receiver (0x0CBDDE), plays sound (+0x65), spawns
 * 0x13E4EE with 0x13E4D0 arg, and transitions to 0x0D89F2. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

#define TBL_0D8A28 0x000D8A28u /* lea (0xe8,PC),A1 at 0x0D893E */
#define TBL_0D8A6C 0x000D8A6Cu /* lea (0x124,PC),A1 at 0x0D8946 */

/* ROM 0x0CBD24 with D0/D1 register tracking. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0_io = (*d0_io & 0xFFFF0000u) | off;
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = (*d1_io & 0xFFFF0000u) | cnt;
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x13E4D0: pack (spr+0)<<3 into D0[23:16] and (spr+2)<<3 into D0[31:24]. */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0_in)
{
    uint32_t spr = rd32(m, obj + 0x10u);
    uint32_t w0 = (uint32_t)((rd16(m, spr) << 3) & 0xFFu);
    uint32_t w1 = (uint32_t)((rd16(m, spr + 2u) << 3) & 0xFFu);
    return (d0_in & 0x0000FFFFu) | (w0 << 16) | (w1 << 24);
}

int Fn_0D8922(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                 /* 0x0D8922 */
        a1 = Obj_0CBDDE(m, obj);                              /* 0x0D89A0 */
        d0 = (uint32_t)rd16(m, obj + 0x4Cu);                  /* 0x0D89A6..0x0D89A8 */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);       /* 0x0D89AC */
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {     /* 0x0D892A */
        a1 = ((rd8(m, obj + 0x21u) & 0x40u) != 0u) ? TBL_0D8A28 : TBL_0D8A6C;
        sub_0CBD24(m, obj, a1, &d0, &d1);                     /* 0x0D894A */
        wr16(m, obj + 0x06u, rd16(m, a1 + (int16_t)d0));      /* 0x0D8950 */
        Obj_SubScrollVel(m, obj);                             /* 0x0D8956 -> 0x0CBE00 */

        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);               /* 0x0D895C -> 0x0CBDA4 */
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        a1 += 4u;
        wr32(m, F3_A5 + 0x0906u, a1);

        Obj_HitFlash(m, obj);                                 /* 0x0D8962 -> 0x0CBE34 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        draw_sprite_regs(m, r);                               /* 0x0D8968 -> 0x001282 */
        if ((int16_t)rd16(m, obj + 0x06u) > 0) {              /* 0x0D896E */
            wr32(m, obj, 0x000D8922u);                        /* 0x0D891A */
            return 0;
        }

        wr16(m, obj + 0x3Au, 0u);                             /* 0x0D8974 */
        wr16(m, obj + 0x3Eu, 0u);                             /* 0x0D8978 */
        if ((rd8(m, obj + 0x4Eu) & 0x08u) != 0u)              /* 0x0D897C */
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 0x40u));
        else
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + 0x40u));
        wr16(m, obj + 0x06u, 0x7FFFu);                        /* 0x0D8996 */

        /* 0x0D87D2 */
        d0 = (uint32_t)((rd8(m, obj + 0x21u) >> 6) & 0x03u);
        a1 = 0x000CC10Eu;
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, a1 + d0) ^ 0x02u));
        wr32(m, obj, 0x000D87F2u);                            /* 0x0D87EA */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    /* 0x0D89B0: death */
    Sound_PlayHooked(m, rd8(m, obj + 0x65u));                 /* 0x0D89B0..0x0D89C4 */
    d0 = sub_13E4D0(m, obj, 2u);                              /* 0x0D89C8..0x0D89CA */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);          /* 0x0D89D0..0x0D89E6 */
    wr32(m, obj, 0x000D89F2u);                                /* 0x0D89EA */
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
