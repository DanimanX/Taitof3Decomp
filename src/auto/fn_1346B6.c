/* ROM 0x1346B6: segmented boss part tick (A0 = object, +0x08 = parent).
 * Mirrors parent+0x59 into +0x59 and frees self if non-zero. Clears +0x50..+0x56,
 * dispatches the part's +0x5A callback (27 handlers in 0x135312..0x136076),
 * unpacks sprite/offset/hitbox from +0x10, applies +0x62 bit-0 X mirroring,
 * clips and draws via Obj_DrawSpriteSub (0x001410), appends to collision lists
 * (0x0CBDA4) when A5+0x30F3 == 0 and +0x30.l != 0, and re-arms 0x1346B6. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

static inline uint32_t setw(uint32_t v, uint16_t w)
{
    return (v & 0xFFFF0000u) | (uint32_t)w;
}

static inline uint16_t min16(uint16_t a, uint16_t b)
{
    return (int16_t)a < (int16_t)b ? a : b;
}

static inline uint16_t max16(uint16_t a, uint16_t b)
{
    return (int16_t)a > (int16_t)b ? a : b;
}

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0w) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d1w * (uint32_t)d0w;
    uint16_t div = (uint16_t)(prod >> 8);
    if (div == 0u)
        div = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % div);
}

/* ROM 0x11AE62: fold obj+0x24 into shared boss HP A5+0x30DA, crediting receiver on <= 0. */
static void sub_11AE62(const f3mem_t *m, uint32_t obj, uint32_t d0, unsigned d1)
{
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)
        return;
    int32_t d2 = (int32_t)rd32(m, obj + 0x24u);
    if (d2 == 0)
        return;
    wr32(m, obj + 0x24u, 0u);
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u) {
        d2 >>= (d1 & 63u);
        if (k != 1u && k != 5u)
            d2 >>= 1;
    }
    wr32(m, F3_A5 + 0x30DAu, rd32(m, F3_A5 + 0x30DAu) + (uint32_t)d2);
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0)
        return;
    a1 = Obj_0CBDDE(m, obj);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x11C096: propagate hit flag +0x35 to parent and queue hit spark in 0x40B0F6. */
