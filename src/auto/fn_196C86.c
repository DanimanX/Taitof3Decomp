/* ROM 0x196C86: boss sub-part AI/animation/attack tick (A0 = object).
 * On HP <= 0, awards 0x7530 via 0x17B02E + 0x12C5F to A5+0x886, flags A5+0x4B26
 * bit 2 (MSB), sets +0x23 bit 2, spawns 0x179D92 death handler, sets +0x06 = 2,
 * and transitions to 0x196BE8. Otherwise oscillates +0x20 when A5+0x4B3A bit 14
 * is set, optionally spawns 0x18339C / 0x180F24 attacks when target +0x4C is
 * at distance >= 0x50, steps Obj_Script10 (0x178982), runs 0x196AE0 (anim tick,
 * centered draw, colour flash, collision list append if A5+0x4B3A bit 15 clear),
 * and re-arms 0x196C86. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x17B02E: resolve score receiver from +0x35 (+0x36->+8 if 1, +0x36 if 2, else -1). */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        *a1_io = a1;
        return rd32(m, a1 + 0x08u);
    }
    if (mode == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem of (A5+0x878) / max(1, ((0x180 - A5+0x87E)*d0) >> 8). */
static uint32_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d0w * (uint32_t)d1w;
    prod <<= 8;
    d1w = (uint16_t)(prod >> 16);
    if (d1w == 0u)
        d1w = 1u;
    *d1_out = setw(prod >> 16, d1w);

    uint32_t dividend = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(dividend / d1w);
    uint16_t rem = (uint16_t)(dividend % d1w);
    return ((uint32_t)quot << 16) | rem;
}

/* ROM 0x196AE0: animate, draw centered sprite, blink colour, append to lists. */
static int sub_196AE0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd8(m, obj + 0x35u) != 0u) {                    /* 0x196AE0 */
        int8_t d0b = (int8_t)rd8(m, obj + 0x60u);
        if ((int8_t)rd8(m, obj + 0x20u) > 0)
            d0b = (int8_t)-d0b;
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)d0b));
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));         /* 0x196AFA */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));         /* 0x196B00 */

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0) /* 0x196B06..0x196B14 */
        return -1;

    {
        uint32_t a1 = rd32(m, obj + 0x10u);             /* 0x196B16 */
        uint16_t dy = (uint16_t)(rd16(m, a1) << 3);
        uint16_t dx = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dx));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));
    }

    if (rd8(m, obj + 0x1Du) == 6u)                      /* 0x196B2A */
        wr8(m, obj + 0x1Du, 8u);
    else if (rd8(m, obj + 0x35u) != 0u)                 /* 0x196B34 */
        wr8(m, obj + 0x1Du, 6u);
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));

    wr8(m, obj + 0x35u, 0u);                            /* 0x196B5A */
    draw_sprite_regs(m, r);                             /* 0x196B5E */

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {      /* 0x196B64: {0xf:1} */
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        a1 += 4u;
        wr32(m, F3_A5 + 0x0906u, a1);
        r->a[1] = a1;
    }

    return 0;
}

/* ROM 0x178BB4 / 0x178DB6 + Obj_Script10 fallback for 0x196C86's script. */
static int script10_step_196C86(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;

    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);

        if (fn == 0u)
            goto loop_marker;

        if (fn == 0x00178B20u) {
            Obj_Script10_SetHeading(m, obj);
        } else if (fn == 0x00178BB4u) {
            wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
        } else if (fn == 0x00178E4Au) {
            Obj_Script10_SteerTo(m, obj);
        } else if (fn == 0x00178DB6u) {
            uint16_t d1w = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
            uint16_t r_in = rd8(m, a1 + 9u);
            uint8_t d0b = (uint8_t)(Obj_RandRange(m, r_in) + rd8(m, obj + 0x20u));
            wr8(m, obj + 0x20u, d0b);
            wr16(m, obj + 0x42u, d1w);
            int16_t mag = (int16_t)(uint16_t)(d1w + 0x0020u) >> 6;
            uint32_t base = 0x00027E40u + (uint32_t)d0b * 2u;
            int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
            int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
            wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
            wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
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
            if ((int32_t)rd32(m, a1) > 0)
                goto store;
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }

loop_marker: {
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
    }

store:
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    return 0;
}

int Fn_196C86(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x196C86..0x196C8A */
        uint32_t d0 = sub_17B02E(m, obj, &r->a[1]);     /* 0x196B84 */
        if ((int32_t)d0 >= 0) {
            uint32_t a1 = d0;
            r->a[1] = a1;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u); /* 0x196B92 */
        }
        wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu); /* 0x196B9A */
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x20u)); /* 0x196BA2 {2:1} */
        wr16(m, F3_A5 + 0x4B20u, rd16(m, obj + 0x34u)); /* 0x196BA8 */
        wr32(m, F3_A5 + 0x4B1Cu, rd32(m, obj + 0x36u)); /* 0x196BAE */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u)); /* 0x196BB4 */
        r->d[0] = Obj_Spawn(m, obj, 0x00179D92u, 5u, 2u, 0u); /* 0x196BBA */
        wr16(m, obj + 0x06u, 2u);                       /* 0x196BD8 (beq folded: never taken) */
        wr32(m, obj + 0x00u, 0x00196BE8u);              /* 0x196BE0 */
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u) {      /* 0x196C8E: {0xe:1} */
        int8_t d0b = 4;
        if ((int8_t)rd8(m, obj + 0x20u) >= 0)
            d0b = -4;
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)d0b;
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)d0b));
    }

    {
        uint32_t tgt = rd32(m, obj + 0x4Cu);            /* 0x196CAA */
        r->d[0] = tgt;
        if ((int32_t)tgt > 0) {
            r->a[1] = tgt;
            uint16_t dx = (uint16_t)(rd16(m, tgt + 0x14u) + 0x18u - rd16(m, obj + 0x50u));
            uint16_t dy = (uint16_t)(rd16(m, tgt + 0x18u) + 0x10u - rd16(m, obj + 0x54u));
            r->d[1] = setw(r->d[1], dy);
            int16_t dist = (int16_t)Obj_Distance(m, dx, dy); /* 0x196CCC */
            if (dist < 0)
                dist = (int16_t)-dist;
            r->d[0] = setw(r->d[0], (uint16_t)dist);

            if (dist >= 0x0050) {                       /* 0x196CDA */
                if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x08u) != 0u) { /* 0x196CE2: {0xc:1} */
                    uint32_t d0 = sub_0CC2BC(m, 0x0014u, &r->d[1]);
                    r->d[0] = d0;
                    if ((uint16_t)d0 == 0u) {
                        r->d[1] = rd8(m, obj + 0x20u);
                        r->d[0] = Obj_Spawn(m, obj, 0x0018339Cu, 9u, 2u, r->d[1]);
                    }
                }
                if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x04u) != 0u) { /* 0x196D1C: {0xd:1} */
                    uint32_t d0 = sub_0CC2BC(m, 0x0014u, &r->d[1]);
                    r->d[0] = d0;
                    if ((uint16_t)d0 == 0u) {
                        r->d[1] = rd8(m, obj + 0x20u);
                        r->d[0] = Obj_Spawn(m, obj, 0x00180F24u, 9u, 2u, r->d[1]);
                    }
                }
            }
        }
    }

    if (script10_step_196C86(m, obj) < 0)               /* 0x196D56 */
        return -1;
    if (sub_196AE0(m, r) < 0)                           /* 0x196D5C */
        return -1;

    wr32(m, obj + 0x00u, 0x00196C86u);                  /* 0x196D60 -> 0x196C7E */
    return 0;
}
