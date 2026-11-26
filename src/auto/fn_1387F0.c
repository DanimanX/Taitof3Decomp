/* ROM 0x1387F0: multi-part boss component tick (A0 = object, +0x08 = parent,
 * +0x0C = {part_tick.l, dead_spr.l}). Frees self (0x0105EC) if parent->+0x59
 * or self->+0x59 != 0. Applies damage diff (+0x44 - +0x24) scaled by hitbox
 * width (+0x2E - +0x2C)/0x32 and shot type (+0x36->+0x22) when hit (+0x35 == 1),
 * dispatches part_tick (*(+0x0C), 15 parts 0x1388A0..0x139E5C), then if +0x24 > 0
 * runs 0x11B06E and latches +0x44 = +0x24 / re-arms 0x1387F0, else sets dead
 * sprite (+0x0C->+4), awards +0x4C score via 0x0CBDDE, adds 0x12C5F to A5+0x886,
 * and transitions to 0x11B12C (0x11B102). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/palette.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }
static uint32_t swap32(uint32_t v) { return (v << 16) | (v >> 16); }

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

/* ROM 0x11B09A with exact D0/D1/A1 register outputs */
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
            if ((int16_t)(h + x) > 0) {
                Obj_DrawSprite(m, obj);
                draw_sprite_post_regs(m, obj, d0_io, d1_io, a1_io);
            }
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t offy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - offy));
    *d0_io = setw(*d0_io, offy);
}

/* ROM 0x0CC2BC: D0 = (rem << 16) | quot, D1 = divisor */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
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
    *d0_out = ((uint32_t)rem << 16) | q;
    *d1_out = d1;
    return q;
}

/* ROM 0x11C2F8: queue palette upload if +0x1D != +0x4F, else upload src */
static void sub_11C2F8(const f3mem_t *m, uint32_t obj, uint32_t src, uint16_t dst,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint32_t d0 = src;
    if (rd8(m, obj + 0x4Fu) != rd8(m, obj + 0x1Du)) {
        d0 = rd8(m, obj + 0x1Du);
        d0 = setw(d0, (uint16_t)((uint16_t)d0 << 6));
        d0 = setw(d0, (uint16_t)((uint16_t)d0 + 0x4000u));
        d0 += 0x00440000u;
    }
    Pal_QueueUpload(m, d0, dst, 0x0020u);
    *d0_io = d0;
    *d1_io = setw(*d1_io, dst);
}

/* ROM 0x138772 -> 0x1385B2: draw compressed tilemap relative to scroll */
static void sub_138772(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint32_t d2,
                       uint16_t d3w, uint32_t a1, uint32_t *d0_io, uint32_t *d1_io,
                       uint32_t *a1_io)
{
    uint16_t idx4 = (uint16_t)(rd16(m, a1 + 6u) << 4);
    uint32_t a2 = rd32(m, 0x001385E2u + (uint32_t)(int32_t)(int16_t)idx4);
    int16_t d4w = (int16_t)rd16(m, a2);
    d4w = (int16_t)(d4w >> 7);
    int16_t d5w = d4w;
    int16_t q = (int16_t)((int32_t)d4w / 0x0C);
    if (d5w < 0)
        q = (int16_t)(q - 1);
    d4w = (int16_t)(q * 0x0C);
    d3w = (uint16_t)((uint16_t)(d3w + (uint16_t)d4w) & 3u);
    d3w <<= 3;
    d0w = (uint16_t)(d0w + d3w);

    uint16_t tbl_idx = rd16(m, a1);
    uint32_t dst = rd32(m, 0x00138622u + (uint32_t)(int32_t)(int16_t)tbl_idx * 4u);
    uint32_t addr = dst + (uint32_t)(int32_t)(int16_t)(d0w << 2)
                        + (uint32_t)((int32_t)(int16_t)d1w * 0x80);
    uint16_t flags = (uint16_t)(((d2 >> 31) & 1u) << 9);
    Gfx_DrawCompressedTilemap(m, d2 & 0x7FFFFFFFu, addr, flags);
    *d0_io = setw(*d0_io, d0w);
    *d1_io = setw(*d1_io, d1w);
    *a1_io = dst;
}

