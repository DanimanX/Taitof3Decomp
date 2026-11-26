/* ROM 0x13BABE: boss/enemy homing-or-burst projectile phase tick (A0 = object).
 * Decrements +0x22.w; while > 0 animates via 0x13BC34, checks bomb/HP/clear,
 * moves along +0x20, flashes/draws (0x11C32C, 0x001282), appends to collision
 * lists (0x0CBDA4), and re-arms 0x13BABE. When +0x22.w <= 0, advances +0x21.b
 * by 0x80, resets +0x22.w = 0x1400 and +0x3C.w = 0, animates/checks bomb/HP,
 * sets speed +0x42 = 0x14, moves/draws/collides, spawns 3x 0x000CCD5C if
 * +0x3C.b < 3 and +0x3D.b == 0, adds 0x80 to +0x3C.w, and arms 0x13BA06. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
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

/* ROM 0x0CBDA4: append obj to both A5+0x8FE and A5+0x906. */
static uint32_t sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    return a1;
}

/* ROM 0x0CBD12 -> 0x0CBB2C: award +0x4C score to A5+0x1D50 and explode. */
static void bomb_explode_0CBD12(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    Obj_ExplodeTick(m, obj);
    if (rd32(m, obj) == 0x000CBC78u) {
        draw_sprite_regs(m, r);
    } else {
        r->a[1] = rd32(m, obj + 0x10u);
        r->d[0] = setw(r->d[0], rd16(m, obj + 0x2Eu));
        r->d[1] = setw(r->d[1], (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u));
    }
}

int Fn_13BABE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t t = (uint16_t)(rd16(m, obj + 0x22u) - 1u);       /* 0x13BABE */
    wr16(m, obj + 0x22u, t);
    if ((int16_t)t > 0) {                                     /* 0x13BAC2 -> 0x13BA5A */
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)          /* 0x13BA5A */
            goto explode_free;
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {             /* 0x13BA64 */
            uint32_t a1 = Obj_0CBDDE(m, obj);                 /* 0x13BBE4 */
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            goto explode_free;
        }
        Obj_Animate(m, obj, 0x0013BC34u);                     /* 0x13BA6C..0x13BA70 */
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u           /* 0x13BA76 */
            && (int16_t)rd16(m, obj + 0x14u) >= -0x000A
            && (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));         /* 0x13BA8E */
            bomb_explode_0CBD12(m, r);                        /* 0x13BA94 */
            return 0;
        }
        Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));        /* 0x13BA9A..0x13BA9E */
        Obj_11C32C(m, obj);                                   /* 0x13BAA4 */
        draw_sprite_regs(m, r);                               /* 0x13BAAA */
        r->a[1] = sub_0CBDA4(m, obj);                         /* 0x13BAB0 */
        wr32(m, obj, 0x0013BABEu);                            /* 0x13BAB6 */
        return 0;
    }

    /* 0x13BAC4 -> 0x13B91C */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + 0x80u)); /* 0x13BAC4 */
    wr16(m, obj + 0x22u, 0x1400u);                            /* 0x13B91C */
    wr16(m, obj + 0x3Cu, 0u);                                 /* 0x13B922 */

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)              /* 0x13B926 */
        goto explode_free;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                 /* 0x13B930 */
        uint32_t a1 = Obj_0CBDDE(m, obj);                     /* 0x13BBE4 */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto explode_free;
    }
    Obj_Animate(m, obj, 0x0013BC34u);                         /* 0x13B938..0x13B93C */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u               /* 0x13B942 */
        && (int16_t)rd16(m, obj + 0x14u) >= -0x000A
        && (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));             /* 0x13B95A */
        bomb_explode_0CBD12(m, r);                            /* 0x13B960 */
        return 0;
    }

    {
        int16_t spd = (int16_t)(int8_t)rd8(m, obj + 0x22u);   /* 0x13B966..0x13B96A */
        wr16(m, obj + 0x42u, (uint16_t)spd);                  /* 0x13B96C */
    }
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));            /* 0x13B970..0x13B974 */
    Obj_11C32C(m, obj);                                       /* 0x13B97A */
    draw_sprite_regs(m, r);                                   /* 0x13B980 */
    r->a[1] = sub_0CBDA4(m, obj);                             /* 0x13B986 */

    if ((int8_t)rd8(m, obj + 0x3Cu) < 3) {                    /* 0x13B98C..0x13B992 */
        if (rd8(m, obj + 0x3Du) == 0u) {                      /* 0x13B996..0x13B99A */
            (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x1020A000u);     /* 0x13B99E */
            (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x10208000u);     /* 0x13B9BC */
            r->d[0] = Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x10206000u); /* 0x13B9DA */
        }
        wr16(m, obj + 0x3Cu, (uint16_t)(rd16(m, obj + 0x3Cu) + 0x0080u)); /* 0x13B9F8 */
    }
    wr32(m, obj, 0x0013BA06u);                                /* 0x13B9FE */
    return 0;

explode_free:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20180002u); /* 0x13BBF4 */
    Sound_PlayHooked(m, 0x0035u);                             /* 0x13BC12 */
    Obj_Free(m, obj);                                         /* 0x13BC26 */
    return 0;
}
