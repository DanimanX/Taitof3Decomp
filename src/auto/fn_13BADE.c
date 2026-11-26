/* ROM 0x13BADE: spinning/aiming boss/enemy projectile tick (A0 = object).
 * While +0x22 == 0, animates via 0x13BC4E; on completion sets +0x34 bit 1,
 * +0x22 = 1, and clears +0x3A/+0x3E. When +0x22 != 0, spins through 16-frame
 * table 0x13BC6E while ramping +0x3E by +4 up to 0x100; past 0x100 increments
 * speed +0x42 and, if +0x22 <= 1, sets +0x22 = 2 and aims +0x20 at A5+0x1D28
 * via 0x0CC3C6. Moves along +0x20, clears +0x35, draws sprite (0x001282),
 * appends to collision lists (0x0CBDA4), frees if off-screen, else installs
 * 0x13BBE0. */
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

int Fn_13BADE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)         /* 0x13BADE */
        goto explode_free;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x13BAE8 */
        uint32_t a1 = Obj_0CBDDE(m, obj);                /* 0x13BBE4 */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto explode_free;
    }

    if (rd8(m, obj + 0x22u) == 0u) {                     /* 0x13BAF0 */
        Obj_AnimFrameStep(m, obj, 0x0013BC4Eu);          /* 0x13BAFC */
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, 0x0013BC4Eu + (uint32_t)(int32_t)off) > 0)
            goto bomb_check;                             /* 0x13BB06 */
        wr8(m, obj + 0x34u, rd8(m, obj + 0x34u) | 0x02u);
        wr8(m, obj + 0x22u, 1u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
    }

    /* 0x13BB1E..0x13BB68: spin & accelerate */
    {
        uint32_t idx = rd8(m, obj + 0x3Au) & 0x0Fu;      /* bfextu (0x3a,A0){4:4} */
        wr32(m, obj + 0x10u, rd32(m, 0x0013BC6Eu + idx * 4u));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x3Eu)));
        uint16_t e = (uint16_t)(rd16(m, obj + 0x3Eu) + 4u);
        wr16(m, obj + 0x3Eu, e);
        if ((int16_t)e > 0x0100) {
            wr16(m, obj + 0x3Eu, 0x0100u);
            wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
            if ((int8_t)rd8(m, obj + 0x22u) <= 1) {      /* 0x13BB4E */
                wr8(m, obj + 0x22u, 2u);
                uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
                wr16(m, obj + 0x20u, Obj_0CC3C6(m, obj, a1));
            }
        }
    }

bomb_check:
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u
        && (int16_t)rd16(m, obj + 0x14u) >= -0x000A
        && (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        Obj_ExplodeTick(m, obj);
        r->d[0] = (uint32_t)rd16(m, obj + 0x4Cu);
        r->a[1] = rd32(m, F3_A5 - 0x0CA0u);
        return 0;
    }

    /* 0x13BB90..0x13BBAA */
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    wr8(m, obj + 0x35u, 0u);
    Obj_11C32C(m, obj);
    draw_sprite_regs(m, r);
    r->a[1] = sub_0CBDA4(m, obj);

    /* 0x13BBB0..0x13BBD4: bounds check */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if (x <= -0x0040 || x >= 0x0180 || y <= -0x0040 || y >= 0x0128) {
            Obj_Free(m, obj);
            return 0;
        }
    }

    wr32(m, obj, 0x0013BBE0u);                           /* 0x13BBD8 */
    return 0;

explode_free:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20180002u);
    Sound_PlayHooked(m, 0x0035u);
    Obj_Free(m, obj);
    return 0;
}