static uint8_t bf_get(const f3mem_t *m, uint32_t base, uint32_t off)
{
    return (uint8_t)(rd8(m, base + (off >> 3)) & (0x80u >> (off & 7u)));
}

static uint8_t bf_clr(const f3mem_t *m, uint32_t base, uint32_t off)
{
    uint32_t addr = base + (off >> 3);
    uint8_t mask = (uint8_t)(0x80u >> (off & 7u));
    uint8_t old = rd8(m, addr);
    wr8(m, addr, (uint8_t)(old & (uint8_t)~mask));
    return (uint8_t)(old & mask);
}

static void bf_set(const f3mem_t *m, uint32_t base, uint32_t off)
{
    uint32_t addr = base + (off >> 3);
    uint8_t mask = (uint8_t)(0x80u >> (off & 7u));
    wr8(m, addr, (uint8_t)(rd8(m, addr) | mask));
}

/* ROM 0x1388A0 */
static void part_1388A0(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    if (bf_get(m, F3_A5 + 0x30EEu, 0u) != 0u) {
        for (;;) {
            *a1_io = 0x00138952u;
            Obj_AnimFrameStep(m, obj, 0x00138952u);
            uint16_t off = rd16(m, obj + 0x3Au);
            *d0_io = setw(*d0_io, off);
            *d1_io = setw(*d1_io, rd16(m, obj + 0x3Eu));
            int16_t dur = (int16_t)rd16(m, 0x00138952u + (int32_t)(int16_t)off);
            if (dur > 0)
                goto draw;
            if (dur < 0)
                break;
            *d0_io = Obj_Spawn(m, obj, 0x0013A7C4u, 0u, 1u, 0u);
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
        }
    }
    bf_clr(m, F3_A5 + 0x30EEu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x10u, 0x0003B30Cu);

draw:
    if (rd32(m, obj + 0x10u) == 0x0003B514u) {
        uint32_t saved_10 = rd32(m, obj + 0x10u);
        wr32(m, obj + 0x10u, 0x0003B5E4u);
        wr16(m, obj + 0x54u, 0u);
        wr16(m, obj + 0x56u, 0x0020u);
        sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
        wr16(m, obj + 0x54u, 0u);
        wr16(m, obj + 0x56u, 0u);
        wr32(m, obj + 0x10u, saved_10);
    }
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        wr16(m, F3_A5 + 0x32D6u, 1u);
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 1)
        wr8(m, obj + 0x59u, 0xFFu);
}

/* ROM 0x13899E */
static void part_13899E(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io)
{
    static const uint32_t args[] = {
        0x5ADEC800u, 0x5AE5B000u, 0x5AF19800u, 0x5A008000u,
        0x5A0F6800u, 0x5A1B5000u, 0x5A223800u
    };
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) < 1) {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    } else {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
        if ((int32_t)rd32(m, obj + 0x24u) <= 0)
            wr16(m, F3_A5 + 0x32D6u, 2u);
        if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 2) {
            wr8(m, obj + 0x59u, 0xFFu);
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        }
        sub_11C2F8(m, obj, 0x00026240u, 0x1F00u, d0_io, d1_io);
        if (bf_clr(m, F3_A5 + 0x30EEu, 4u) != 0u)
            *d0_io = Obj_Spawn(m, obj, 0x0013A548u, 0xBu, 1u, 0u);
        if (bf_clr(m, F3_A5 + 0x30EEu, 5u) != 0u) {
            for (unsigned i = 0; i < 7u; i++)
                *d0_io = Obj_Spawn(m, obj, 0x000CDDA6u, 5u, 2u, args[i]);
        }
    }
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + rd16(m, obj + 0x50u)));
    uint16_t y = (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + rd16(m, obj + 0x52u));
    wr16(m, obj + 0x18u, y);
    *d0_io = setw(*d0_io, y);
}