static void sub_11C096(const f3mem_t *m, uint32_t obj)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 0u)
        goto done;

    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | hit));
    }
    if ((rd8(m, obj + 0x34u) & 0x02u) != 0u)
        goto done;

    {
        uint16_t now = rd16(m, F3_A5 + 0x0878u);
        uint16_t d0w = (uint16_t)(now - rd16(m, F3_A5 + 0x311Cu));
        wr16(m, F3_A5 + 0x311Cu, now);
        if ((int16_t)d0w < 2) {
            uint32_t prev = rd32(m, F3_A5 + 0x3116u);
            wr32(m, F3_A5 + 0x3116u, obj);
            if (prev == obj) {
                uint16_t c = (uint16_t)(rd16(m, F3_A5 + 0x311Au) + 1u);
                wr16(m, F3_A5 + 0x311Au, c);
                if ((int16_t)c <= 3)
                    goto done;
            }
        }
    }

    wr16(m, F3_A5 + 0x311Au, 0u);
    {
        uint32_t a2 = 0x0040B0F6u;
        int found = 0;
        for (int i = 0; i < 8; i++) {
            uint8_t b = rd8(m, a2);
            wr8(m, a2, (uint8_t)(b | 0x80u));
            if ((b & 0x80u) == 0u) {
                found = 1;
                break;
            }
            a2 += 4u;
        }
        if (!found)
            goto done;

        uint32_t a1 = rd32(m, obj + 0x36u);
        uint16_t d0w, d1w;
        if (rd8(m, a1 + 0x22u) == 6u || rd8(m, obj + 0x35u) == 2u) {
            uint32_t d1 = Rng_NextMem(m) & 0xFFFFu;
            uint16_t dv = (uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u));
            if (dv != 0u)
                d1 = ((d1 / dv) << 16) | (d1 % dv);
            d1w = (uint16_t)((uint16_t)d1 + rd16(m, obj + 0x30u) + rd16(m, obj + 0x18u));

            uint32_t d0 = Rng_NextMem(m) & 0xFFFFu;
            uint16_t dv2 = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
            if (dv2 != 0u)
                d0 = ((d0 / dv2) << 16) | (d0 % dv2);
            d0w = (uint16_t)((uint16_t)d0 + rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x14u));
        } else {
            uint16_t ox = rd16(m, obj + 0x14u), ax = rd16(m, a1 + 0x14u);
            uint16_t oy = rd16(m, obj + 0x18u), ay = rd16(m, a1 + 0x18u);
            d0w = max16((uint16_t)(rd16(m, obj + 0x2Cu) + ox), (uint16_t)(rd16(m, a1 + 0x2Cu) + ax));
            d1w = min16((uint16_t)(rd16(m, obj + 0x2Eu) + ox), (uint16_t)(rd16(m, a1 + 0x2Eu) + ax));
            d0w = (uint16_t)((int16_t)(uint16_t)(d0w + d1w) >> 1);
            d1w = max16((uint16_t)(rd16(m, obj + 0x30u) + oy), (uint16_t)(rd16(m, a1 + 0x30u) + ay));
            uint16_t d2w = min16((uint16_t)(rd16(m, obj + 0x32u) + oy), (uint16_t)(rd16(m, a1 + 0x32u) + ay));
            d1w = (uint16_t)((int16_t)(uint16_t)(d1w + d2w) >> 1);
        }
        d0w = (uint16_t)(d0w - rd16(m, F3_A5 + 0x30D2u));
        d1w = (uint16_t)(d1w - rd16(m, F3_A5 + 0x30D6u));
        uint32_t v = rd32(m, a2);
        v = (v & ~(0x3FFu << 21)) | (((uint32_t)d0w & 0x3FFu) << 21);
        v = (v & ~(0x1FFu << 12)) | (((uint32_t)d1w & 0x1FFu) << 12);
        wr32(m, a2, v);
    }

done:
    wr8(m, obj + 0x35u, 0u);
}

/* ROM 0x0CC290: compute packed (x, y, ang) spawn arg. */
static uint32_t sub_0CC290(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                           uint16_t d2w, uint16_t d3w)
{
    uint32_t a1 = 0x00027E40u;
    int16_t cos_v = (int16_t)rd16(m, a1 + 0x80u + (uint32_t)d2w * 2u);
    int16_t sin_v = (int16_t)rd16(m, a1 + (uint32_t)d2w * 2u);
    uint16_t ox = (uint16_t)((uint32_t)(((int32_t)(int16_t)d3w * (int32_t)cos_v) << 2) >> 16);
    uint16_t oy = (uint16_t)((uint32_t)(((int32_t)(int16_t)d3w * (int32_t)sin_v) << 2) >> 16);
    d0w = (uint16_t)(d0w + ox);
    d1w = (uint16_t)(d1w + oy);
    return (((uint32_t)d0w & 0xFFu) << 24)
         | (((uint32_t)d1w & 0xFFu) << 16)
         | (((uint32_t)d2w & 0xFFu) << 8);
}

/* ROM 0x135A4A: shared pose/offset update for clamp-table parts. */
static void sub_135A4A(const f3mem_t *m, uint32_t obj, uint32_t a1, uint8_t d1b,
                       uint32_t *d0_io)
{
    uint8_t d0b = rd8(m, obj + 0x20u);
    uint32_t a2 = rd32(m, obj + 0x08u);
    if (d0b > rd8(m, a2 + 0x20u))
        d0b = rd8(m, a2 + 0x20u);
    d0b = (uint8_t)(d0b - d1b);
    uint16_t d0w = (uint16_t)(d0b >> 3);
    wr8(m, obj + 0x63u, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, obj + 0x5Eu) + (uint32_t)d0w * 0x10u);
    uint16_t p63 = rd8(m, a2 + 0x63u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a1 + (uint32_t)p63 * 4u) + rd16(m, a2 + 0x50u)));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, a1 + (uint32_t)p63 * 4u + 2u) + rd16(m, a2 + 0x52u)));
    *d0_io = (uint32_t)rd16(m, a2 + 0x52u);
}

