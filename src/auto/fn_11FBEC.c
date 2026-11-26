/* ROM 0x11FBEC: boss core/body tick and attack/phase spawner (A0 = object).
 * Frees self (0x0105EC) if parent (+0x08)->+0x59 != 0. Selects sprite/hitbox
 * and offsets from descriptor +0x0C via A5+0x3262 or A5+0x30EE flags
 * (0x11FE2A -> 0x0CBD24 -> 0x12048C). On HP (+0x24) <= 0 sets A5+0x30EE bit 5,
 * sets +0x10=0x28740, and once plays sound 0x16, spawns 0x13E4EE explosion,
 * awards 0x7530 via 0x0CBDDE, and adds 0x12C5F to A5+0x886. Mirrors parent
 * +0x1D if +0x34 != 0 else sets +0x34 bit 4 and calls 0x11B06E; draws sprite
 * via 0x11B09A. Polls A5+0x30EE/30EF flags to spawn 0x11B78C, 0x120A9C (snd 0x3C),
 * pull players (0x11FF4A), spawn 0x120CC8, 0x120DF6, 0x120BD6 (snd 0x3C),
 * and re-arms 0x11FBEC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_out = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x11B06E with A1 register tracking. */
static void sub_11B06E_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        a1 += 4u;
        wr32(m, F3_A5 + 0x0906u, a1);
        *a1_io = a1;
    }
    Obj_11C32C(m, obj);
}

/* ROM 0x11B09A with exact D0/D1/A1 register outputs. */
static void sub_11B09A_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, x);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, y);
    *d0_io = setw(*d0_io, y);

    if ((int16_t)y < 0x00E8 && (int16_t)x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t w = (uint16_t)(rd16(m, a1) << 4);
        a1 += 2u;
        *d0_io = setw(*d0_io, w);
        *a1_io = a1;
        if ((int16_t)(w + y) > 0) {
            uint16_t h = (uint16_t)(rd16(m, a1) << 4);
            a1 += 2u;
            *d0_io = setw(*d0_io, h);
            *a1_io = a1;
            if ((int16_t)(h + x) > 0)
                draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t offy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - offy));
    *d0_io = setw(*d0_io, offy);
}

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem, D1 = swapped scaled period (or 1). */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1 = swap32(d1);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t div = (uint16_t)d1;
    uint16_t q = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    *d0_out = ((uint32_t)q << 16) | rem;
    *d1_out = d1;
}

/* ROM 0x11FE2A -> 0x12048C: select animation/sprite/hitbox record */
static void sub_11FE2A(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                     /* 0x11FE2A */
    uint32_t d0 = rd8(m, F3_A5 + 0x3262u);                  /* 0x11FE2E..0x11FE30 */
    if (d0 != 0u) {                                         /* 0x11FE34 */
        wr8(m, obj + 0x34u, 0xFFu);                         /* 0x11FE38 */
        a1 = rd32(m, a1 - 4u + d0 * 4u);                    /* 0x11FE3C */
    } else {
        a1 += 0x10u;                                        /* 0x11FE44 */
        wr32(m, obj + 0x10u, rd32(m, a1 + 2u));             /* 0x11FE48 */
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) != 0u) {      /* 0x11FE4E */
            wr8(m, obj + 0x34u, 0xFFu);                     /* 0x11FE98 */
            a1 = rd32(m, obj + 0x10u);                      /* 0x11FE9C */
        } else if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) { /* 0x11FE58 */
            Obj_AnimFrameStep(m, obj, a1);                  /* 0x11FE62 */
            uint16_t off = rd16(m, obj + 0x3Au);
            d0 = setw(d0, off);
            int16_t dur = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)off); /* 0x11FE68 */
            if (dur <= 0) {
                if (dur < 0) {
                    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0xC0u)); /* 0x11FE86..0x11FE8C */
                } else if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) != 0u) { /* 0x11FE74 */
                    wr16(m, obj + 0x3Au, (uint16_t)(off + 2u)); /* 0x11FE7E */
                }
                wr32(m, obj + 0x10u, rd32(m, a1 - 4u + (uint32_t)(int32_t)(int16_t)off)); /* 0x11FE92 */
            }
            wr8(m, obj + 0x34u, 0xFFu);                     /* 0x11FE98 */
            a1 = rd32(m, obj + 0x10u);                      /* 0x11FE9C */
        } else {
            wr8(m, obj + 0x34u, 0u);                        /* 0x11FEA4 */
            a1 = rd32(m, a1 + 2u);                          /* 0x11FEA8 */
            wr16(m, obj + 0x3Au, 0u);                       /* 0x11FEAC */
            wr16(m, obj + 0x3Eu, 0u);                       /* 0x11FEB0 */
        }
    }

    /* 0x11FEB4 -> 0x12048C */
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    int16_t last_b = (int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x32u, (uint16_t)last_b);
    *d0_io = setw(d0, (uint16_t)last_b);
    *a1_io = a1;
}