/* ROM 0x138B10 */
static void part_138B10(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr32(m, obj + 0x10u, 0x00028740u);
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) < 2) {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    } else {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
        if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (int16_t)rd16(m, F3_A5 + 0x32D6u) >= 3) {
            if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 3)
                wr8(m, obj + 0x59u, 0xFFu);
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x18000005u);
            Sound_PlayHooked(m, 0x0016u);
            sub_138772(m, 0u, 1u, 0x000883BCu, 0u, 0x0013863Au, d0_io, d1_io, a1_io);
            wr16(m, F3_A5 + 0x32D6u, 3u);
            *d0_io = Obj_Spawn(m, obj, 0x00117C28u, 5u, 2u, 2u);
        }
        sub_11C2F8(m, obj, 0x00026240u, 0x1F00u, d0_io, d1_io);
        if (bf_clr(m, F3_A5 + 0x30EEu, 8u) != 0u)
            *d0_io = Obj_Spawn(m, obj, 0x0013BF9Eu, 5u, 2u, 0x10008000u);
        if (bf_get(m, F3_A5 + 0x30EEu, 7u) != 0u) {
            if (sub_0CC2BC(m, 8u, d0_io, d1_io) == 0u) {
                uint16_t rem = (uint16_t)((*d0_io >> 16) & 0x0Fu);
                *d0_io = (*d0_io & 0xFFFF0000u) | rem;
                if ((int16_t)rem > 7) {
                    (void)Obj_Spawn(m, obj, 0x000CCF72u, 9u, 2u, 0xF4F58000u);
                    (void)Obj_Spawn(m, obj, 0x000CCF72u, 9u, 2u, 0xF40B8000u);
                    (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0xF0F58000u);
                    *d0_io = Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0xF00B8000u);
                    Sound_PlayHooked(m, 0x003Bu);
                }
            }
        }
    }
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + rd16(m, obj + 0x50u)));
    uint16_t y = (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + rd16(m, obj + 0x52u));
    wr16(m, obj + 0x18u, y);
    *d0_io = setw(*d0_io, y);
}

/* ROM 0x138CCE / 0x138FC2: side wing/pod pair */
static void part_wing(const f3mem_t *m, uint32_t obj, int right,
                      uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    static const uint32_t b5_L[5] = { 0xEC12C000u, 0xFB12C000u, 0x0A12C000u, 0x1912C000u, 0x2912C000u };
    static const uint32_t b8_L[8] = { 0xCE404000u, 0xDD404000u, 0xEC404000u, 0xFB404000u, 0x0A404000u, 0x19404000u, 0x29404000u, 0x38404000u };
    static const uint32_t b5_R[5] = { 0xEC2D4000u, 0xFB2D4000u, 0x0A2D4000u, 0x192D4000u, 0x292D4000u };
    static const uint32_t b8_R[8] = { 0xCEFFC000u, 0xDDFFC000u, 0xECFFC000u, 0xFBFFC000u, 0x0AFFC000u, 0x19FFC000u, 0x29FFC000u, 0x38FFC000u };

    wr8(m, obj + 0x4Fu, 0xDEu);
    wr8(m, obj + 0x1Fu, 0x0Du);
    wr8(m, obj + 0x1Cu, right ? 0x02u : 0x00u);
    if (bf_get(m, F3_A5 + 0x30EEu, 9u) != 0u) {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    } else {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
    wr32(m, obj + 0x10u, 0x0003B80Cu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, F3_A5 + (right ? 0x32D2u : 0x32CEu)) + 0xFE79u));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, F3_A5 + (right ? 0x32D4u : 0x32D0u)) + (right ? 0u : 0xFFC0u)));
    wr16(m, obj + 0x54u, 0xFFB0u);
    wr16(m, obj + 0x56u, 0u);
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);

    {
        int32_t p = (int32_t)rd32(m, obj + 0x3Au);
        if (bf_get(m, F3_A5 + 0x30EEu, 10u) == 0u) {
            p -= 0x4000;
            if (p < 0x8000) p = 0x8000;
        } else {
            p += 0x4000;
            if (p > 0x78000) p = 0x78000;
        }
        wr32(m, obj + 0x3Au, (uint32_t)p);
    }

    {
        uint16_t idx = rd16(m, obj + 0x3Au);
        *a1_io = 0x001392DEu;
        wr32(m, obj + 0x10u, rd32(m, 0x001392DEu + (uint32_t)(int32_t)(int16_t)idx * 4u));
        wr16(m, obj + 0x54u, 0xFFE5u);
        wr16(m, obj + 0x56u, right ? 0x0020u : 0u);
        sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
    }

    if ((right ? bf_clr(m, F3_A5 + 0x30EEu, 11u) : bf_get(m, F3_A5 + 0x30EEu, 11u)) != 0u) {
        const uint32_t *t = right ? b5_R : b5_L;
        for (unsigned i = 0; i < 5u; i++)
            *d0_io = Obj_Spawn(m, obj, 0x0013C2A8u, 5u, 2u, t[i]);
        if (right)
            Sound_PlayHooked(m, 0x0011u);
    }
    if ((right ? bf_clr(m, F3_A5 + 0x30EEu, 12u) : bf_get(m, F3_A5 + 0x30EEu, 12u)) != 0u) {
        const uint32_t *t = right ? b8_R : b8_L;
        for (unsigned i = 0; i < 8u; i++)
            *d0_io = Obj_Spawn(m, obj, 0x0013C2A8u, 5u, 2u, t[i]);
        if (right)
            Sound_PlayHooked(m, 0x0011u);
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr16(m, F3_A5 + 0x32D6u, 4u);
        *d0_io = Obj_Spawn(m, obj, 0x00117C28u, 5u, 2u, right ? 1u : 3u);
    }
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 4) {
        wr8(m, obj + 0x59u, 0xFFu);
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, right ? 0x14170005u : 0x14280005u);
        *d0_io = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, right ? 0x54170005u : 0x54280005u);
        Sound_PlayHooked(m, 0x0016u);
    }
}

