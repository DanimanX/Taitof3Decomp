#include "obj_shared.h"
#include "obj_shared_ext.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* One verified copy of each shared primitive (see obj_shared.h). The bodies are
 * lifted verbatim from src/enemy/obj_CCFD4.c, obj_108936.c and obj_104EFA.c,
 * all of which pass their MAME captures; only the name and linkage changed. */

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

uint16_t Obj_Atan2(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

uint16_t Obj_Distance(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)d0_in * (int32_t)(int16_t)d0_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)d1_in * (int32_t)(int16_t)d1_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    d0 = (d0 << 16) | (d0 >> 16);
    return (uint16_t)d0;
}

void Obj_ExplodeTick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x32u, 0x00u);
    wr8(m, obj + 0x34u, 0xFFu);

    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    int16_t off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)off));

    d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u);
    prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)off));

    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    d0 = (uint16_t)(Obj_Distance(m, dx, dy) << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0 < (int16_t)thresh;
    d0 = (uint16_t)(d0 - thresh);
    if (is_lt) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    d0 = (uint16_t)(d0 << 6);
    wr16(m, obj + 0x2Cu, d0);

    dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    d0 = (uint16_t)(Obj_Atan2(m, dx, dy) << 8);
    wr16(m, obj + 0x2Eu, d0);
    wr16(m, obj + 0x30u, 0x0100u);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0200u));

    uint8_t sh = rd8(m, obj + 0x32u);
    if (sh != 0u) {
        uint32_t tbl = rd32(m, obj + 0x0Cu);
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x2Eu) + 0x4800u);
        unsigned cnt = (unsigned)sh & 63u;
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
        wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)((int32_t)(int16_t)d1 * 4)));
    }

    uint16_t rad = rd16(m, obj + 0x2Cu);
    rad = (uint16_t)(rad - (uint16_t)(rad >> 5));
    wr16(m, obj + 0x2Cu, rad);

    uint32_t d1_l = 0x0100u * (uint32_t)rad;
    int32_t d1_s = ASR32(d1_l, 8u);
    d1_s = ASR32(d1_s, 6u);
    uint16_t d1w = (uint16_t)((uint16_t)(uint32_t)d1_s + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t d2w = d1w;

    uint16_t ang = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    wr32(m, obj + 0x14u, (uint32_t)((int32_t)(int16_t)d1w * (int32_t)cos_v) + 0x00A00000u);

    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    wr32(m, obj + 0x18u, (uint32_t)((int32_t)(int16_t)d2w * (int32_t)sin_v) + 0x00740000u);

    int32_t z = (int32_t)(int16_t)(-0x0100) * (int32_t)(int16_t)rd16(m, obj + 0x30u);
    z = ASR32(z, 8u);
    uint16_t zw = (uint16_t)((uint16_t)(uint32_t)z + 0x0100u);
    wr8(m, obj + 0x1Fu, (uint8_t)zw);
    wr8(m, obj + 0x1Eu, (uint8_t)zw);

    a1 = rd32(m, obj + 0x10u);
    d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)off));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)off));

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CBC78u);
}

