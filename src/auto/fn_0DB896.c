/* ROM 0x0DB896: enemy object tick (A0 = obj).
 * Handles debug on-screen kill (0x0CBB2C), HP <= 0 / bomb-clear death
 * (spawning 0x0DBDA2), 5-state machine (+0x06: 0=emerge from parent +0x08,
 * 1=descend/align with player Y, 2/3=split-sprite open & fire, 4=fly out),
 * off-screen flag (0x0CF85A), terrain check (0x0C6632), hazard (0x0D0DCC)
 * and sprite draw (0x001282, skipped in states 2 & 3). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D0CB0: difficulty speed offset from table 0x0D0D82. */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D82u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D2D44: scale (0x0D2D64[angle], 0x0D2ECC[angle]) by speed (0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
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

/* ROM 0x0C6632 wrapper tracking D0/D1 register state. */
static uint8_t terrain_test_regs(const f3mem_t *m, int16_t x, int16_t y,
                                 uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t hit = (uint8_t)Terrain_TestPoint(m, x, y);
    if (rd8(m, F3_A5 + 0x1D1Cu) != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u);
        return 0u;
    }
    uint32_t d0 = (uint32_t)(((uint16_t)((uint16_t)x + rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x3FFu);
    uint32_t d1 = (uint32_t)(((uint16_t)((uint16_t)y + rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x3Fu);
    d1 = (uint32_t)((int32_t)(int16_t)d1 * 0x280);
    d0 += d1;
    *d1_io = d1;
    *d0_io = (d0 & 0xFFFFFF00u) | (hit ? 0xFFu : 0x00u);
    return hit;
}

int Fn_0DB896(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    /* 0x0DB896: debug on-screen kill */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_OrbitDebug(m, obj);                           /* 0x0DB8AE -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0DB8B4: HP <= 0 or bomb-clear */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        d0 = Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);      /* 0x0DB92E */
        Obj_StartDying(m, obj);                               /* 0x0DB94A -> 0x0CEE74 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    switch (rd8(m, obj + 0x06u)) {                            /* 0x0DB8CC -> table 0x0DB950 */
    case 0: {                                                 /* 0x0DB964 */
        a1 = rd32(m, obj + 0x08u);
        if (rd8(m, a1 + 0x06u) == 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
            break;
        }
        wr32(m, obj + 0x3Au, rd32(m, a1 + 0x14u));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x58u)));
        wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x60u)));
        uint8_t e = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x10u);
        wr8(m, obj + 0x1Eu, e);
        d1 = (uint32_t)(e >> 4);
        d0 = 0x10u - d1;
        wr16(m, obj + 0x32u, (uint16_t)d0);
        if (e == 0u) {
            wr16(m, obj + 0x32u, 0x000Du);
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    }
    case 1: {                                                 /* 0x0DB9C2 */
        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);
        d0 = rd32(m, obj + 0x50u);
        uint32_t v = rd32(m, obj + 0x3Au) + d0;
        wr32(m, obj + 0x3Au, v);
        if ((int32_t)v < 0)
            break;

        uint32_t target = 0u;
        a1 = 0x0041B190u;
        if ((int16_t)rd16(m, a1 + 0x4Cu) >= 0) {
            int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a1 + 0x18u) - 7);
            if (dy < 0) dy = (int16_t)(-dy);
            d0 = (d0 & 0xFFFF0000u) | (uint16_t)dy;
            if ((uint16_t)dy <= 9u)
                target = a1;
        }
        if (target == 0u) {
            a1 = 0x0041B21Au;
            if ((int16_t)rd16(m, a1 + 0x4Cu) >= 0) {
                int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a1 + 0x18u) - 7);
                if (dy < 0) dy = (int16_t)(-dy);
                d0 = (d0 & 0xFFFF0000u) | (uint16_t)dy;
                if ((uint16_t)dy <= 9u)
                    target = a1;
            }
        }
        if (target != 0u) {
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
            int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - rd16(m, target + 0x14u) - 0x10);
            d0 = (d0 & 0xFFFF0000u) | (uint16_t)dx;
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            draw_sprite_regs(m, r);
            d0 = r->d[0];
            d1 = r->d[1];
            a1 = r->a[1];
            wr8(m, obj + 0x06u, (dx >= 0) ? 2u : 3u);
        }
        break;
    }
    case 2: {                                                 /* 0x0DBA62 */
        d0 = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        uint8_t e = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u);
        wr8(m, obj + 0x1Eu, e);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (e >> 4)));
        wr32(m, obj + 0x10u, 0x0005A588u);
        Obj_DrawSprite(m, obj);

        uint8_t f = (uint8_t)~rd8(m, obj + 0x1Eu);
        wr8(m, obj + 0x1Fu, f);
        wr8(m, obj + 0x1Eu, 0u);
        d1 = (uint32_t)(f >> 4);
        d0 = (uint32_t)((uint16_t)(0x10u - d1 - 9u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)d0));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
        wr32(m, obj + 0x10u, 0x0005A5C8u);
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        draw_sprite_regs(m, r);
        d0 = r->d[0];
        d1 = r->d[1];
        a1 = r->a[1];

        uint8_t d0b = (uint8_t)~rd8(m, obj + 0x1Fu);
        wr8(m, obj + 0x1Eu, d0b);
        wr8(m, obj + 0x1Fu, 0u);
        d0 = (d0 & 0xFFFFFF00u) | d0b;
        if (d0b == 0xF0u) {
            uint16_t spd = (uint16_t)(sub_0D0CB0(m) + 0x0014u);
            sub_0D2D44(m, 0x00B4u, spd, &d0, &d1);
            wr32(m, obj + 0x50u, d0);
            wr32(m, obj + 0x54u, d1);
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 8u));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 8u));
            wr8(m, obj + 0x1Eu, 0u);
            wr8(m, obj + 0x06u, 4u);
            wr16(m, obj + 0x2Cu, 2u);
            wr16(m, obj + 0x2Eu, 0x000Du);
            wr16(m, obj + 0x30u, 6u);
            wr16(m, obj + 0x32u, 9u);
        }
        break;
    }
    case 3: {                                                 /* 0x0DBB34 */
        d0 = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        uint8_t e = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u);
        wr8(m, obj + 0x1Eu, e);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (e >> 4)));
        wr32(m, obj + 0x10u, 0x0005A588u);
        Obj_DrawSprite(m, obj);

        uint8_t f = (uint8_t)~rd8(m, obj + 0x1Eu);
        wr8(m, obj + 0x1Fu, f);
        wr8(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 7u));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u));
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));
        wr32(m, obj + 0x10u, 0x0005A5C8u);
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        draw_sprite_regs(m, r);
        d0 = r->d[0];
        d1 = r->d[1];
        a1 = r->a[1];

        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) & ~0x02u));
        uint8_t d0b = (uint8_t)~rd8(m, obj + 0x1Fu);
        wr8(m, obj + 0x1Eu, d0b);
        wr8(m, obj + 0x1Fu, 0u);
        d0 = (d0 & 0xFFFFFF00u) | d0b;
        if (d0b == 0xF0u) {
            uint16_t spd = (uint16_t)(sub_0D0CB0(m) + 0x0014u);
            sub_0D2D44(m, 0u, spd, &d0, &d1);
            wr32(m, obj + 0x50u, d0);
            wr32(m, obj + 0x54u, d1);
            wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 7u));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 8u));
            wr8(m, obj + 0x1Eu, 0u);
            wr8(m, obj + 0x06u, 4u);
            wr16(m, obj + 0x2Cu, 2u);
            wr16(m, obj + 0x2Eu, 0x000Du);
            wr16(m, obj + 0x30u, 6u);
            wr16(m, obj + 0x32u, 9u);
        }
        break;
    }
    case 4:                                                   /* 0x0DBC04 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);
        break;
    default:
        break;
    }

    Obj_FlagOffscreen(m, obj);                                /* 0x0DB8D4 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {                /* 0x0DB8DA */
        Obj_StartDying(m, obj);
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    {
        int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 7);
        int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 8);
        if (terrain_test_regs(m, tx, ty, &d0, &d1) != 0u) {   /* 0x0DB8F2 */
            Obj_StartDying(m, obj);
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));               /* 0x0DB900 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));               /* 0x0DB906 */
    Coll_AddHazard(m, obj);                                   /* 0x0DB90C -> 0x0D0DCC */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    {
        uint8_t st = rd8(m, obj + 0x06u);
        if (st != 2u && st != 3u)
            draw_sprite_regs(m, r);                           /* 0x0DB926 -> 0x001282 */
    }
    return 0;
}