/* ROM 0x1392FE */
static void part_1392FE(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr8(m, obj + 0x4Fu, 0xDEu);
    uint16_t idx = rd8(m, obj + 0x3Au);
    wr32(m, obj + 0x10u, rd32(m, 0x0013941Cu + (uint32_t)idx * 4u));
    *d0_io = idx;

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) < 3) {
        sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
    } else {
        Obj_DrawSprite(m, obj);
        draw_sprite_post_regs(m, obj, d0_io, d1_io, a1_io);
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
        if ((int16_t)rd16(m, obj + 0x3Eu) > 0x0100) {
            wr16(m, obj + 0x3Eu, 0x0100u);
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00010000u);
            wr16(m, obj + 0x2Cu, 4u);
            wr16(m, obj + 0x2Eu, 0x0034u);
            if ((int16_t)rd16(m, obj + 0x22u) >= 0x0096) {
                (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20200005u);
                Sound_PlayHooked(m, 0x0016u);
                uint32_t arg = 0x20200000u;
                for (int i = 0; i < 32; i++) {
                    *d0_io = Obj_Spawn(m, obj, 0x000CCDB6u, 9u, 2u, arg);
                    arg = (arg & 0xFFFF0000u) | (uint16_t)(arg + 0x0800u);
                }
                wr8(m, obj + 0x59u, 0xFFu);
                bf_set(m, F3_A5 + 0x30EEu, 9u);
            } else {
                sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
            }
        } else {
            sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
        }
    }

    uint16_t d0w = rd16(m, obj + 0x3Eu);
    wr16(m, obj + 0x3Au, (uint16_t)((rd16(m, obj + 0x3Au) + d0w) & 0x03FFu));
    *d0_io = setw(*d0_io, d0w);
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        *d0_io = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20200005u);
        Sound_PlayHooked(m, 0x0016u);
        bf_set(m, F3_A5 + 0x30EEu, 9u);
    }
}

