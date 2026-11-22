#include "obj_18765E.h"
#include "../objects/obj_pool.h"

/* ROM 0x001660: queue a palette upload {count.w, src.l, dst.w} (max 0x20) */
static void sub_001660(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count)
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

/* ROM 0x17B3AE: upload the 8 boss palettes (alternate set when A5+0x32DC == 1) */
static void sub_17B3AE(const f3mem_t *m)
{
    uint32_t a2 = 0x0017B4ACu;                  /* lea (0xf8,PC) */
    if (rd16(m, F3_A5 + 0x32DCu) == 0x0001u)
        a2 = 0x0017B6ECu;                       /* lea (0x328,PC) */
    for (uint32_t i = 0; i < 8u; i++)
        sub_001660(m, a2 + i * 0x40u, (uint16_t)(0x6800u + i * 0x80u), 0x20u);
}

/* ROM 0x1847E8: reset the boss globals at A5+0x4B02.. */
static void sub_1847E8(const f3mem_t *m, uint32_t obj)
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

/* ROM 0x18495E: +0x24 hit points scaled up by the difficulty table 0x1849E6 (PC base = extension word 0x18496C) */
static void sub_18495E(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 4);
    uint16_t k = rd8(m, 0x001849E6u + idx);
    uint16_t hp = rd16(m, obj + 0x24u);
    uint32_t d1 = (uint32_t)(uint16_t)(hp / 100u) * (uint32_t)k;   /* divu.w #100, mulu.w */
    wr16(m, obj + 0x24u, (uint16_t)(hp + (uint16_t)d1));
}

/* ROM 0x18765E */
int Obj_18765E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0018765Eu);   /* 0x187656 */
        return 0;
    }

    /* 0x187664: lea (-0x4c,PC),A1 -> 0x18761A, then 0x18768A */
    uint32_t a1 = 0x0018761Au;
    sub_1847E8(m, obj);
    sub_17B3AE(m);
    wr32(m, obj + 0x64u, rd32(m, a1)); a1 += 4u;
    wr16(m, F3_A5 + 0x4B38u, rd16(m, a1)); a1 += 2u;
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));   /* bfclr {4:1} */
    static const uint16_t dst[4] = { 0x6200u, 0x6180u, 0x6380u, 0x6280u };
    for (int i = 0; i < 4; i++) {
        sub_001660(m, rd32(m, a1), dst[i], 0x20u);
        a1 += 4u;
    }
    Obj_Spawn(m, obj, 0x00118CF6u, 3u, 2u, rd32(m, a1));
    a1 += 4u;

    /* 0x18771E */
    wr16(m, F3_A5 + 0x4B2Au, 0x000Au);
    wr16(m, F3_A5 + 0x4B2Cu, 0x0014u);
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);
    wr16(m, F3_A5 + 0x4B32u, 0x0014u);
    wr32(m, obj + 0x24u, 0x01900000u);
    sub_18495E(m, obj);
    uint32_t hp = rd32(m, obj + 0x24u);
    wr32(m, F3_A5 + 0x4B0Cu, hp);
    wr32(m, F3_A5 + 0x4B10u, hp);
    wr32(m, F3_A5 + 0x4B14u, hp);
    wr32(m, F3_A5 + 0x4B18u, hp / 10u);           /* divul.l #10 */
    wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - rd32(m, F3_A5 + 0x4B18u) * 2u);

    /* 0x187770: boss parts */
    static const struct { uint32_t fn, arg; } parts[] = {
        { 0x0017A8D0u, 0x00188FD2u }, { 0x0017A8D0u, 0x00188FE2u },
        { 0x0017A8D0u, 0x00188FC2u }, { 0x0017A8D0u, 0x00188FF2u },
        { 0x0017AAEEu, 0x00189002u }, { 0x0017AAEEu, 0x00189012u },
        { 0x0017A8D0u, 0x00189032u }, { 0x0017A8D0u, 0x00189022u },
        { 0x00118D12u, 0x0018931Eu }, { 0x00118D12u, 0x0018932Cu },
        { 0x0017B056u, 0x00000000u },
        { 0x0017A664u, 0x00187B26u }, { 0x0017A664u, 0x00187B3Au },
        { 0x0017A664u, 0x00187B4Eu }, { 0x0017A664u, 0x00187B62u },
    };
    for (unsigned i = 0; i < sizeof parts / sizeof parts[0]; i++)
        Obj_Spawn(m, obj, parts[i].fn, 3u, 2u, parts[i].arg);

    wr32(m, obj + 0x00u, 0x00187938u);   /* 0x187930 */
    return 0;
}
