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