/* ROM 0x11FF4A: pull active player ships toward boss core */
static void sub_11FF4A(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u)           /* 0x11FF4A */
        return;

    uint32_t a1 = 0x00409D2Cu;                              /* 0x11FF54 */
    int16_t cnt = (int16_t)rd16(m, F3_A5 + 0x1D34u);        /* 0x11FF60 */
    while (--cnt >= 0) {                                    /* 0x11FF64 */
        uint32_t p = rd32(m, a1); a1 += 4u;                 /* 0x11FF6A */
        int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) + 8); /* 0x11FF6C..0x11FF74 */
        int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, p + 0x18u) + 0x0C); /* 0x11FF76..0x11FF7E */
        uint16_t ang = Obj_Atan2(m, (uint16_t)dx, (uint16_t)dy); /* 0x11FF82 */
        uint32_t d0 = Rng_NextMem(m);                       /* 0x11FF8A */
        uint16_t spd = (uint16_t)((d0 & 7u) + 6u);          /* 0x11FF90..0x11FF94 */
        int32_t vx = (int32_t)(int16_t)spd * (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang); /* 0x11FF98 */
        int32_t vy = (int32_t)(int16_t)spd * (int32_t)(int16_t)rd16(m, 0x00027E40u + 2u * ang); /* 0x11FFA2 */
        wr32(m, p + 0x14u, rd32(m, p + 0x14u) + (uint32_t)vx); /* 0x11FF9E */
        wr32(m, p + 0x18u, rd32(m, p + 0x18u) + (uint32_t)vy); /* 0x11FFA8 */
        *d0_io = (uint32_t)vx;
        *d1_io = (uint32_t)vy;
    }
    *a1_io = a1;
}

int Fn_11FBEC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                 /* 0x11FBEC */
    r->a[1] = parent;

    if (rd8(m, parent + 0x59u) != 0u) {                     /* 0x11FBF0 */
        Obj_Free(m, obj);                                   /* 0x11FE1C */
        return 0;
    }

    sub_11FE2A(m, obj, &r->d[0], &r->a[1]);                 /* 0x11FBF8 */

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* 0x11FBFC */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x20u)); /* 0x11FDB2 */
        wr32(m, obj + 0x10u, 0x00028740u);                  /* 0x11FDB8 */
        if (rd8(m, obj + 0x22u) == 0u) {                    /* 0x11FDC0 */
            Sound_PlayHooked(m, 0x0016u);                   /* 0x11FDC8..0x11FDD8 */
            uint32_t arg = Obj_13E4D0(m, obj, 3u);          /* 0x11FDDC..0x11FDE4 */
            r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, arg); /* 0x11FDE4..0x11FDFA */
            wr8(m, obj + 0x22u, 0xFFu);                     /* 0x11FDFE */
            uint32_t recv = Obj_0CBDDE(m, obj);             /* 0x11FE02 */
            r->a[1] = recv;
            wr32(m, recv + 0x48u, rd32(m, recv + 0x48u) + 0x00007530u); /* 0x11FE08 */
            wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu); /* 0x11FE10 */
        }
        goto draw_and_attacks;                              /* 0x11FE18 */
    }

    if (rd8(m, obj + 0x34u) != 0u) {                        /* 0x11FC04 */
        r->a[1] = parent;                                   /* 0x11FC0C */
        wr8(m, obj + 0x1Du, rd8(m, parent + 0x1Du));        /* 0x11FC10 */
    } else {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x11FC1A */
        sub_11B06E_regs(m, obj, &r->a[1]);                  /* 0x11FC20 */
    }