/* ROM 0x13942C */
static void part_13942C(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr8(m, obj + 0x4Fu, 0xDEu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr16(m, obj + 0x54u, 0u);
    wr16(m, obj + 0x56u, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x10u, 0x0003BBE8u);
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);

    Obj_Animate(m, obj, 0x0013953Eu);
    *d0_io = setw(*d0_io, rd16(m, obj + 0x3Au));
    *d1_io = setw(*d1_io, rd16(m, obj + 0x3Eu));
    wr16(m, obj + 0x1Eu, rd16(m, obj + 0x10u));
    uint16_t off = rd16(m, obj + 0x12u);
    wr16(m, obj + 0x54u, (uint16_t)(0x0080u + off));
    wr32(m, obj + 0x10u, 0x0003BC1Cu);
    wr16(m, obj + 0x56u, (uint16_t)(0xFFE6u + off));
    {
        uint32_t saved_d0 = setw(*d0_io, off);
        *d0_io = saved_d0;
        sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
        *d0_io = saved_d0;
    }
    wr16(m, obj + 0x56u, (uint16_t)(0x002Au + off));
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);

    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 5)
        wr8(m, obj + 0x59u, 0xFFu);

    if (bf_get(m, F3_A5 + 0x30EEu, 14u) != 0u) {
        wr32(m, obj + 0x2Cu, 0u);
    } else {
        wr16(m, obj + 0x2Cu, 0x0028u);
        wr16(m, obj + 0x30u, 0xFFE8u);
        wr16(m, obj + 0x2Eu, 0x00AFu);
        wr16(m, obj + 0x32u, 0x0057u);
    }

    if (bf_get(m, F3_A5 + 0x30EEu, 15u) != 0u) {
        if (sub_0CC2BC(m, 0x0032u, d0_io, d1_io) == 0u)
            *d0_io = Obj_Spawn(m, obj, 0x0013B7B4u, 0u, 1u, 0x48200000u);
    }
    if (bf_clr(m, F3_A5 + 0x30EEu, 13u) != 0u)
        *d0_io = Obj_Spawn(m, obj, 0x0013A38Cu, 0xBu, 1u, 0u);
}

/* ROM 0x139564 */
static void part_139564(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr8(m, obj + 0x4Fu, 0xDEu);
    wr8(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x10u, 0x0003BC8Cu);
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);

    uint16_t save_x = rd16(m, obj + 0x14u);
    uint16_t save_y = rd16(m, obj + 0x18u);
    wr16(m, obj + 0x14u, (uint16_t)(save_x + 0x0010u));
    wr16(m, obj + 0x18u, (uint16_t)(save_y + 0x0009u));

    uint16_t d0w = 1u;
    uint16_t d1w = rd16(m, obj + 0x3Au);
    do {
        uint16_t d2w = (uint16_t)((d1w >> 10) & 0x3Fu);
        if ((int16_t)d0w > (int16_t)d2w) {
            wr32(m, obj + 0x10u, 0x0003BC80u);
        } else if ((int16_t)d0w < (int16_t)d2w) {
            wr32(m, obj + 0x10u, 0x0003BC44u);
        } else {
            uint16_t idx = (uint16_t)((d1w >> 8) & 3u);
            wr32(m, obj + 0x10u, rd32(m, 0x0013970Cu + (uint32_t)idx * 4u));
        }
        wr8(m, obj + 0x1Cu, 0u);
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x003Eu));
        wr8(m, obj + 0x1Cu, 2u);
        Obj_DrawSprite(m, obj);
        {
            uint32_t td0 = d0w, td1 = d1w;
            draw_sprite_post_regs(m, obj, &td0, &td1, a1_io);
        }
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x003Eu));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
        d0w = (uint16_t)(d0w + 1u);
        d1w = (uint16_t)(d1w + 0x0080u);
    } while ((int16_t)d0w <= 8);

    *d0_io = d0w;
    *d1_io = setw(*d1_io, d1w);
    wr16(m, obj + 0x18u, save_y);
    wr16(m, obj + 0x14u, save_x);

    if (bf_get(m, F3_A5 + 0x30EEu, 16u) != 0u && (int8_t)rd8(m, obj + 0x3Au) < 0x24)
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x002Au));
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 5)
        wr8(m, obj + 0x59u, 0xFFu);
    if (bf_get(m, F3_A5 + 0x30EEu, 14u) != 0u) {
        wr16(m, obj + 0x2Cu, 8u);
        wr16(m, obj + 0x2Eu, 0x008Fu);
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr16(m, F3_A5 + 0x32D6u, 5u);
        *d0_io = Obj_Spawn(m, obj, 0x00117C28u, 5u, 2u, 1u);
    }
    if (bf_get(m, F3_A5 + 0x30EEu, 17u) != 0u) {
        if (sub_0CC2BC(m, 0x000Au, d0_io, d1_io) == 0u) {
            uint16_t s = (uint16_t)((rd16(m, obj + 0x3Eu) << 3) + 0x0014u);
            *d1_io = setw(*d1_io, s);
            uint32_t hi = ((uint32_t)(s & 0xFFu) << 24);
            (void)Obj_Spawn(m, obj, 0x0013BCAE, 5u, 2u, hi | 0x0016C000u);
            *d0_io = Obj_Spawn(m, obj, 0x0013BCAE, 5u, 2u, hi | 0x005A4000u);
            Sound_PlayHooked(m, 0x0011u);
            wr16(m, obj + 0x3Eu, (uint16_t)((rd16(m, obj + 0x3Eu) + 1u) & 7u));
        }
    } else {
        wr16(m, obj + 0x3Eu, 0u);
    }
}

