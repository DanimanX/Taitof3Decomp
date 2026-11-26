/* ROM 0x0D87F2: grid-aligned terrain-clearing enemy tick (A0 = object).
 * On HP <= 0 or bomb-clear, credits +0x4C to score receiver (0x0CBDDE), plays
 * sound (+0x65), spawns 0x13E4EE with 0x13E4D0 arg, and transitions to 0x0D89F2.
 * Otherwise animates (0x0CBD52), moves (0x0CBE12 + 0x0CBE00), clears collision
 * mask (0x0C66F4) and 16x16 tile in gfx RAM (0x610000), pushes to collision
 * lists (0x0CBDA4), flashes/draws (0x0CBE34 + 0x001282), frees if off-screen
 * (0x0D89F2), and when +0x06 <= 0 and aligned to tile center ([7..9] mod 16)
 * transitions to 0x0D8922. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

#define TBL_0D8A0C 0x000D8A0Cu
#define TBL_0D8A1A 0x000D8A1Au
#define MSK_0D8AB0 0x000D8AB0u

/* ROM 0x0C66F4: clear bitfield mask at A1 in collision bitmap 0x40891C. */
static void sub_0C66F4(const f3mem_t *m, uint16_t x, uint16_t y, uint32_t a1)
{
    uint16_t tx = (uint16_t)((uint16_t)(x + rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x03FFu;
    uint16_t ty = (uint16_t)((uint16_t)(y + rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x003Fu;
    uint32_t bitpos = (uint32_t)tx + (uint32_t)(int32_t)(int16_t)ty * 0x0280u;
    uint16_t rows = rd16(m, a1);
    a1 += 2u;

    for (int16_t r = (int16_t)(rows - 1u); r >= 0; r--) {
        uint32_t byte_addr = 0x0040891Cu + (bitpos >> 3);
        uint32_t shift = 8u - (bitpos & 7u);
        uint32_t w24 = ((uint32_t)rd8(m, byte_addr) << 16)
                     | ((uint32_t)rd8(m, byte_addr + 1u) << 8)
                     | (uint32_t)rd8(m, byte_addr + 2u);
        uint16_t val = (uint16_t)((w24 >> shift) & 0xFFFFu);
        val &= rd16(m, a1);
        a1 += 2u;
        uint32_t mask24 = 0xFFFFu << shift;
        w24 = (w24 & ~mask24) | (((uint32_t)val << shift) & mask24);
        wr8(m, byte_addr, (uint8_t)(w24 >> 16));
        wr8(m, byte_addr + 1u, (uint8_t)(w24 >> 8));
        if ((bitpos & 7u) != 0u)
            wr8(m, byte_addr + 2u, (uint8_t)w24);
        bitpos += 0x0280u;
    }
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
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

int Fn_0D87F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                 /* 0x0D87F2 */
        uint32_t recv = Obj_0CBDDE(m, obj);                   /* 0x0D89A0 */
        r->a[1] = recv;
        uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);
        wr32(m, recv + 0x48u, rd32(m, recv + 0x48u) + pts);
        goto die_explode;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)              /* 0x0D87FA */
        goto die_explode;

    {
        uint32_t tbl = ((rd8(m, obj + 0x21u) & 0x40u) != 0u) ? TBL_0D8A0C : TBL_0D8A1A;
        Obj_Animate(m, obj, tbl);                             /* 0x0D881A -> 0x0CBD52 */
    }
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));            /* 0x0D8824 -> 0x0CBE12 */
    Obj_SubScrollVel(m, obj);                                 /* 0x0D882A -> 0x0CBE00 */

    sub_0C66F4(m, rd16(m, obj + 0x14u), rd16(m, obj + 0x18u), MSK_0D8AB0); /* 0x0D883C */

    {
        uint16_t gx = (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, F3_A5 - 0x5DA8u) + 0x10u) & 0x01FFu;
        uint16_t gy = (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, F3_A5 - 0x5DA4u) + 0x10u) & 0x01FFu;
        uint16_t idx = (uint16_t)((gx >> 4) + ((gy >> 4) << 5));
        wr32(m, 0x00610000u + (uint32_t)idx * 4u, 0u);        /* 0x0D8870 */
    }

    Obj_0D0DC2(m, obj);                                       /* 0x0D8874 -> 0x0CBDA4 */
    r->a[1] = rd32(m, F3_A5 + 0x0906u);
    Obj_HitFlash(m, obj);                                     /* 0x0D887A -> 0x0CBE34 */
    draw_sprite_regs(m, obj, r);                              /* 0x0D8880 -> 0x001282 */

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if (x <= -0x30 || x >= 0x0150 || y <= -0x30 || y >= 0x00F8) {
            Obj_Free(m, obj);                                 /* 0x0D89F2 */
            return 0;
        }
    }

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x0D88AE */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {
            wr32(m, obj, 0x000D87F2u);                        /* 0x0D87EA */
            return 0;
        }
    }

    {
        uint16_t mx = (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, F3_A5 - 0x5DA8u)) & 0x0Fu;
        if ((int16_t)mx < 7 || (int16_t)mx > 9) {
            wr32(m, obj, 0x000D87F2u);
            return 0;
        }
        uint16_t my = (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, F3_A5 - 0x5DA4u)) & 0x0Fu;
        if ((int16_t)my < 7 || (int16_t)my > 9) {
            wr32(m, obj, 0x000D87F2u);
            return 0;
        }
    }

    wr16(m, obj + 0x3Au, 0u);                                 /* 0x0D88EE */
    wr16(m, obj + 0x3Eu, 0u);                                 /* 0x0D88F2 */
    if ((rd8(m, obj + 0x4Eu) & 0x08u) != 0u) {                /* 0x0D88F6 */
        if ((rd8(m, obj + 0x21u) & 0x40u) == 0u)
            wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x02u));
        else
            wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u));
    }
    wr32(m, obj, 0x000D8922u);                                /* 0x0D891A */
    return 0;

die_explode:
    Sound_PlayHooked(m, (uint16_t)rd8(m, obj + 0x65u));       /* 0x0D89B0 */
    {
        uint32_t arg = sub_13E4D0(m, obj, 2u);                /* 0x0D89C8 */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg); /* 0x0D89E0 */
    }
    wr32(m, obj, 0x000D89F2u);                                /* 0x0D89EA */
    return 0;
}