draw_and_attacks:
    sub_11B09A_regs(m, obj, &r->d[0], &r->d[1], &r->a[1]);  /* 0x11FC26 */

    if ((rd8(m, F3_A5 + 0x30EFu) & 0x40u) != 0u) {          /* 0x11FC2C */
        sub_0CC2BC(m, 4u, &r->d[0], &r->d[1]);              /* 0x11FC3A */
        if ((uint16_t)r->d[0] == 0u) {                      /* 0x11FC40 */
            uint32_t d0 = Rng_NextMem(m);                   /* 0x11FC46 */
            d0 &= 0xFFFF003Fu;                              /* 0x11FC52 */
            d0 = (d0 & 0xFFFFFF00u) | (uint8_t)(d0 + 0x60u); /* 0x11FC56 */
            d0 = setw(d0, (uint16_t)((uint16_t)d0 << 8));   /* 0x11FC5A */
            uint32_t d1 = 0x10100000u | (uint16_t)d0;       /* 0x11FC4C..0x11FC5C */
            r->d[1] = d1;
            r->d[0] = Obj_Spawn(m, obj, 0x0011B78Cu, 5u, 2u, d1); /* 0x11FC6E */
        }
    }

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x01u) != 0u) {          /* 0x11FC78 */
        sub_0CC2BC(m, 2u, &r->d[0], &r->d[1]);              /* 0x11FC86 */
        if ((uint16_t)r->d[0] == 0u) {                      /* 0x11FC8C */
            uint32_t d0 = Rng_NextMem(m);                   /* 0x11FC92 */
            d0 &= 0xFFFF001Fu;                              /* 0x11FC9E */
            d0 = (d0 & 0xFFFFFF00u) | (uint8_t)(d0 - 0x60u); /* 0x11FCA2 */
            d0 = setw(d0, (uint16_t)((uint16_t)d0 << 8));   /* 0x11FCA6 */
            uint32_t d1 = 0x18100000u | (uint16_t)d0;       /* 0x11FC98..0x11FCA8 */
            r->d[1] = d1;
            r->d[0] = Obj_Spawn(m, obj, 0x00120A9Cu, 9u, 2u, d1); /* 0x11FCBA */
            Sound_PlayHooked(m, 0x003Cu);                   /* 0x11FCC4..0x11FCD4 */
        }
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) == 0u) {          /* 0x11FCD8 */
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x04u) != 0u)        /* 0x11FCE2 */
            sub_11FF4A(m, obj, &r->d[0], &r->d[1], &r->a[1]); /* 0x11FCEC */
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x02u) != 0u)        /* 0x11FCF0 */
            r->d[0] = Obj_Spawn(m, obj, 0x00120CC8u, 3u, 2u, 0u); /* 0x11FD0C */
    }

    {
        uint8_t f = rd8(m, F3_A5 + 0x30EFu);                /* 0x11FD16 */
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(f & ~0x20u));
        if ((f & 0x20u) != 0u) {
            uint32_t d0 = Rng_NextMem(m);                   /* 0x11FD20 */
            d0 = setw(d0, (uint16_t)(((uint16_t)d0 & 0x7F00u) + 0x5000u)); /* 0x11FD26..0x11FD2A */
            uint32_t d1 = 0x0E100000u | (uint16_t)d0;       /* 0x11FD2E..0x11FD34 */
            r->d[1] = d1;
            r->d[0] = Obj_Spawn(m, obj, 0x00120DF6u, 5u, 2u, d1); /* 0x11FD46 */
        }
    }

    if ((rd8(m, F3_A5 + 0x30EFu) & 0x80u) != 0u) {          /* 0x11FD50 */
        sub_0CC2BC(m, 2u, &r->d[0], &r->d[1]);              /* 0x11FD5E */
        if ((uint16_t)r->d[0] == 0u) {                      /* 0x11FD64 */
            uint32_t d0 = Rng_NextMem(m);                   /* 0x11FD6A */
            d0 = setw(d0, (uint16_t)(((uint16_t)d0 & 0x3F00u) - 0x7800u)); /* 0x11FD70..0x11FD74 */
            uint32_t d1 = 0x10140000u | (uint16_t)d0;       /* 0x11FD78..0x11FD7E */
            r->d[1] = d1;
            r->d[0] = Obj_Spawn(m, obj, 0x00120BD6u, 5u, 2u, d1); /* 0x11FD90 */
            Sound_PlayHooked(m, 0x003Cu);                   /* 0x11FD9A..0x11FDAA */
        }
    }

    wr32(m, obj + 0x00u, 0x0011FBECu);                      /* 0x11FDAE -> 0x11FBE4 */
    return 0;
}