void Obj_BlinkColour(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

void Obj_RelayTimer(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

uint16_t Obj_Next4Way(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

void Obj_StartScript(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x44u, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
}

/* ---- 0x0CEC4C block init --------------------------------------------- */

/* ROM 0x0CEC4C */
uint32_t Obj_SeedFromParams(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x34u, 0x0200u);
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u);
    wr32(m, obj + 0x0Cu, a2);
    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
    return a2;
}

/* ROM 0x0CEE2C */
void Obj_ClassColour(const f3mem_t *m, uint32_t obj)
{
    uint8_t c = rd8(m, 0x000CEE40u + (uint32_t)rd8(m, obj + 0x4Eu));
    wr8(m, obj + 0x1Du, c);
    wr8(m, obj + 0x4Fu, c);
}

/* ROM 0x0D0678 */
void Obj_StartBehaviour(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x1794A4: D0w = value in [-(d0-1)..d0-1] from the LCG at A5+0x4B3C,
 * stepping the generator once (x' = 5x - 1, halves swapped back). */
uint16_t Obj_RandRange(const f3mem_t *m, uint16_t d0)
{
    uint16_t r = d0;
    if (d0 != 0u) {
        int off = 0;
        while (off < 32 && !((uint32_t)d0 & (0x80000000u >> off)))
            off++;
        int16_t d1w = (int16_t)(off - 16);
        uint16_t d2w = rd16(m, 0x00179530u + (uint32_t)((int32_t)d1w * 2));
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & d2w);
        if ((int16_t)r < (int16_t)v)
            v = (uint16_t)(v - r);
        r = v;
        if (rd8(m, F3_A5 + 0x4B3Cu) & 1u)
            r = (uint16_t)(0u - r);
    }
    uint32_t s = rd32(m, F3_A5 + 0x4B3Cu) * 5u - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (s << 16) | (s >> 16));
    return r;
}

/* ROM 0x0D0E52: unlike 0x006422 this one reads its quadrant offsets from
 * 0xD0EB4, where the +y half (entries 2 and 4) needs no axis swap; its 0xD0EBC
 * table is indexed by D1*0x40 + D0 after the BFFFO scale, and a result of
 * 0x168 (the seam between the last two half-turns) folds to 0. */
uint16_t Obj_Atan256(const f3mem_t *m, int16_t x, int16_t y)
{
    uint16_t d0 = (uint16_t)y;   /* D0: second axis */
    uint16_t d1 = (uint16_t)x;   /* D1: first axis */
    uint16_t d2 = 0u;
    if ((int16_t)d0 < 0) { d2 = (uint16_t)(d2 + 2u); d0 = (uint16_t)(0u - d0); }
    if ((int16_t)d1 < 0) { d2 = (uint16_t)(d2 + 4u); d1 = (uint16_t)(0u - d1); }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) { d3 = (uint16_t)(0x10u + i); break; }
    }
    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }
    if (d2 != 2u && d2 != 4u) {
        uint16_t t = d0; d0 = d1; d1 = t;
    }
    d1 = (uint16_t)(d1 << 6);
    d1 = (uint16_t)(d1 + d0);
    uint16_t r = rd8(m, 0x000D0EBCu + (uint32_t)d1);
    r = (uint16_t)(r + rd16(m, 0x000D0EB4u + (uint32_t)d2 * 2u));
    if (r == 0x0168u)
        r = 0u;
    return r;
}

/* ROM 0x006422, kept as its own entry because 0x0CBB2C calls it with the
 * (D0 = x, D1 = y) register order the ROM uses, which is not the order of
 * Obj_Atan2's (dy, dx) parameters. */
