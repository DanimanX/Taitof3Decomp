/* ROM 0x18767C: boss encounter init after +0x06 countdown reaches 0 (A0 = object).
 * Decrements +0x06 and re-arms 0x18767C while > 0. Once <= 0, resets boss
 * globals (0x1847E8), uploads boss palettes (0x17B3AE), loads +0x64=0x187DB6,
 * A5+0x4B38=1, 4 palette uploads (0x6200/0x6180/0x6380/0x6280) and spawns
 * 0x118CF6 from table 0x187634, initializes bounds/HP (0x01900000 scaled by
 * 0x18495E, sub-bars A5+0x4B0C/4B10/4B14/4B18), spawns boss parts (0x17A8D0,
 * 0x17AAEE, 0x118D12, 0x17B056, 0x17A664), and arms 0x187938. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_18767C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x18767C */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {                                        /* 0x187680 */
            wr32(m, obj + 0x00u, 0x0018767Cu);              /* 0x187674 */
            return 0;
        }
    }

    Boss_ResetGlobals(m, obj);                              /* 0x18768A (0x1847E8) */
    Boss_UploadPalettes(m);                                 /* 0x187690 (0x17B3AE) */

    uint32_t a1 = 0x00187634u;                              /* 0x187682 */
    wr32(m, obj + 0x64u, rd32(m, a1)); a1 += 4u;            /* 0x187696 */
    wr16(m, F3_A5 + 0x4B38u, rd16(m, a1)); a1 += 2u;        /* 0x18769A */
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u)); /* 0x18769E */

    Pal_QueueUpload(m, rd32(m, a1), 0x6200u, 0x0020u); a1 += 4u; /* 0x1876B2 */
    Pal_QueueUpload(m, rd32(m, a1), 0x6180u, 0x0020u); a1 += 4u; /* 0x1876CA */
    Pal_QueueUpload(m, rd32(m, a1), 0x6380u, 0x0020u); a1 += 4u; /* 0x1876E2 */
    Pal_QueueUpload(m, rd32(m, a1), 0x6280u, 0x0020u); a1 += 4u; /* 0x1876FA */

    (void)Obj_Spawn(m, obj, 0x00118CF6u, 3u, 2u, rd32(m, a1)); a1 += 4u; /* 0x187714 */
    r->a[1] = a1;

    wr16(m, F3_A5 + 0x4B2Au, 0x000Au);                      /* 0x18771E */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0014u);                      /* 0x187724 */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                      /* 0x18772A */
    wr16(m, F3_A5 + 0x4B32u, 0x0014u);                      /* 0x187730 */

    wr32(m, obj + 0x24u, 0x01900000u);                      /* 0x187736 */
    Obj_18495E(m, obj);                                     /* 0x18773E */

    {
        uint32_t hp = rd32(m, obj + 0x24u);                 /* 0x187744 */
        wr32(m, F3_A5 + 0x4B0Cu, hp);                       /* 0x187748 */
        wr32(m, F3_A5 + 0x4B10u, hp);                       /* 0x18774C */
        wr32(m, F3_A5 + 0x4B14u, hp);                       /* 0x187750 */
        uint32_t tenth = hp / 0x0Au;                        /* 0x187754 */
        wr32(m, F3_A5 + 0x4B18u, tenth);                    /* 0x18775C */
        wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 2u); /* 0x187760..0x18776C */
    }

    static const struct { uint32_t fn, arg; } spawns[15] = {
        { 0x0017A8D0u, 0x00188FD2u },
        { 0x0017A8D0u, 0x00188FE2u },
        { 0x0017A8D0u, 0x00188FC2u },
        { 0x0017A8D0u, 0x00188FF2u },
        { 0x0017AAEEu, 0x00189002u },
        { 0x0017AAEEu, 0x00189012u },
        { 0x0017A8D0u, 0x00189032u },
        { 0x0017A8D0u, 0x00189022u },
        { 0x00118D12u, 0x0018931Eu },
        { 0x00118D12u, 0x0018932Cu },
        { 0x0017B056u, 0x00000000u },
        { 0x0017A664u, 0x00187B26u },
        { 0x0017A664u, 0x00187B3Au },
        { 0x0017A664u, 0x00187B4Eu },
        { 0x0017A664u, 0x00187B62u },
    };
    uint32_t d0 = 0u;
    for (int i = 0; i < 15; i++)
        d0 = Obj_Spawn(m, obj, spawns[i].fn, 3u, 2u, spawns[i].arg);
    r->d[0] = d0;

    wr32(m, obj + 0x00u, 0x00187938u);                      /* 0x187930 */
    return 0;
}
