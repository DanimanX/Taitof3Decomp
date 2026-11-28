/* ROM 0x0E2D1E: UNVERIFIED (no usable capture - the only pair is
 * VBlank-interrupted and the test harness rejects it): mid-boss segment tick
 * (A0 = object).
 * Debug on-screen kill (A5+0x1D58 bit 1 with +0x14 in [-10, 0x11C]) tail-calls
 * 0x0CBB2C; with +0x24 <= 0 or the bomb-clear bit (A5-0x144B bit 5) it spawns
 * 0x0DBE98 (group 9, count 2, arg 0) and starts dying. Otherwise it integrates
 * (+0x3A,+0x3E) by (+0x50 - A5+0x4224, +0x54 - A5+0x4228), flags off-screen
 * (0x0CF85A), starts dying when +0x07 bit 3 is set, copies the position into
 * (+0x14,+0x18), dies when the terrain point at (+0x14+8, +0x18+8) is solid
 * (0x0C6632), pushes the hazard slot (0x0D0DCC) and draws (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t set_word(uint32_t reg, uint16_t w)
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

int Fn_0E2D1E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {           /* 0x0E2D1E btst.b #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                     /* 0x0E2D26 blt / 0x0E2D2E bgt */
            Obj_OrbitDebug(m, obj);                          /* 0x0E2D36 jmp 0x0CBB2C */
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||                /* 0x0E2D3C ble */
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {           /* 0x0E2D44 btst.b #5 */
        d0 = Obj_Spawn(m, obj, 0x000DBE98u, 9u, 2u, 0u);     /* 0x0E2DAC..0x0E2DC4 */
        Obj_StartDying(m, obj);                              /* 0x0E2DC8 bra 0x0CEE74 */
        r->d[0] = d0;
        return 0;
    }

    d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);    /* 0x0E2D4E..0x0E2D52 */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);         /* 0x0E2D56 */
    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);    /* 0x0E2D5A..0x0E2D5E */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);         /* 0x0E2D62 */

    Obj_FlagOffscreen(m, obj);                               /* 0x0E2D66 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {               /* 0x0E2D6C btst.b #3 */
        Obj_StartDying(m, obj);                              /* 0x0E2D72 -> 0x0CEE74 */
        r->d[0] = d0;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));              /* 0x0E2D78 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));              /* 0x0E2D7E */
    d0 = set_word(d0, (uint16_t)(rd16(m, obj + 0x14u) + 8u)); /* 0x0E2D84..0x0E2D88 */
    d1 = set_word(d1, (uint16_t)(rd16(m, obj + 0x18u) + 8u)); /* 0x0E2D8A..0x0E2D8E */
    if (terrain_test_regs(m, (int16_t)(uint16_t)d0, (int16_t)(uint16_t)d1,
                          &d0, &d1) != 0u) {                  /* 0x0E2D90 -> 0x0C6632 */
        Obj_StartDying(m, obj);                              /* 0x0E2D98 -> 0x0CEE74 */
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;
    }

    Coll_AddHazard(m, obj);                                  /* 0x0E2D9E -> 0x0D0DCC */
    r->d[0] = d0;
    r->d[1] = d1;
    draw_sprite_regs(m, r);                                  /* 0x0E2DA4 -> 0x001282 */
    return 0;                                                /* 0x0E2DAA rts */
}