uint16_t Obj_AtanByte(const f3mem_t *m, int16_t x, int16_t y)
{
    uint16_t d0 = (uint16_t)x;
    uint16_t d1 = (uint16_t)y;
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) { d2 |= 0x0002u; d0 = (uint16_t)(0u - d0); }
    if ((int16_t)d1 < 0) { d2 |= 0x0004u; d1 = (uint16_t)(0u - d1); }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) { d3 = (uint16_t)(0x10u + i); break; }
    }
    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }
    if (d2 != 0u && d2 != 6u) {
        uint16_t t = d0; d0 = d1; d1 = t;
    }
    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    d0 = (uint16_t)(d0 + rd16(m, 0x006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x00649A */
uint16_t Obj_DistScaled(const f3mem_t *m, int16_t x, int16_t y, uint16_t *shift)
{
    uint32_t d0 = (uint32_t)((int32_t)x * (int32_t)x);
    uint32_t d1 = (uint32_t)((int32_t)y * (int32_t)y);
    d0 += d1;
    d0 += d0;
    uint16_t d1w = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        d1w = (uint16_t)(d1w + 4u);
    }
    uint16_t v = rd16(m, 0x028340u + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 2u);
    uint32_t r = (uint32_t)v << (d1w & 63u);
    if (shift)
        *shift = d1w;
    return (uint16_t)((r << 16) | (r >> 16));
}

/* ROM 0x179434 */
void Obj_BossMapScreenPos(const f3mem_t *m, uint32_t obj)
{
    uint32_t d1 = rd32(m, obj + 0x58u) & 0x00FFFF00u;
    if (d1 == 0u)
        return;
    d1 += 0x00010000u;
    {
        int32_t d0 = (int32_t)rd32(m, obj + 0x18u) - 0x00820000;
        d0 /= (int32_t)d1;
        wr16(m, obj + 0x18u, (uint16_t)(d0 + 0x82));
    }
    {
        int32_t d0 = (int32_t)rd32(m, obj + 0x14u) - 0x00900000;
        d0 /= (int32_t)d1;
        wr16(m, obj + 0x14u, (uint16_t)(d0 + 0x90));
    }
}

/* ROM 0x17AF9A */
uint8_t Obj_AimAtPlayer17AF9A(const f3mem_t *m, uint32_t p, uint16_t x, uint16_t y)
{
    if ((int32_t)p <= 0)
        return (uint8_t)p;
    uint16_t dx = (uint16_t)(rd16(m, p + 0x14u) + 0x18u - x);
    uint16_t dy = (uint16_t)(rd16(m, p + 0x18u) + 0x10u - y);
    return (uint8_t)Obj_AtanByte(m, (int16_t)dx, (int16_t)dy);
}

/* ROM 0x001660: queue a palette upload {count.w, src.l, dst.w} (max 0x20) */
static void queue_pal_upload(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t a2 = F3_A5 - 0x6750u + (uint32_t)(int32_t)(int16_t)(uint16_t)(n << 3);
    wr16(m, a2, count);
    wr32(m, a2 + 2u, src);
    wr16(m, a2 + 6u, dst);
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x17B3AE */
void Boss_UploadPalettes(const f3mem_t *m)
{
    uint32_t a2 = 0x0017B4ACu;
    if (rd16(m, F3_A5 + 0x32DCu) == 0x0001u)
        a2 = 0x0017B6ECu;
    for (uint32_t i = 0; i < 8u; i++)
        queue_pal_upload(m, a2 + i * 0x40u, (uint16_t)(0x6800u + i * 0x80u), 0x20u);
}

/* ROM 0x1847E8 */
void Boss_ResetGlobals(const f3mem_t *m, uint32_t obj)
{
    static const uint16_t clrw[] = { 0x4B02, 0x4B22, 0x4B24, 0x4B26, 0x4B36, 0x4B38, 0x4B3A };
    for (unsigned i = 0; i < sizeof clrw / sizeof clrw[0]; i++)
        wr16(m, F3_A5 + clrw[i], 0u);
    wr32(m, F3_A5 + 0x4B1Cu, 0u);
    wr16(m, F3_A5 + 0x4B20u, 0u);
    wr16(m, F3_A5 + 0x4B34u, 0u);
    wr32(m, F3_A5 + 0x4B3Cu, rd32(m, F3_A5 - 0x1456u));
    wr32(m, F3_A5 + 0x4B0Cu, 0x00010000u);
    wr32(m, F3_A5 + 0x4B10u, 0x00010000u);
    wr32(m, F3_A5 + 0x4B14u, 0x00010000u);
    wr8(m, F3_A5 + 0x4B28u, 0x02u);
    wr8(m, F3_A5 + 0x4B29u, 0x00u);
    wr16(m, F3_A5 + 0x4B2Au, 0x0064u);
    wr16(m, F3_A5 + 0x4B2Cu, 0x0064u);
    wr16(m, F3_A5 + 0x4B2Eu, 0x0064u);
    for (uint32_t a = 0x4B44u; a <= 0x4B50u; a += 2u)
        wr16(m, F3_A5 + a, 0u);
    wr8(m, obj + 0x34u, 0x10u);
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x23u, 0u);
    wr16(m, obj + 0x16u, 0u);
    wr16(m, obj + 0x1Au, 0u);
}

/* ROM 0x184888 */
void Obj_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x62u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x66u, rd32(m, a1));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* ROM 0x1848D6 */
void Obj_1848D6(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x24u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x28u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x2Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x30u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x2Eu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x32u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x16u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x1Au, rd16(m, a1));
    wr16(m, obj + 0x34u, 0u);
}

/* ROM 0x13F238 */
int16_t Sound_PreCmdHook(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    return cmd;
}

/* ROM 0x11B2F6 */
void Obj_11B2F6(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x113652 */
void Obj_113652(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)(d0 >> 24));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)((d0 >> 16) & 0xFFu));
    wr16(m, obj + 0x20u, (uint16_t)(((d0 >> 8) & 0xFFu) << 8));
    uint16_t d1 = (uint16_t)(d0 & 0xFFu);
    wr16(m, obj + 0x22u, d1);
    if (d1 != 0u)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
}

