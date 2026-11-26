/* ROM 0x12296E: multi-part boss/ship main body tick (A0 = object).
 * Frees self (0x0105EC) if parent (+0x08)->+0x59 is nonzero. Draws main hull
 * sprite 0x03944C (scrolling out right when A5+0x326E >= 6, freeing at X >= 0x140;
 * otherwise adding to collision lists 0x11B06E and drawing via 0x11B09A).
 * Calls sub_122C92 (cockpit/bay door anim via 0x0CBD24 on A5+0x30EE bit 0/1)
 * and sub_122D3E (14-part rotating/zooming wing/thruster array). While
 * A5+0x326E < 6, checks A5+0x30EE bits 4/5/6/9/10 to spawn 0x0CC570,
 * 0x0CC756 + sound 0x29, triples of 0x11B5E6, 0x0CCA8E, and 0x0CD956 +
 * 0x0CDCA0 + sound 0x2A, then re-arms 0x12296E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

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

/* ROM 0x11B09A with D0/D1/A1 register tracking. */
static void sub_11B09A_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);

    if (y < 0x00E8 && x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        *a1_io = a1;
        *d0_io = setw(*d0_io, (uint16_t)h);
        if ((int16_t)(h + y) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            *a1_io = a1;
            *d0_io = setw(*d0_io, (uint16_t)w);
            if ((int16_t)(w + x) > 0)
                draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
        }
    } else {
        *d0_io = setw(*d0_io, (uint16_t)y);
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t sy = rd16(m, obj + 0x56u);
    *d0_io = setw(*d0_io, sy);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy));
}

/* ROM 0x0CC2BC: frame-rate-compensated modulo timer -> D0w = rem, D0hi = quot. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    d1 = (d1 >> 16) | (d1 << 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t d0 = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(d0 / (uint16_t)d1);
    uint16_t rem = (uint16_t)(d0 % (uint16_t)d1);
    *d0_io = ((uint32_t)quot << 16) | rem;
    *d1_io = d1;
}

/* ROM 0x122C92: draw animated bay/door sub-sprite around (+0x48, +0x30). */
static void sub_122C92(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0048u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0030u));

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) {
        uint32_t tbl = 0x00122CFEu;
        *a1_io = tbl;
        Obj_AnimFrameStep(m, obj, tbl);
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        *d0_io = setw(*d0_io, (uint16_t)off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)off);
        if (dur > 0)
            goto draw;
        if (dur == 0) {
            wr32(m, obj + 0x10u, 0x0003955Cu);
            if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) != 0u)
                wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
            goto draw;
        }
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0xC0u));
    }

    wr32(m, obj + 0x10u, 0x00039520u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);

draw:
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
}

/* ROM 0x122D3E: draw 14-part rotating/zooming wing array from 0x122DE2/E0C/E18. */
static void sub_122D3E(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0031u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0020u));

    uint32_t a1 = 0x00122DE2u;
    uint32_t a2 = 0x00122E0Cu;
    uint32_t a3 = 0x00122E18u;
    uint16_t base_y = rd16(m, obj + 0x18u);
    uint32_t d2 = rd16(m, obj + 0x44u);

    for (int16_t d0w = 0x0D; d0w >= 0; d0w--) {
        wr16(m, obj + 0x18u, base_y);
        uint16_t dx = rd8(m, a1++);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx));
        int16_t dy = (int16_t)(int8_t)rd8(m, a1++);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)dy));
        uint16_t zoom = rd8(m, a1++);
        wr8(m, obj + 0x1Eu, (uint8_t)zoom);

        d2 = (uint16_t)(d2 % 12u);
        uint32_t idx = rd8(m, a2 + d2);
        uint32_t a4 = a3 + idx * 4u;
        wr8(m, obj + 0x1Cu, rd8(m, a4++));
        int16_t adj_h = (int16_t)((int8_t)rd8(m, a4++) - 1);
        int16_t inv_z = (int16_t)(0x0100 - (int16_t)zoom);
        int16_t y_off = (int16_t)(((int32_t)adj_h * (int32_t)inv_z) >> 8) + 1;
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)y_off));

        int32_t spr_off = (int16_t)rd16(m, a4);
        wr32(m, obj + 0x10u, (uint32_t)(0x000397D4 + spr_off));

        Obj_DrawSprite(m, obj);
        d2 = (uint16_t)(d2 + 1u);
    }

    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 0x00004000u);
    *d0_io = setw(*d0_io, 0xFFFFu);
    *d1_io = setw(*d1_io, base_y);
    *a1_io = a1;
}

