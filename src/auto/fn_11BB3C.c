/* ROM 0x11BB3C: homing/turret enemy tick (A0 = object, +0x0C = target).
 * On +0x24 <= 0, awards +0x4C to score receiver (0x0CBDDE), plays sound
 * 0x35, shifts (+8, +8), and jumps to 0x0CD3D6 (dying/transform sequence #2).
 * On A5-0x144B bit 4 set, also jumps to sound 0x35 + 0x0CD3D6. When screen
 * bomb/flash A5+0x1D58 bit 1 is set and X in [-10, 0x11C], awards +0x4C to
 * A5+0x1D50->+0x48 and runs 0x0CBB2C. Otherwise aims +0x20 toward +0x0C via
 * 0x0CC3C6, animates table 0x11BC48 via 0x0CBD52, after +0x06 >= 0x28 rolls
 * 0x0CC2DE(0x26) to spawn 0x0CD124, appends collision lists (0x0CBDA4), draws
 * sprite (0x001282), and re-arms 0x11BB3C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x001282 register outputs after Obj_DrawSprite(m, obj) */
static void draw_sprite_post_regs(const f3mem_t *m, uint32_t obj,
                                  uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);
    *a1_out = rd32(m, SPR_CURSOR);
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

/* ROM 0x00649A with full 32-bit D0 output (high word from swap D0) */
static uint32_t dist_full(const f3mem_t *m, int16_t dx, int16_t dy)
{
    uint32_t d0 = (uint32_t)((int32_t)dx * (int32_t)dx)
                + (uint32_t)((int32_t)dy * (int32_t)dy);
    d0 += d0;
    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }
    uint16_t v = rd16(m, 0x00028340u + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 2u);
    uint32_t r = (uint32_t)v << (shift & 63u);
    return (r << 16) | (r >> 16);
}

/* ROM 0x0CC2DE: D0 = (rem << 16) | quot, D1 = divisor */
static uint16_t sub_0CC2DE(const f3mem_t *m, uint16_t d0w,
                           uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t rng = Rng_NextMem(m) & 0x0000FFFFu;
    uint16_t div = (uint16_t)d1;
    uint16_t q = (uint16_t)(rng / div);
    uint16_t rem = (uint16_t)(rng % div);
    *d0_out = ((uint32_t)rem << 16) | q;
    *d1_out = d1;
    return q;
}

int Fn_11BB3C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x11BB3C..0x11BB40 */
        a1 = Obj_0CBDDE(m, obj);                         /* 0x11BBE0 */
        d0 = rd16(m, obj + 0x4Cu);                       /* 0x11BBE6..0x11BBE8 */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);   /* 0x11BBEC */
        goto die_transform;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)         /* 0x11BB44..0x11BB4A */
        goto die_transform;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x11BB4E */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                 /* 0x11BB56..0x11BB64 */
            /* 0x11BB66 -> jmp 0x000CBD12 */
            a1 = rd32(m, F3_A5 + 0x1D50u);               /* 0x0CBD12 */
            d0 = rd16(m, obj + 0x4Cu);                   /* 0x0CBD16..0x0CBD18 */
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0); /* 0x0CBD1C */
            Obj_ExplodeTick(m, obj);                     /* 0x0CBD20 -> 0x0CBB2C */
            if (rd32(m, obj + 0x00u) == 0x000CBC78u) {
                draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
            } else {
                /* 0x0CBB98 blt 0x0CBC80: +0x14/+0x18 already adjusted by 0x0CBB2C */
                int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
                int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - 0x0074u);
                uint32_t df = dist_full(m, dx, dy);
                uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
                d0 = setw(df, (uint16_t)(((uint16_t)df << 2) - thresh));
                d1 = setw(d1, (uint16_t)dy);
                a1 = rd32(m, obj + 0x10u) + 4u;
            }
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    {
        uint32_t tgt = rd32(m, obj + 0x0Cu);             /* 0x11BB6C */
        int16_t dy = (int16_t)(((int16_t)rd16(m, tgt + 0x30u) + (int16_t)rd16(m, tgt + 0x32u)
                                - (int16_t)rd16(m, obj + 0x30u) - (int16_t)rd16(m, obj + 0x32u)) >> 1);
        dy = (int16_t)(dy + (int16_t)rd16(m, tgt + 0x18u) - (int16_t)rd16(m, obj + 0x18u));
        d1 = setw(d1, (uint16_t)dy);
        uint16_t ang = Obj_0CC3C6(m, obj, tgt);          /* 0x11BB70 */
        ang = (uint16_t)((ang & 0xFF00u) | (uint8_t)(ang + 8u)); /* 0x11BB76 */
        wr16(m, obj + 0x20u, ang);                       /* 0x11BB78 */
    }

    Obj_Animate(m, obj, 0x0011BC48u);                    /* 0x11BB7C..0x11BB82 */
    d0 = setw(d0, rd16(m, obj + 0x3Au));
    d1 = setw(d1, rd16(m, obj + 0x3Eu));

    {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) + 1u); /* 0x11BB88 */
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t >= 0x0028) {                      /* 0x11BB8C..0x11BB92 */
            d0 = setw(d0, 0x0026u);                      /* 0x11BB96 */
            if (sub_0CC2DE(m, 0x0026u, &d0, &d1) == 0u) { /* 0x11BB9A..0x11BBA2 */
                d1 = 0x10100000u | (uint16_t)((rd16(m, obj + 0x20u) << 8) & 0xF000u); /* 0x11BBA6..0x11BBB2 */
                d0 = Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, d1); /* 0x11BBB6..0x11BBCC */
            }
        }
    }

    {
        /* 0x11BBD0 jsr 0x000CBDA4 */
        uint32_t p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x08FEu, p + 4u);
        p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        p += 4u;
        wr32(m, F3_A5 + 0x0906u, p);
        a1 = p;
    }

    Obj_DrawSprite(m, obj);                              /* 0x11BBD6 */
    draw_sprite_post_regs(m, obj, &d0, &d1, &a1);
    wr32(m, obj + 0x00u, 0x0011BB3Cu);                   /* 0x11BBDC -> 0x11BB34 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

die_transform:
    Sound_PlayHooked(m, 0x0035u);                        /* 0x11BBF0..0x11BC00 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u)); /* 0x11BC04 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u)); /* 0x11BC08 */
    wr16(m, obj + 0x42u, 0u);                            /* 0x11BC10 */
    /* 0x11BC14 jmp 0x000CD3D6 with D0w = 2 */
    a1 = rd32(m, 0x000CD444u + 2u * 4u);                 /* 0x000CD4D0 */
    {
        uint16_t snd = rd16(m, a1);                      /* 0x0CD3DE */
        a1 += 2u;
        d0 = setw(d0, snd);
        if (snd != 0u) {
            d0 = setw(d0, (uint16_t)Sound_PreCmdHook(m, (int16_t)snd));
            Sound_PlayCmd(m, (int16_t)d0);
        }
    }
    wr32(m, obj + 0x0Cu, a1);                            /* 0x0CD3F6 */
    wr16(m, obj + 0x3Au, 0u);                            /* 0x0CD3FA */
    wr16(m, obj + 0x3Eu, 0u);                            /* 0x0CD3FE */
    wr16(m, obj + 0x1Eu, 0u);                            /* 0x0CD402 */
    wr16(m, obj + 0x1Cu, 0u);                            /* 0x0CD406 */
    wr32(m, obj + 0x00u, 0x000CD412u);                   /* 0x0CD40A */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