/* ROM 0x17A7FA */
void Obj_17A7FA(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0u);
    wr16(m, obj + 0x18u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr32(m, obj + 0x50u, 0u);
    wr32(m, obj + 0x54u, 0u);
    wr32(m, obj + 0x58u, 0u);
    wr16(m, obj + 0x16u, 0u);
    wr16(m, obj + 0x1Au, 0u);
    wr16(m, obj + 0x42u, 0u);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    wr16(m, obj + 0x60u, 0u);
    wr32(m, obj + 0x4Cu, 0xFFFFFFFFu);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x22u, 0u);
    wr8(m, obj + 0x23u, 0u);
    wr8(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x35u, 0u);
}

/* ROM 0x18495E */
void Obj_18495E(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((rd8(m, F3_A5 + 0x087Fu) >> 4) & 0x0Fu);
    uint16_t d0 = (uint16_t)rd8(m, 0x001849E6u + idx);
    uint16_t d1 = (uint16_t)((rd16(m, obj + 0x24u) / 0x0064u) * d0);
    wr16(m, obj + 0x24u, (uint16_t)(rd16(m, obj + 0x24u) + d1));
}

/* ROM 0x11AE2C */
void Obj_11AE2C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 + 0x30F4u, 0u);
    if (m->gfx) {
        uint32_t base = 0x00627600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        for (int i = 0; i < 0xE8; i++)
            wr16(m, base + (uint32_t)i * 2u, 0xDDDDu);
    }
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x08u));
    Obj_Free(m, obj);
}

/* ROM 0x0CEDE4 */
void Obj_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0x0200u);
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0CFA7C */
void Obj_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)rd16(m, 0x000CFA96u + (uint32_t)d0 * 2u);
    a1 = (uint32_t)((int32_t)a1 + (int32_t)off);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0CBDDE */
uint32_t Obj_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u)
        return a1;
    if (rd8(m, obj + 0x35u) == 1u)
        return rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x13E4D0 */
uint32_t Obj_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint8_t d1_hi = (uint8_t)(rd16(m, a1) << 3);
    uint8_t d1_lo = (uint8_t)(rd16(m, a1 + 2u) << 3);
    return (d0 & 0x0000FFFFu) | ((uint32_t)d1_lo << 24) | ((uint32_t)d1_hi << 16);
}

/* ROM 0x0CC3C6 */
uint16_t Obj_0CC3C6(const f3mem_t *m, uint32_t obj, uint32_t target)
{
    int16_t dx = (int16_t)(((int16_t)rd16(m, target + 0x2Cu) + (int16_t)rd16(m, target + 0x2Eu)
                            - (int16_t)rd16(m, obj + 0x2Cu) - (int16_t)rd16(m, obj + 0x2Eu)) >> 1);
    dx = (int16_t)(dx + (int16_t)rd16(m, target + 0x14u) - (int16_t)rd16(m, obj + 0x14u));
    int16_t dy = (int16_t)(((int16_t)rd16(m, target + 0x30u) + (int16_t)rd16(m, target + 0x32u)
                            - (int16_t)rd16(m, obj + 0x30u) - (int16_t)rd16(m, obj + 0x32u)) >> 1);
    dy = (int16_t)(dy + (int16_t)rd16(m, target + 0x18u) - (int16_t)rd16(m, obj + 0x18u));
    return (uint16_t)Obj_Atan2(m, dx, dy);
}

/* ROM 0x11C32C */
void Obj_11C32C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 6u) {
            wr8(m, obj + 0x35u, 0u);
            wr8(m, obj + 0x1Du, 8u);
        } else {
            wr8(m, obj + 0x1Du, 6u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

/* ROM 0x11B06E */
void Obj_11B06E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
        wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
        wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
        wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    }
    Obj_11C32C(m, obj);
}

/* ROM 0x11B09A */
void Obj_11B09A(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    wr16(m, obj + 0x18u, (uint16_t)y);
    if (y < 0xE8 && x < 0x140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        int16_t w = (int16_t)((int16_t)rd16(m, a1 + 2u) << 4);
        if ((int16_t)(h + y) > 0 && (int16_t)(w + x) > 0)
            Obj_DrawSprite(m, obj);
    }
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, obj + 0x56u)));
}

/* ROM 0x0D0DC2 */
void Obj_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
}

/* ROM 0x0CEF0E */
void Obj_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

static int sub_0D1EBC_shared(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                             uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    *out_d0 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
    return 0;
}

static int sub_0D25E4_shared(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                             uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;
    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }
    if (d4w > 50u)
        return -1;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d3 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    uint32_t d4 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
    *out_d0 = rd32(m, 0x000D263Cu + off) * (uint32_t)d4w + d3;
    *out_d1 = rd32(m, 0x000D27A4u + off) * (uint32_t)d4w + d4;
    return 0;
}