/* Clamp word at A5+off to [lo, hi], write +0x20, +0x63, +0x10. */
static uint32_t clamp_part_60_A0(const f3mem_t *m, uint32_t obj, uint32_t a5_off)
{
    int32_t d0 = (int32_t)rd16(m, F3_A5 + a5_off);
    if (d0 < 0x6000) d0 = 0x6000;
    if (d0 > 0xA000) d0 = 0xA000;
    wr16(m, F3_A5 + a5_off, (uint16_t)d0);
    wr16(m, obj + 0x20u, (uint16_t)d0);
    uint16_t idx = (uint16_t)((uint16_t)(rd8(m, obj + 0x20u) - 0x60u) >> 3);
    wr8(m, obj + 0x63u, (uint8_t)idx);
    uint32_t ptr = rd32(m, obj + 0x5Eu) + (uint32_t)idx * 0x10u;
    wr32(m, obj + 0x10u, ptr);
    return ptr;
}

static void clamp_part_00_80(const f3mem_t *m, uint32_t obj, uint32_t a5_off,
                             uint32_t *d0_io, uint32_t *d1_io)
{
    int32_t d0 = (int32_t)rd16(m, F3_A5 + a5_off);
    if (d0 < 0) d0 = 0;
    if (d0 > 0x8000) d0 = 0x8000;
    wr16(m, F3_A5 + a5_off, (uint16_t)d0);
    wr16(m, obj + 0x20u, (uint16_t)d0);
    *d1_io = 0u;
    sub_135A4A(m, obj, 0x0013573Eu, 0u, d0_io);
}

static uint32_t clamp_part_40_C0(const f3mem_t *m, uint32_t obj, uint32_t a5_off)
{
    int32_t d0 = (int32_t)rd16(m, F3_A5 + a5_off);
    if (d0 < 0x4000) d0 = 0x4000;
    if (d0 > 0xC000) d0 = 0xC000;
    wr16(m, F3_A5 + a5_off, (uint16_t)d0);
    wr16(m, obj + 0x20u, (uint16_t)d0);
    uint16_t idx = (uint16_t)((uint16_t)(rd8(m, obj + 0x20u) - 0x40u) >> 3);
    wr8(m, obj + 0x63u, (uint8_t)idx);
    uint32_t ptr = rd32(m, obj + 0x5Eu) + (uint32_t)idx * 0x10u;
    wr32(m, obj + 0x10u, ptr);
    return ptr;
}

static void clamp_part_40_C0_sub(const f3mem_t *m, uint32_t obj, uint32_t a5_off,
                                 uint32_t *d0_io, uint32_t *d1_io)
{
    int32_t d0 = (int32_t)rd16(m, F3_A5 + a5_off);
    if (d0 < 0x4000) d0 = 0x4000;
    if (d0 > 0xC000) d0 = 0xC000;
    wr16(m, F3_A5 + a5_off, (uint16_t)d0);
    wr16(m, obj + 0x20u, (uint16_t)d0);
    *d1_io = 0x40u;
    sub_135A4A(m, obj, 0x00135906u, 0x40u, d0_io);
}

static void clamp_part_00_60(const f3mem_t *m, uint32_t obj, uint32_t a5_off,
                             uint32_t *d0_io, uint32_t *d1_io)
{
    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x32CCu));
    int16_t d0w = (int16_t)rd16(m, F3_A5 + a5_off);
    if (d0w < 0) d0w = 0;
    if (d0w > 0x6000) d0w = 0x6000;
    wr16(m, F3_A5 + a5_off, (uint16_t)d0w);
    wr16(m, obj + 0x20u, (uint16_t)d0w);
    *d1_io = 0u;
    sub_135A4A(m, obj, 0x00135A06u, 0u, d0_io);
}