/* ROM 0x13971C / 0x139878 / 0x1399D4 / 0x139B30 / 0x139C8C */
static void part_armor(const f3mem_t *m, uint32_t obj, uint32_t spr, int16_t thr,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    static const uint32_t args[10] = {
        0x0020DC00u, 0x0038EC00u, 0x0050FC00u, 0x00680C00u, 0x007F1C00u,
        0x1020E400u, 0x1038F400u, 0x10500400u, 0x10681400u, 0x107F2400u
    };
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr16(m, obj + 0x32u, 0x0087u);
    wr32(m, obj + 0x10u, spr);
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= thr) {
        wr8(m, obj + 0x59u, 0xFFu);
        for (unsigned i = 0; i < 10u; i++)
            *d0_io = Obj_Spawn(m, obj, 0x0013A608u, 5u, 2u, args[i]);
    }
}

/* ROM 0x139DE8 */
static void part_139DE8(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr32(m, obj + 0x10u, 0x0003BF30u);
    sub_11B09A_regs(m, obj, d0_io, d1_io, a1_io);
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 5) {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    } else {
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        *d0_io = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x30200005u);
        Sound_PlayHooked(m, 0x0016u);
        wr16(m, F3_A5 + 0x32D6u, 0x000Bu);
    }
}

/* ROM 0x139E5C */
static void part_139E5C(const f3mem_t *m, uint32_t obj,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x2Cu, 0xFEF8u);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x4Fu));
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    sub_11C2F8(m, obj, 0x00025540u, 0x1200u, d0_io, d1_io);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + rd16(m, obj + 0x52u)));
    a1 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x59u, rd8(m, a1 + 0x59u));
    *a1_io = a1;
    *d0_io = setw(*d0_io, rd16(m, obj + 0x18u));

    if (bf_get(m, F3_A5 + 0x30EEu, 19u) != 0u) {
        if (sub_0CC2BC(m, 6u, d0_io, d1_io) == 0u) {
            uint16_t rem = (uint16_t)((*d0_io >> 16) & 7u);
            *d0_io = (*d0_io & 0xFFFF0000u) | rem;
            if ((int16_t)rem < 5) {
                (void)Obj_Spawn(m, obj, 0x000CD956u, 9u, 2u, 0x4CFE0000u);
                (void)Obj_Spawn(m, obj, 0x000CD956u, 9u, 2u, 0x4C0A0000u);
                (void)Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x50FE0000u);
                *d0_io = Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, 0x500A0000u);
                Sound_PlayHooked(m, 0x002Au);
            }
        }
    }
    if (bf_get(m, F3_A5 + 0x30EEu, 20u) != 0u) {
        if (sub_0CC2BC(m, 0x0064u, d0_io, d1_io) == 0u) {
            (void)Obj_Spawn(m, obj, 0x000CDDB0u, 5u, 2u, 0x50FE0000u);
            *d0_io = Obj_Spawn(m, obj, 0x000CDDB0u, 5u, 2u, 0x500A0000u);
        }
    }
}