/* ROM 0x0CF898 */
int Obj_0CF898(const f3mem_t *m, uint32_t obj, uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    const uint16_t d6w = 0x0168u;
    uint32_t v;

    v = rd32(m, a1 + 0x08u) + rd32(m, a1 + 0x0Cu);
    wr32(m, a1 + 0x08u, v);
    if ((int32_t)v < 0)
        wr16(m, a1 + 0x08u, (uint16_t)(rd16(m, a1 + 0x08u) + d6w));
    else if (rd16(m, a1 + 0x08u) >= d6w)
        wr16(m, a1 + 0x08u, (uint16_t)(rd16(m, a1 + 0x08u) - d6w));

    v = rd32(m, a1 + 0x10u) + rd32(m, a1 + 0x14u);
    wr32(m, a1 + 0x10u, v);
    if ((int32_t)v < 0)
        wr32(m, a1 + 0x10u, 0u);
    else if (rd16(m, a1 + 0x10u) > 0x0032u)
        wr16(m, a1 + 0x10u, 0x0032u);

    v = rd32(m, a1 + 0x18u) + rd32(m, a1 + 0x1Cu);
    wr32(m, a1 + 0x18u, v);
    if ((int32_t)v < 0)
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + d6w));
    else if (rd16(m, a1 + 0x18u) >= d6w)
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) - d6w));

    v = rd32(m, a1 + 0x20u) + rd32(m, a1 + 0x24u);
    wr32(m, a1 + 0x20u, v);
    if ((int32_t)v < 0)
        wr16(m, a1 + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) + d6w));
    else if (rd16(m, a1 + 0x20u) >= d6w)
        wr16(m, a1 + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) - d6w));

    uint16_t d0w = rd16(m, a1 + 0x08u);
    uint16_t d1w = rd16(m, a1 + 0x10u);
    uint16_t d2w = d0w;
    uint16_t d3w = d1w;
    uint32_t d0, d1;
    if (sub_0D1EBC_shared(m, d0w, d1w, &d0, &d1) < 0)
        return -1;

    uint32_t d4 = d0;
    uint32_t d5 = d1;

    d2w = (uint16_t)(d2w - rd16(m, a1 + 0x18u));
    if ((int16_t)d2w < 0)
        d2w = (uint16_t)(d2w + d6w);
    if (sub_0D1EBC_shared(m, d2w, d3w, &d0, &d1) < 0)
        return -1;

    d0w = rd16(m, a1 + 0x20u);
    d1w = (uint16_t)(d1 >> 16);
    d3w = 0x005Au;
    if (d2w < 0x00B4u) {
        d1w = (uint16_t)(0u - d1w);
        d3w = (uint16_t)(d3w + 0x00B4u);
    }
    d2w = d1w;
    if (sub_0D1EBC_shared(m, d0w, d1w, &d0, &d1) < 0)
        return -1;

    d0w = (uint16_t)(rd16(m, a1 + 0x18u) + d3w);
    if (d0w >= d6w)
        d0w = (uint16_t)(d0w - d6w);
    d1w = (uint16_t)((uint16_t)(d1 >> 16) + d2w);
    if (sub_0D25E4_shared(m, d0w, d1w, &d0, &d1) < 0)
        return -1;

    *out_d0 = d0 + d4;
    *out_d1 = d1 + d5;
    return 0;
}