/* Dispatch the +0x5A callback; returns 0 or -1 if unknown. */
static int run_part_cb(const f3mem_t *m, uint32_t obj, uint32_t cb,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    switch (cb) {
    case 0x00135312u: {
        *d0_io = 0x000186A0u;
        *d1_io = 2u;
        sub_11AE62(m, obj, 0x000186A0u, 2u);
        sub_11C096(m, obj);
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        wr32(m, obj + 0x10u, rd32(m, obj + 0x5Eu));
        return 0;
    }
    case 0x00135394u: *d0_io = clamp_part_60_A0(m, obj, 0x32C8u); return 0;
    case 0x001353E0u: *d0_io = clamp_part_60_A0(m, obj, 0x32C4u); return 0;
    case 0x0013545Eu: clamp_part_00_80(m, obj, 0x32CAu, d0_io, d1_io); return 0;
    case 0x00135494u: clamp_part_00_80(m, obj, 0x32C6u, d0_io, d1_io); return 0;
    case 0x001354D2u:
    case 0x00135506u: {
        Obj_Animate(m, obj, 0x0013553Au);
        uint32_t d0 = rd32(m, obj + 0x10u) + rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, d0);
        int16_t d0w = (int16_t)(0x0074 - (int16_t)rd16(m, F3_A5 + 0x30D6u));
        if (d0w > 0x0040) d0w = 0x0040;
        d0w >>= 3;
        wr16(m, obj + 0x56u, (uint16_t)d0w);
        *d0_io = setw(d0, (uint16_t)d0w);
        return 0;
    }
    case 0x0013555Cu:
    case 0x00135590u: {
        Obj_Animate(m, obj, 0x001355C4u);
        uint32_t d0 = rd32(m, obj + 0x10u) + rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, d0);
        int16_t d0w = (int16_t)(0x0074 - (int16_t)rd16(m, F3_A5 + 0x30D6u));
        if (d0w > 0x0040) d0w = 0x0040;
        d0w >>= 2;
        wr16(m, obj + 0x56u, (uint16_t)d0w);
        *d0_io = setw(d0, (uint16_t)d0w);
        return 0;
    }
    case 0x0013562Au: *d0_io = clamp_part_60_A0(m, obj, 0x32C0u); return 0;
    case 0x00135676u: *d0_io = clamp_part_60_A0(m, obj, 0x32BCu); return 0;
    case 0x001356D2u: clamp_part_00_80(m, obj, 0x32C2u, d0_io, d1_io); return 0;
    case 0x00135708u: clamp_part_00_80(m, obj, 0x32BEu, d0_io, d1_io); return 0;
    case 0x001357AEu: *d0_io = clamp_part_40_C0(m, obj, 0x32B6u); return 0;
    case 0x001357FAu: *d0_io = clamp_part_40_C0(m, obj, 0x32B0u); return 0;
    case 0x00135892u: clamp_part_40_C0_sub(m, obj, 0x32B8u, d0_io, d1_io); return 0;
    case 0x001358CCu: clamp_part_40_C0_sub(m, obj, 0x32B2u, d0_io, d1_io); return 0;
    case 0x00135996u: clamp_part_00_60(m, obj, 0x32BAu, d0_io, d1_io); return 0;
    case 0x001359CEu: clamp_part_00_60(m, obj, 0x32B4u, d0_io, d1_io); return 0;
    case 0x00135AA0u: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) {
            Obj_AnimFrameStep(m, obj, 0x00135B08u);
            uint16_t off3a = rd16(m, obj + 0x3Au);
            *d0_io = setw(*d0_io, off3a);
            int16_t v = (int16_t)rd16(m, 0x00135B08u + (uint32_t)(int32_t)(int16_t)off3a);
            if (v > 0) {
                uint32_t d1 = rd32(m, obj + 0x5Eu);
                wr32(m, obj + 0x10u, rd32(m, obj + 0x10u) + d1);
                *d1_io = d1;
                return 0;
            }
            if (v == 0) {
                wr32(m, obj + 0x10u, 0x00000030u);
                if ((rd8(m, F3_A5 + 0x30EEu) & 0x40u) != 0u)
                    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
                uint32_t d1 = rd32(m, obj + 0x5Eu);
                wr32(m, obj + 0x10u, rd32(m, obj + 0x10u) + d1);
                *d1_io = d1;
                return 0;
            }
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0xC0u));
        }
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        uint32_t d1 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, d1);
        *d1_io = d1;
        return 0;
    }
    case 0x00135B54u: {
        if ((int32_t)rd32(m, obj + 0x24u) > 0) {
            Obj_11C32C(m, obj);
            if (rd8(m, obj + 0x06u) == 0u) {
                if (rd8(m, obj + 0x35u) != 0u) {
                    wr8(m, obj + 0x06u, 0xFFu);
                    wr16(m, obj + 0x3Eu, 0u);
                    wr16(m, obj + 0x3Au, 0u);
                } else {
                    Obj_Animate(m, obj, 0x00135C04u);
                    uint32_t d0 = rd32(m, obj + 0x5Eu);
                    wr32(m, obj + 0x10u, rd32(m, obj + 0x10u) + d0);
                    *d0_io = d0;
                    return 0;
                }
            }
            Obj_AnimFrameStep(m, obj, 0x00135C36u);
            if ((int16_t)rd16(m, 0x00135C36u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au)) <= 0) {
                wr16(m, obj + 0x3Eu, 0u);
                wr16(m, obj + 0x3Au, 0u);
                wr8(m, obj + 0x06u, 0u);
                Obj_Animate(m, obj, 0x00135C04u);
            }
        } else {
            if (rd32(m, obj + 0x36u) != 0u) {
                uint32_t a1 = Obj_0CBDDE(m, obj);
                wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u);
                Obj_Spawn(m, obj, 0x0013E4EEu, 3u, 2u, 2u);
                wr32(m, obj + 0x36u, 0u);
                wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            }
            wr32(m, obj + 0x10u, 0x00000080u);
        }
        uint32_t d0 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, rd32(m, obj + 0x10u) + d0);
        *d0_io = d0;
        return 0;
    }
    case 0x00135C82u: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        for (;;) {
            Obj_AnimFrameStep(m, obj, 0x00135CC2u);
            if ((int16_t)rd16(m, 0x00135CC2u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au)) > 0)
                break;
            if ((rd8(m, F3_A5 + 0x30EEu) & 0x02u) == 0u) {
                wr32(m, obj + 0x10u, 0u);
                break;
            }
            wr16(m, obj + 0x3Au, 0u);
        }
        uint32_t d0 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, rd32(m, obj + 0x10u) + d0);
        *d0_io = d0;
        return 0;
    }
    case 0x00135CEAu: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x02u) == 0u) {
            if (rd16(m, obj + 0x3Eu) != 0u)
                wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
        } else if ((int16_t)rd16(m, obj + 0x3Eu) < 0x0080) {
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
        } else if (sub_0CC2BC(m, 8u) == 0u) {
            uint16_t d2w = (uint16_t)((rd16(m, F3_A5 + 0x0878u) << 1) & 0x00FFu);
            uint32_t arg1 = sub_0CC290(m, 0x10u, 8u, d2w, 6u);
            Obj_Spawn(m, obj, 0x00134EAEu, 9u, 2u, arg1);
            d2w = (uint16_t)(((uint8_t)(arg1 >> 8) + 0x55u) & 0xFFu);
            uint32_t arg2 = sub_0CC290(m, 0x10u, 8u, d2w, 6u);
            Obj_Spawn(m, obj, 0x00134EAEu, 9u, 2u, arg2);
            d2w = (uint16_t)(((uint8_t)(arg2 >> 8) + 0x55u) & 0xFFu);
            uint32_t arg3 = sub_0CC290(m, 0x10u, 8u, d2w, 6u);
            Obj_Spawn(m, obj, 0x00134EAEu, 9u, 2u, arg3);
            Sound_PlayHooked(m, 0x0029u);
        }
        uint16_t spd = rd16(m, obj + 0x3Eu);
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + spd));
        if ((int8_t)rd8(m, obj + 0x3Au) >= 3)
            wr8(m, obj + 0x3Au, 0u);
        uint32_t d0 = (uint32_t)rd8(m, obj + 0x3Au) * 0x10u + rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, d0);
        *d0_io = d0;
        return 0;
    }
    case 0x00135E24u: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        a1 = rd32(m, F3_A5 + 0x30E2u);
        int16_t dx = (int16_t)((int16_t)rd16(m, a1 + 0x14u) - (int16_t)rd16(m, F3_A5 + 0x30D2u) + 0x1C);
        int16_t dy = (int16_t)((int16_t)rd16(m, a1 + 0x18u) - (int16_t)rd16(m, F3_A5 + 0x30D6u) + 0x18);
        uint8_t ang = (uint8_t)Obj_AtanByte(m, dx, dy);
        if (ang > 0xC0u) ang = 0xC0u;
        if (ang < 0x80u) ang = 0x80u;
        uint16_t d0w = (uint16_t)(ang & 0xF0u);
        d0w = (uint16_t)((uint8_t)((uint8_t)d0w - rd8(m, obj + 0x20u)));
        d0w = (uint16_t)((int16_t)(d0w << 8) >> 2);
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0w));
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) != 0u && sub_0CC2BC(m, 0x000Au) == 0u) {
            uint16_t idx = (uint16_t)(((uint8_t)((rd16(m, obj + 0x20u) & 0xF000u) >> 8) - 0x80u) & 0xFFu) >> 4;
            uint16_t hi = rd16(m, 0x00135F0Cu + (uint32_t)idx * 2u);
            uint32_t arg = ((uint32_t)hi << 16) | (rd16(m, obj + 0x20u) & 0xF000u);
            Obj_Spawn(m, obj, 0x000CCF72u, 9u, 2u, arg);
            Sound_PlayHooked(m, 0x003Bu);
        }
        uint8_t b = (uint8_t)(0u - (uint8_t)((rd16(m, obj + 0x20u) & 0xF000u) >> 8));
        b = (uint8_t)(b - 0x40u);
        uint32_t off = (uint32_t)(b >> 4) * 0x10u;
        uint32_t d1 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, off + d1);
        *d0_io = off;
        *d1_io = d1;
        return 0;
    }
    case 0x00135F16u: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        a1 = rd32(m, F3_A5 + 0x30E2u);
        int16_t dx = (int16_t)((int16_t)rd16(m, a1 + 0x14u) - (int16_t)rd16(m, F3_A5 + 0x30D2u) - 0x1B);
        int16_t dy = (int16_t)((int16_t)rd16(m, a1 + 0x18u) - (int16_t)rd16(m, F3_A5 + 0x30D6u) + 0x18);
        int8_t ang = (int8_t)Obj_AtanByte(m, dx, dy);
        if (ang < -0x40) ang = -0x40;
        if (ang > 0) ang = 0;
        uint16_t d0w = (uint16_t)((uint8_t)ang & 0xF0u);
        d0w = (uint16_t)((uint8_t)((uint8_t)d0w - rd8(m, obj + 0x20u)));
        d0w = (uint16_t)((int16_t)(d0w << 8) >> 2);
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0w));
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x10u) != 0u && sub_0CC2BC(m, 0x000Au) == 0u) {
            uint16_t idx = (uint16_t)((uint8_t)(((rd16(m, obj + 0x20u) & 0xF000u) >> 8) + 0x40u)) >> 4;
            uint16_t hi = rd16(m, 0x00135FFCu + (uint32_t)idx * 2u);
            uint32_t arg = ((uint32_t)hi << 16) | (rd16(m, obj + 0x20u) & 0xF000u);
            Obj_Spawn(m, obj, 0x000CCF72u, 9u, 2u, arg);
            Sound_PlayHooked(m, 0x003Bu);
        }
        uint8_t b = (uint8_t)(((rd16(m, obj + 0x20u) & 0xF000u) >> 8) + 0x40u);
        uint32_t off = (uint32_t)(b >> 4) * 0x10u;
        uint32_t d1 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, off + d1);
        *d0_io = off;
        *d1_io = d1;
        return 0;
    }
    case 0x00136008u: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x08u) != 0u && sub_0CC2BC(m, 0x003Cu) == 0u) {
            Obj_Spawn(m, obj, 0x000CDDA6u, 9u, 2u, 0x0507A000u);
            Obj_Spawn(m, obj, 0x000CDDA6u, 9u, 2u, 0x0C03B000u);
        }
        uint32_t d1 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, d1);
        *d1_io = d1;
        return 0;
    }
    case 0x00136076u: {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
        if ((rd8(m, F3_A5 + 0x30EEu) & 0x04u) != 0u && sub_0CC2BC(m, 0x003Cu) == 0u) {
            Obj_Spawn(m, obj, 0x000CDDA6u, 9u, 2u, 0x1A07E000u);
            Obj_Spawn(m, obj, 0x000CDDA6u, 9u, 2u, 0x1303D000u);
        }
        uint32_t d1 = rd32(m, obj + 0x5Eu);
        wr32(m, obj + 0x10u, d1);
        *d1_io = d1;
        return 0;
    }
    default:
        return -1;
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

