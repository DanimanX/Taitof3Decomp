/* ROM 0x13AEB2: boss/enemy animated turret tick (A0 = object).
 * Frees self via 0x0105EC if bit 4 of A5-0x144B is set or A5+0x32D6 >= 3.
 * On +0x24 <= 0, awards 0x7530 score (0x0CBDDE), spawns 0x13E4EE (grp 5,
 * cnt 2, arg 2), plays sound 0x35 via Sound_PlayHooked, sets +0x06 countdown
 * via 0x0CBEB0(0x64), and transitions to 0x13B058 (or 0x13AD8A if 0).
 * Otherwise steps frame table 0x13B070 via 0x0CBD24; if duration < 0, sets
 * +0x20 = 0x8000 and advances to 0x13AEFC; else updates collision/flash
 * (0x11B06E), draws via 0x11B09A, and re-arms 0x13AEB2. */
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
    uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, x);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, y);
    r->d[0] = setw(r->d[0], y);

    if ((int16_t)y < 0x00E8 && (int16_t)x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)(rd16(m, a1) << 4);
        a1 += 2u;
        r->d[0] = setw(r->d[0], (uint16_t)(h + (int16_t)y));
        if ((int16_t)r->d[0] > 0) {
            int16_t w = (int16_t)(rd16(m, a1) << 4);
            a1 += 2u;
            r->d[0] = setw(r->d[0], (uint16_t)(w + (int16_t)x));
            if ((int16_t)r->d[0] > 0) {
                r->a[1] = a1;
                draw_sprite_regs(m, r);
            } else {
                r->a[1] = a1;
            }
        } else {
            r->a[1] = a1;
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t offy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - offy));
    r->d[0] = setw(r->d[0], offy);
}

/* ROM 0x0CBD24 with D0/D1/A1 tracking. */
static int16_t sub_0CBD24(const f3mem_t *m, f3regs_t *r, uint32_t tbl)
{
    uint32_t obj = r->a[0];
    uint16_t d0w = rd16(m, obj + 0x3Au);
    int16_t dur;

    for (;;) {
        dur = (int16_t)rd16(m, tbl + (int32_t)(int16_t)d0w);
        if (dur <= 0)
            break;
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, d1w);
        r->d[1] = setw(r->d[1], d1w);
        if ((int16_t)d1w <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (int32_t)(int16_t)d0w + 2));
            break;
        }
        wr16(m, obj + 0x3Eu, 0u);
        d0w = (uint16_t)(d0w + 6u);
        wr16(m, obj + 0x3Au, d0w);
    }

    r->d[0] = setw(r->d[0], d0w);
    r->a[1] = tbl;
    return dur;
}

int Fn_13AEB2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u
        || (int16_t)rd16(m, F3_A5 + 0x32D6u) >= 3) {    /* 0x13AEB2..0x13AEC2 */
        Obj_Free(m, obj);                               /* 0x13B062 */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x13AEC6 */
        uint32_t a1 = Obj_0CBDDE(m, obj);               /* 0x13AFFA */
        r->a[1] = a1;
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u);
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u); /* 0x13B008..0x13B022 */
        Sound_PlayHooked(m, 0x0035u);                   /* 0x13B026..0x13B036 */

        uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0 */
        uint32_t d0 = (uint32_t)((int32_t)0x0064 * (int32_t)(int16_t)d1w) << 4;
        d0 = (d0 << 16) | (d0 >> 16);
        uint16_t d0w = (uint16_t)(0u - (int16_t)(uint16_t)((uint16_t)d0 - 0x00C8u)); /* 0x13B044..0x13B048 */
        wr16(m, obj + 0x06u, d0w);                      /* 0x13B04A */
        r->d[0] = setw(d0, d0w);
        r->d[1] = setw(r->d[1], d1w);
        if (d0w == 0u)
            wr32(m, obj, 0x0013AD8Au);                  /* 0x13B04E -> 0x13B05E -> 0x13AD82 */
        else
            wr32(m, obj, 0x0013B058u);                  /* 0x13B050 */
        return 0;
    }

    int16_t dur = sub_0CBD24(m, r, 0x0013B070u);        /* 0x13AECE..0x13AED2 */
    if (dur < 0) {                                      /* 0x13AED8..0x13AEDC */
        wr16(m, obj + 0x20u, 0x8000u);                  /* 0x13AEEE */
        wr32(m, obj, 0x0013AEFCu);                      /* 0x13AEF4 */
        return 0;
    }

    /* 0x13AEE0: 0x11B06E */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        r->a[1] = p2 + 4u;
        wr32(m, F3_A5 + 0x0906u, r->a[1]);
    }
    Obj_11C32C(m, obj);                                 /* 0x11B092 */

    sub_11B09A(m, r);                                   /* 0x13AEE6 */
    wr32(m, obj, 0x0013AEB2u);                          /* 0x13AEEC -> 0x13AEAA */
    return 0;
}