/* ROM 0x0D0DE4 */
void Obj_0D0DE4(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) != 0u)
        return;
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0D0E04 */
void Obj_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u) {
        uint32_t a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (k == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0CEE48 */
void Obj_0CEE48(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        uint8_t d0 = rd8(m, obj + 0x4Eu);
        if (d0 != 0u)
            (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, (uint32_t)d0);
    }
    Obj_StartDying(m, obj);
}

/* ROM 0x0D825E */
void Obj_0D825E(const f3mem_t *m, uint32_t obj)
{
    int16_t d0 = (int16_t)rd16(m, obj + 0x60u);
    wr8(m, obj + 0x1Fu, (uint8_t)d0);
    wr8(m, obj + 0x1Eu, (uint8_t)d0);
    d0 = (int16_t)(0x100 - d0);
    int32_t x = ((int32_t)d0 * (int16_t)rd16(m, obj + 0x54u)) << 8;
    int32_t y = ((int32_t)d0 * (int16_t)rd16(m, obj + 0x58u)) << 8;
    wr32(m, obj + 0x14u, (uint32_t)(x + 0x00A00000));
    wr32(m, obj + 0x18u, (uint32_t)(y + 0x00740000));
}

/* ROM 0x0D8318 */
void Obj_0D8318(const f3mem_t *m, uint32_t obj, uint32_t a2, int16_t d0, int16_t d1)
{
    uint16_t save_x = rd16(m, obj + 0x14u);
    uint16_t save_y = rd16(m, obj + 0x18u);
    if (rd8(m, obj + 0x64u) != 0u) {
        d0 = (int16_t)(-d0);
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u));
    }
    int16_t ox = (int16_t)(rd16(m, a2 + 2u) << 3);
    int16_t oy = (int16_t)(rd16(m, a2) << 3);
    d0 = (int16_t)(d0 - ox);
    d1 = (int16_t)(d1 - oy);
    int16_t sx = (int16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x100u);
    int16_t sy = (int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x100u);
    int32_t dx = ((int32_t)sx * (int32_t)d0) >> 8;
    int32_t dy = ((int32_t)sy * (int32_t)d1) >> 8;
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)save_x - (int16_t)dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)save_y - (int16_t)dy));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x18u, save_y);
    wr16(m, obj + 0x14u, save_x);
}

/* ROM 0x0D8294 */
void Obj_0D8294(const f3mem_t *m, uint32_t obj)
{
    uint16_t save_1c = rd16(m, obj + 0x1Cu);
    uint32_t a1 = rd32(m, 0x000D8446u + (uint32_t)rd16(m, obj + 0x5Cu) * 4u);
    uint8_t idx = rd8(m, a1++);
    Obj_Animate(m, obj, rd32(m, 0x000D83E6u + (uint32_t)idx * 4u));
    wr8(m, obj + 0x1Cu, rd8(m, a1++));
    wr8(m, obj + 0x1Du, 0x82u);
    int16_t d0 = (int8_t)rd8(m, a1++);
    int16_t d1 = (int8_t)rd8(m, a1++);
    Obj_0D8318(m, obj, rd32(m, obj + 0x10u), d0, d1);
    wr16(m, obj + 0x1Cu, save_1c);

    for (;;) {
        uint16_t save_34 = rd16(m, obj + 0x34u);
        uint16_t save_1c2 = rd16(m, obj + 0x1Cu);
        uint32_t a2 = rd32(m, a1); a1 += 4u;
        wr32(m, obj + 0x10u, a2);
        d0 = (int8_t)rd8(m, a1++);
        d1 = (int8_t)rd8(m, a1++);
        wr8(m, obj + 0x1Cu, rd8(m, a1++));
        wr8(m, obj + 0x4Fu, rd8(m, a1++));
        Obj_BlinkColour(m, obj);
        Obj_0D8318(m, obj, a2, d0, d1);
        wr16(m, obj + 0x1Cu, save_1c2);
        wr16(m, obj + 0x34u, save_34);
        if ((int16_t)rd16(m, a1) < 0)
            break;
    }
    Obj_BlinkColour(m, obj);
}

/* ROM 0x0D8378 */
void Obj_0D8378(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u) {
        int16_t v = (int16_t)(rd16(m, obj + 0x42u) + 4);
        wr16(m, obj + 0x42u, (uint16_t)v);
        if (v >= 0x100)
            wr8(m, obj + 0x22u, 0xFFu);
    } else {
        int16_t v = (int16_t)(rd16(m, obj + 0x42u) - 4);
        wr16(m, obj + 0x42u, (uint16_t)v);
        if (v <= 0) {
            wr8(m, obj + 0x22u, 0x00u);
            uint8_t r = (uint8_t)((Rng_NextMem(m) & 0x0Fu) - 0x88u);
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + r));
        }
    }
    uint16_t d0 = rd16(m, obj + 0x20u);
    int16_t d1 = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d0 * 2u);
    int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d0 * 2u);
    wr32(m, obj + 0x54u, (uint32_t)((int32_t)rd32(m, obj + 0x54u) + (((int32_t)cos_v * (int32_t)d1) >> 8)));
    wr32(m, obj + 0x58u, (uint32_t)((int32_t)rd32(m, obj + 0x58u) + (((int32_t)sin_v * (int32_t)d1) >> 8)));
}