int Fn_1346B6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                       /* 0x1346B6 */

    uint8_t dead = rd8(m, a1 + 0x59u);                        /* 0x1346BA */
    wr8(m, obj + 0x59u, dead);
    if (dead != 0u) {                                         /* 0x1346C0 -> 0x134836 */
        Obj_Free(m, obj);
        r->a[1] = a1;
        return 0;
    }

    wr16(m, obj + 0x50u, 0u);                                 /* 0x1346C4 */
    wr16(m, obj + 0x52u, 0u);                                 /* 0x1346C8 */
    wr16(m, obj + 0x54u, 0u);                                 /* 0x1346CC */
    wr16(m, obj + 0x56u, 0u);                                 /* 0x1346D0 */

    uint32_t cb = rd32(m, obj + 0x5Au);                       /* 0x1346D4 */
    if (run_part_cb(m, obj, cb, &d0, &d1) < 0)                /* 0x1346D8 */
        return -1;

    a1 = rd32(m, obj + 0x10u);                                /* 0x1346DA */
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;              /* 0x1346DE */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, a1))); a1 += 2u; /* 0x1346E2..0x1346E4 */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, a1))); a1 += 2u; /* 0x1346E8..0x1346EA */
    wr16(m, obj + 0x2Cu, rd16(m, a1)); a1 += 2u;              /* 0x1346EE */
    wr16(m, obj + 0x30u, rd16(m, a1)); a1 += 2u;              /* 0x1346F2 */
    wr16(m, obj + 0x2Eu, rd16(m, a1)); a1 += 2u;              /* 0x1346F6 */
    wr16(m, obj + 0x32u, rd16(m, a1)); a1 += 2u;              /* 0x1346FA */

    uint16_t d0w = rd16(m, obj + 0x50u);                      /* 0x1346FE */
    uint16_t d1w = rd16(m, obj + 0x52u);                      /* 0x134702 */
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x62u) & 1u);            /* 0x134706..0x13470C */
    if ((rd8(m, obj + 0x1Cu) & 1u) != 0u) {                   /* 0x134712 */
        a1 = rd32(m, obj + 0x10u);                            /* 0x134716 */
        uint16_t d2w = (uint16_t)(rd16(m, a1 + 2u) << 4);     /* 0x13471A..0x13471E */
        d0w = (uint16_t)((uint16_t)(0u - d0w) - d2w);         /* 0x134720..0x134722 */
        d2w = (uint16_t)(d2w - 1u);                           /* 0x134724 */
        uint16_t d3w = d2w;                                   /* 0x134726 */
        d2w = (uint16_t)(d2w - rd16(m, obj + 0x2Eu));         /* 0x134728 */
        d3w = (uint16_t)(d3w - rd16(m, obj + 0x2Cu));         /* 0x13472C */
        wr16(m, obj + 0x2Cu, d2w);                            /* 0x134730 */
        wr16(m, obj + 0x2Eu, d3w);                            /* 0x134734 */
    }

    d0w = (uint16_t)(d0w + rd16(m, F3_A5 + 0x30D2u));         /* 0x134738 */
    wr16(m, obj + 0x14u, d0w);                                /* 0x13473C */
    d1w = (uint16_t)(d1w + rd16(m, F3_A5 + 0x30D6u));         /* 0x134740 */
    wr16(m, obj + 0x18u, d1w);                                /* 0x134744 */

    uint16_t saved_14 = rd16(m, obj + 0x14u);                 /* 0x134748 */
    uint16_t saved_18 = rd16(m, obj + 0x18u);                 /* 0x13474C */
    wr16(m, obj + 0x14u, (uint16_t)(saved_14 + rd16(m, obj + 0x54u))); /* 0x134750..0x134754 */
    wr16(m, obj + 0x18u, (uint16_t)(saved_18 + rd16(m, obj + 0x56u))); /* 0x134758..0x13475C */

    a1 = rd32(m, obj + 0x10u);                                /* 0x134760 */
    d1w = rd16(m, a1); a1 += 2u;                              /* 0x134764 */
    d0w = rd16(m, a1); a1 += 2u;                              /* 0x134766 */

    int draw = 1;
    if ((rd8(m, obj + 0x1Cu) & 2u) != 0u) {                   /* 0x134768 */
        int16_t d2w = (int16_t)((int16_t)(d1w << 4) + (int16_t)rd16(m, obj + 0x18u));
        if (d2w <= -0x10) {
            draw = 0;
        } else {
            d2w = (int16_t)(d2w - 0x00E8);
            if (d2w > 0) {
                uint16_t sh = (uint16_t)d2w >> 4;
                d1w = (uint16_t)(d1w - sh);
                if ((int16_t)d1w <= 0)
                    draw = 0;
            }
        }
    } else {
        if ((int16_t)rd16(m, obj + 0x18u) >= 0x00E8) {
            draw = 0;
        } else {
            int16_t d2w = (int16_t)(-0x10 - (int16_t)rd16(m, obj + 0x18u));
            if (d2w > 0) {
                uint16_t sh = (uint16_t)d2w >> 4;
                d1w = (uint16_t)(d1w - sh);
                if ((int16_t)d1w <= 0)
                    draw = 0;
                else
                    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (sh << 4)));
            }
        }
    }

    if (draw) {
        if ((rd8(m, obj + 0x1Cu) & 1u) != 0u) {               /* 0x1347BA */
            int16_t d2w = (int16_t)((int16_t)(d0w << 4) + (int16_t)rd16(m, obj + 0x14u));
            if (d2w <= -0x10) {
                draw = 0;
            } else {
                d2w = (int16_t)(d2w - 0x0140);
                if (d2w > 0) {
                    uint16_t sh = (uint16_t)d2w >> 4;
                    d0w = (uint16_t)(d0w - sh);
                    if ((int16_t)d0w <= 0)
                        draw = 0;
                }
            }
        } else {
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x0140) {
                draw = 0;
            } else {
                int16_t d2w = (int16_t)(-0x10 - (int16_t)rd16(m, obj + 0x14u));
                if (d2w > 0) {
                    uint16_t sh = (uint16_t)d2w >> 4;
                    d0w = (uint16_t)(d0w - sh);
                    if ((int16_t)d0w <= 0)
                        draw = 0;
                    else
                        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (sh << 4)));
                }
            }
        }
    }

    d0 = setw(d0, d0w);
    d1 = setw(d1, d1w);
    if (draw) {                                               /* 0x13480C */
        Obj_DrawSpriteSub(m, obj, d0w, d1w, 3u);
        d0 = setw(d0, 0xFFFFu);
        d1 = setw(d1, 0xFFFFu);
        a1 = rd32(m, F3_A5 - 0x0CA0u);
    }

    wr16(m, obj + 0x18u, saved_18);                           /* 0x134814 */
    wr16(m, obj + 0x14u, saved_14);                           /* 0x134818 */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x30u) != 0u) /* 0x13481C..0x134828 */
        a1 = sub_0CBDA4(m, obj);                              /* 0x13482C */

    wr32(m, obj, 0x001346B6u);                                /* 0x134832 -> 0x1346AE */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
