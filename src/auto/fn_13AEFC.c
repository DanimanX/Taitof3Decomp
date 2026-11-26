/* ROM 0x13AEFC: boss/enemy tracking turret tick (A0 = object).
 * Frees if bit4(A5-0x144B) or A5+0x32D6 >= 3. While +0x24 > 0, steers +0x20
 * toward A5+0x30E2 by +-0x200 clamped unsigned to [0x6000, 0xA000], picks one
 * of 9 sprites from 0x13B090, updates collision & draws (0x11B06E, 0x11B09A),
 * optionally fires 0x0CD124 shot with sound 0x2A via 0x0CC2BC and 0x0CC290,
 * and re-arms 0x13AEFC. On +0x24 <= 0, awards 0x7530 score (0x0CBDDE), spawns
 * 0x13E4EE with sound 0x35, sets +0x06 countdown via 0x0CBEB0, and advances to
 * 0x13B058 (or 0x13AD8A if 0). */
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

/* ROM 0x11B09A with exact D0/D1/A1 output for ALLREGS. */
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

/* ROM 0x0CC2BC: D0 = (rem << 16) | quot, D1 = swapped scaled period. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w, uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = (d1 & 0xFFFF0000u) | 1u;
    uint32_t d0 = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint16_t div = (uint16_t)d1;
    uint16_t quot = (uint16_t)(d0 / div);
    uint16_t rem = (uint16_t)(d0 % div);
    *d0_out = ((uint32_t)quot << 16) | rem;
    *d1_out = d1;
}

int Fn_13AEFC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u
        || (int16_t)rd16(m, F3_A5 + 0x32D6u) >= 3) {    /* 0x13AEFC..0x13AF0C */
        Obj_Free(m, obj);
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x13AF10 */
        uint32_t a1 = Obj_0CBDDE(m, obj);                /* 0x13AFFA */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u);
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u);
        Sound_PlayHooked(m, 0x0035u);
        /* 0x13B03A..0x13B04A: 0x0CBEB0(0x64) */
        uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
        uint32_t d0 = (uint32_t)((int32_t)0x0064 * (int32_t)(int16_t)d1w) << 4;
        d0 = (d0 << 16) | (d0 >> 16);
        uint16_t d0w = (uint16_t)(0u - (int16_t)(uint16_t)((uint16_t)d0 - 0x00C8u));
        wr16(m, obj + 0x06u, d0w);
        if (d0w == 0u)
            wr32(m, obj, 0x0013AD8Au);                   /* 0x13B05E -> 0x13AD82 */
        else
            wr32(m, obj, 0x0013B058u);
        r->d[0] = setw(d0, d0w);
        r->d[1] = setw(r->d[1], d1w);
        r->a[1] = a1;
        return 0;
    }

    /* 0x13AF18..0x13AF60: aim +0x20 toward A5+0x30E2, clamped unsigned [0x6000, 0xA000] */
    {
        uint32_t tgt = rd32(m, F3_A5 + 0x30E2u);
        uint16_t ang = Obj_0CC3C6(m, obj, tgt);
        int16_t diff = (int16_t)((uint16_t)(ang << 8) - rd16(m, obj + 0x20u));
        if (diff > 0x0200)
            diff = 0x0200;
        else if (diff < -0x0200)
            diff = -0x0200;
        uint16_t v = (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)diff);
        if (v > 0xA000u)                                 /* 0x13AF40 cmpi.w #-0x6000 ; bls */
            v = 0xA000u;
        if (v < 0x6000u)                                 /* 0x13AF50 cmpi.w #0x6000 ; bcc */
            v = 0x6000u;
        wr16(m, obj + 0x20u, v);

        uint16_t idx = (uint16_t)(((uint16_t)(v - 0x5C00u) >> 11) & 0x1Fu);
        wr32(m, obj + 0x10u, rd32(m, 0x0013B090u + (uint32_t)idx * 4u));
    }

    Obj_11B06E(m, obj);                                  /* 0x13AF76 */
    sub_11B09A(m, r);                                    /* 0x13AF7C */

    /* 0x13AF82: bftst (0x30ee, A5){6:1} = bit 1 (0x02) of byte A5+0x30EE */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x02u) != 0u) {
        uint32_t d0, d1;
        sub_0CC2BC(m, 0x0008u, &d0, &d1);                /* 0x13AF90 */
        r->d[0] = d0;
        r->d[1] = d1;
        if ((uint16_t)d0 == 0u) {
            d0 = (d0 << 16) | (d0 >> 16);                /* 0x13AF9C swap D0 */
            d0 = setw(d0, (uint16_t)(d0 & 0x000Fu));
            r->d[0] = d0;
            if ((int16_t)(uint16_t)d0 <= 7) {            /* 0x13AFA2..0x13AFA6 */
                /* 0x13AFAA..0x13AFF2: compute packed spawn arg via 0x0CC290 */
                uint16_t ang = (uint16_t)(((uint8_t)((rd16(m, obj + 0x20u) >> 8) + 4u)) & 0xF8u);
                int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
                int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
                uint16_t ox = (uint16_t)((uint32_t)(0x18 * (int32_t)cos_v << 2) >> 16);
                uint16_t oy = (uint16_t)((uint32_t)(0x18 * (int32_t)sin_v << 2) >> 16);
                uint32_t d2 = ((uint32_t)(ox & 0xFFu) << 24)
                            | ((uint32_t)(oy & 0xFFu) << 16)
                            | ((uint32_t)ang << 8);
                r->d[2] = d2;
                r->d[0] = Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, d2);
                r->d[1] = setw(d1, oy);
                r->a[1] = 0x00027E40u;
                Sound_PlayHooked(m, 0x002Au);
            }
        }
    }

    wr32(m, obj, 0x0013AEFCu);                           /* 0x13AEF4 */
    return 0;
}