int Fn_1387F0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x1387F0 */

    if (rd8(m, a1 + 0x59u) != 0u) {                      /* 0x1387F4..0x1387F8 */
        Obj_Free(m, obj);                                /* 0x138894 (preserves D0,A0,A1) */
        r->a[1] = a1;
        return 0;
    }
    if (rd8(m, obj + 0x59u) != 0u) {                     /* 0x1387FC..0x138800 */
        Obj_Free(m, obj);
        r->a[1] = a1;
        return 0;
    }

    d0 = rd32(m, obj + 0x44u) - rd32(m, obj + 0x24u);    /* 0x138804..0x138808 */
    if (d0 != 0u) {                                      /* 0x13880C */
        wr32(m, obj + 0x24u, rd32(m, obj + 0x44u));      /* 0x138810 */
        if (rd8(m, obj + 0x35u) == 1u) {                 /* 0x138816..0x13881C */
            int16_t w = (int16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)); /* 0x138820..0x138824 */
            if (w >= 0) {                                /* 0x138828 */
                int16_t q = (int16_t)(((int32_t)w / 0x32) + 1);
                int16_t rem = (int16_t)((int32_t)w % 0x32);
                d1 = ((uint32_t)(uint16_t)rem << 16) | (uint16_t)q;
                a1 = rd32(m, obj + 0x36u);               /* 0x138834 */
                uint8_t k = rd8(m, a1 + 0x22u);
                if (k != 0u && k != 4u) {                /* 0x138838..0x138848 */
                    unsigned sh = d1 & 63u;
                    int32_t s = (int32_t)d0;
                    d0 = (sh >= 32u) ? (s < 0 ? 0xFFFFFFFFu : 0u) : (uint32_t)(s >> sh);
                    if (k != 1u && k != 5u)              /* 0x13884E..0x13885E */
                        d0 = (uint32_t)((int32_t)d0 >> 1);
                }
            } else {
                d1 = setw(d1, (uint16_t)w);
            }
        }
    }
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) - d0);     /* 0x138864 */

    {
        uint32_t desc = rd32(m, obj + 0x0Cu);            /* 0x138868 */
        uint32_t fn = rd32(m, desc);                     /* 0x13886C */
        a1 = fn;
        switch (fn) {
        case 0x001388A0u: part_1388A0(m, obj, &d0, &d1, &a1); break;
        case 0x0013899Eu: part_13899E(m, obj, &d0, &d1); break;
        case 0x00138B10u: part_138B10(m, obj, &d0, &d1, &a1); break;
        case 0x00138CCEu: part_wing(m, obj, 0, &d0, &d1, &a1); break;
        case 0x00138FC2u: part_wing(m, obj, 1, &d0, &d1, &a1); break;
        case 0x001392FEu: part_1392FE(m, obj, &d0, &d1, &a1); break;
        case 0x0013942Cu: part_13942C(m, obj, &d0, &d1, &a1); break;
        case 0x00139564u: part_139564(m, obj, &d0, &d1, &a1); break;
        case 0x0013971Cu: part_armor(m, obj, 0x0003BD8Cu,  6, &d0, &d1, &a1); break;
        case 0x00139878u: part_armor(m, obj, 0x0003BDE0u,  7, &d0, &d1, &a1); break;
        case 0x001399D4u: part_armor(m, obj, 0x0003BE34u,  8, &d0, &d1, &a1); break;
        case 0x00139B30u: part_armor(m, obj, 0x0003BE88u,  9, &d0, &d1, &a1); break;
        case 0x00139C8Cu: part_armor(m, obj, 0x0003BEDCu, 10, &d0, &d1, &a1); break;
        case 0x00139DE8u: part_139DE8(m, obj, &d0, &d1, &a1); break;
        case 0x00139E5Cu: part_139E5C(m, obj, &d0, &d1, &a1); break;
        default:          return -1;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) > 0) {             /* 0x138870..0x138874 */
        if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
            a1 = rd32(m, F3_A5 + 0x0906u) + 4u;
        Obj_11B06E(m, obj);                              /* 0x138878 */
        wr32(m, obj + 0x44u, rd32(m, obj + 0x24u));      /* 0x13887E -> 0x1387E2 */
        wr32(m, obj + 0x00u, 0x001387F0u);               /* 0x1387E8 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    a1 = rd32(m, obj + 0x0Cu);                           /* 0x138882 */
    wr32(m, obj + 0x10u, rd32(m, a1 + 4u));              /* 0x138886 */
    /* 0x13888C jmp 0x0011B102 */
    a1 = Obj_0CBDDE(m, obj);                             /* 0x11B102 */
    d0 = rd16(m, obj + 0x4Cu);                           /* 0x11B108..0x11B10A */
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);       /* 0x11B10E */
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu); /* 0x11B112 */
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));            /* 0x11B11A */
    wr8(m, obj + 0x35u, 0u);                             /* 0x11B120 */
    wr32(m, obj + 0x00u, 0x0011B12Cu);                   /* 0x11B124 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
