/* ROM 0x19D6FA: boss part/muzzle follow & scripted animation tick (A0 = object).
 * If (int16_t)(A5+0x4B38) == -1, sets +0x06=2 and transitions to 0x17A85C.
 * Otherwise loads parent (+0x08) into A4, stores (A4+0x50, A4+0x54 - 9) into
 * (+0x4C, +0x4E), steps Obj_Script10 (0x178982), copies +0x50/+0x54 to
 * +0x14/+0x18, ticks 0x179A5C[+0x40], centers by sprite {w,h}*8 - 4, draws
 * sprite (0x001282), and re-arms 0x19D6FA if +0x44 != -1 else arms 0x19D640. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static int16_t abs16(int16_t v)
{
    return v < 0 ? (int16_t)(0u - (uint16_t)v) : v;
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

/* ROM 0x179074 with D0/D1/A1 register tracking (high word of D1 survives 0x001282). */
static void scr_179074_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    int16_t d2 = (int16_t)(int8_t)rd8(m, obj + 0x42u);
    if (d2 < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    int16_t ex = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t ey = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    r->d[0] = setw(r->d[0], (uint16_t)ex);
    r->d[1] = setw(r->d[1], (uint16_t)ey);
    if (!(abs16(ex) > d2 || abs16(ey) > d2)) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    uint16_t ang = Obj_Atan2(m, (uint16_t)ex, (uint16_t)ey);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t jitter = rd16(m, a1 + 8u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    uint8_t b = (uint8_t)(((uint8_t)ang & (uint8_t)jitter)
                          + (uint8_t)(int8_t)(int16_t)(jitter >> 8));
    wr8(m, obj + 0x20u, b);

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    r->a[1] = 0x00027E40u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    r->d[0] = (uint32_t)vx;
    r->d[1] = (uint32_t)vy;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178982 with D0/D1/A1 register tracking for 19D6FA's script commands. */
static int script10_step_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        r->a[1] = a1;
        uint32_t fn = rd32(m, a1);

        if (fn == 0u) {
            int16_t c = (int16_t)rd16(m, obj + 0x4Au);
            if (c >= 0) {
                if (c == 0)
                    wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
                c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
                wr16(m, obj + 0x4Au, (uint16_t)c);
                if (c == 0) {
                    a1 += 0x0Au;
                    goto store;
                }
            }
            a1 = rd32(m, a1 + 6u);
        store:
            r->a[1] = a1;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            return 0;
        }

        if (fn == 0x00178B20u) {
            Obj_Script10_SetHeading(m, obj);
        } else if (fn == 0x00179074u) {
            scr_179074_regs(m, r, obj);
        } else {
            return Obj_Script10(m, obj);
        }

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0) {
                r->a[1] = a1;
                wr32(m, obj + 0x44u, a1);
                wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                return 0;
            }
            int16_t c = (int16_t)rd16(m, obj + 0x4Au);
            if (c >= 0) {
                if (c == 0)
                    wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
                c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
                wr16(m, obj + 0x4Au, (uint16_t)c);
                if (c == 0) {
                    a1 += 0x0Au;
                    r->a[1] = a1;
                    wr32(m, obj + 0x44u, a1);
                    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                    return 0;
                }
            }
            a1 = rd32(m, a1 + 6u);
            r->a[1] = a1;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            return 0;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        r->a[1] = a1;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }
}

int Fn_19D6FA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {             /* 0x19D6FA..0x19D700 */
        wr16(m, obj + 0x06u, 0x0002u);                         /* 0x17A84C (beq never taken) */
        wr32(m, obj + 0x00u, 0x0017A85Cu);                     /* 0x17A854 */
        return 0;
    }

    uint32_t a4 = rd32(m, obj + 0x08u);                        /* 0x19D706 */
    r->a[4] = a4;
    wr16(m, obj + 0x4Cu, rd16(m, a4 + 0x50u));                 /* 0x19D70A */
    wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, a4 + 0x54u) - 9u)); /* 0x19D710..0x19D716 */

    if (script10_step_regs(m, r, obj) < 0)                     /* 0x19D71C */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));                /* 0x19D722 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                /* 0x19D728 */

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)        /* 0x19D72E..0x19D73C */
        return -1;

    uint32_t a1 = rd32(m, obj + 0x10u);                        /* 0x19D73E */
    uint16_t sy = (uint16_t)(rd16(m, a1) << 3);                /* 0x19D742, 0x19D748 */
    uint16_t sx = (uint16_t)(rd16(m, a1 + 2u) << 3);           /* 0x19D744, 0x19D746 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - sx)); /* 0x19D74A */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy)); /* 0x19D74E */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 4u)); /* 0x19D752 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 4u)); /* 0x19D756 */

    draw_sprite_regs(m, r);                                    /* 0x19D75A */

    if (rd32(m, obj + 0x44u) != 0xFFFFFFFFu)                   /* 0x19D760..0x19D768 */
        wr32(m, obj + 0x00u, 0x0019D6FAu);                     /* 0x19D6F2 */
    else
        wr32(m, obj + 0x00u, 0x0019D640u);                     /* 0x19D638 */
    return 0;
}