int Fn_12296E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x12296E */

    if (rd8(m, a1 + 0x59u) != 0u) {                     /* 0x122972 */
        Obj_Free(m, obj);                               /* 0x122C84 */
        r->a[1] = a1;
        return 0;
    }

    wr16(m, obj + 0x1Eu, 0u);                           /* 0x12297A */
    wr16(m, obj + 0x1Cu, 0u);                           /* 0x12297E */
    wr32(m, obj + 0x10u, 0x0003944Cu);                  /* 0x122982 */

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) >= 6) {         /* 0x12298A */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + 0x00014000u);
        if ((int16_t)rd16(m, obj + 0x14u) >= 0x0140) {  /* 0x12299C */
            Obj_Free(m, obj);
            r->a[1] = a1;
            return 0;
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);        /* 0x1229A6 */
    } else {
        wr8(m, obj + 0x35u, 0u);                        /* 0x1229B0 */
        Obj_11B06E(m, obj);                             /* 0x1229B4 */
        if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
            a1 = rd32(m, F3_A5 + 0x0906u);
        sub_11B09A_regs(m, obj, &d0, &d1, &a1);         /* 0x1229BA */
    }

    {
        uint32_t sx = rd32(m, obj + 0x14u);             /* 0x1229C0 */
        uint32_t sy = rd32(m, obj + 0x18u);             /* 0x1229C4 */
        sub_122C92(m, obj, &d0, &d1, &a1);              /* 0x1229C8 */
        wr32(m, obj + 0x18u, sy);
        wr32(m, obj + 0x14u, sx);

        sx = rd32(m, obj + 0x14u);                      /* 0x1229D4 */
        sy = rd32(m, obj + 0x18u);
        sub_122D3E(m, obj, &d0, &d1, &a1);              /* 0x1229DC */
        wr32(m, obj + 0x18u, sy);
        wr32(m, obj + 0x14u, sx);
    }

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) >= 6)           /* 0x1229E8 */
        goto rearm;

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x08u) != 0u) {      /* 0x1229F2: bit 4 */
        sub_0CC2BC(m, 0x001Eu, &d0, &d1);
        if ((uint16_t)d0 == 0u) {
            d0 = Rng_NextMem(m);
            d1 = 0x5C340000u | (uint16_t)d0;
            d0 = Obj_Spawn(m, obj, 0x000CC570u, 9u, 2u, d1);
        }
    }

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x04u) != 0u) {      /* 0x122A34: bit 5 */
        sub_0CC2BC(m, 0x0050u, &d0, &d1);
        if ((uint16_t)d0 == 0u) {
            d1 = 0x5E323000u;
            d0 = Obj_Spawn(m, obj, 0x000CC756u, 9u, 2u, d1);
            Sound_PlayHooked(m, 0x29u);
        }
    }

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x02u) != 0u) {      /* 0x122A82: bit 6 */
        sub_0CC2BC(m, 0x000Au, &d0, &d1);
        if ((uint16_t)d0 == 0u) {
            d0 = (d0 >> 16) | (d0 << 16);
            d0 = (d0 & 0xFFFF0000u) | ((uint16_t)d0 & 7u);
            int16_t sel = (int16_t)(uint16_t)d0;
            if (sel < 3) {
                static const uint32_t args[3][3] = {
                    { 0x5810B000u, 0x4D14A000u, 0x43159000u },
                    { 0x6912B000u, 0x5F16A000u, 0x541D9000u },
                    { 0x7C14B000u, 0x7218A000u, 0x681F9000u },
                };
                int row = (sel == 1) ? 1 : (sel > 1 ? 2 : 0);
                for (int i = 0; i < 3; i++) {
                    d1 = args[row][i];
                    d0 = Obj_Spawn(m, obj, 0x0011B5E6u, 9u, 2u, d1);
                }
            }
        }
    }

    {
        uint8_t b = rd8(m, F3_A5 + 0x30EFu);
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(b & ~0x20u));  /* 0x122BDE: bit 10 */
        if ((b & 0x20u) != 0u) {
            d1 = 0x5E325000u;
            d0 = Obj_Spawn(m, obj, 0x000CCA8Eu, 9u, 2u, d1);
        }
    }

    if ((rd8(m, F3_A5 + 0x30EFu) & 0x40u) != 0u) {      /* 0x122C08: bit 9 */
        sub_0CC2BC(m, 0x0008u, &d0, &d1);
        if ((uint16_t)d0 == 0u) {
            d0 = (d0 >> 16) | (d0 << 16);
            d0 = (d0 & 0xFFFF0000u) | ((uint16_t)d0 & 7u);
            if ((int16_t)(uint16_t)d0 <= 3) {
                (void)Obj_Spawn(m, obj, 0x000CD956u, 9u, 2u, 0x050E8000u);
                d0 = Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x050E8000u);
                Sound_PlayHooked(m, 0x2Au);
            }
        }
    }

rearm:
    wr32(m, obj, 0x0012296Eu);                          /* 0x122C80 -> 0x122966 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
