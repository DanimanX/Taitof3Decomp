/* ROM 0x19C304: boss encounter init after +0x06 countdown reaches 0 (A0 = object).
 * Decrements +0x06 and re-arms 0x19C304 while > 0. Once <= 0, resets boss
 * globals (0x1847E8), uploads boss palettes (0x17B3AE), sets +0x64 = 0x19C7A8,
 * A5+0x4B38 = 1, clears bit 4 of A5+0x4B26 and +0x62, initializes bounds and
 * HP (0x03E80000 scaled by 0x18495E, sub-bars A5+0x4B0C/4B10/4B14/4B18),
 * spawns 16 parts (0x17A8D0 / 0x17AAEE), 20 parts (0x19D7D2, index 19..0),
 * boss controller 0x19D610 in group 11 (saved in A5+0x4B40), queues palette
 * upload 0x01E740 -> 0x6D00, spawns 0x19D1DA and 3x 0x17A664, and arms 0x19C648. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_19C304(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x19C304 */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {                                        /* 0x19C308 */
            wr32(m, obj + 0x00u, 0x0019C304u);              /* 0x19C2FC */
            return 0;
        }
    }

    Boss_ResetGlobals(m, obj);                              /* 0x19C30A (0x1847E8) */
    Boss_UploadPalettes(m);                                 /* 0x19C310 (0x17B3AE) */

    wr32(m, obj + 0x64u, 0x0019C7A8u);                      /* 0x19C316 */
    wr16(m, F3_A5 + 0x4B38u, 1u);                           /* 0x19C31E */
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u)); /* 0x19C324 */
    wr16(m, obj + 0x62u, 0u);                               /* 0x19C32A */
    wr16(m, F3_A5 + 0x4B2Au, 0x003Cu);                      /* 0x19C32E */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0050u);                      /* 0x19C334 */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                      /* 0x19C33A */
    wr16(m, F3_A5 + 0x4B32u, 0x0020u);                      /* 0x19C340 */

    wr32(m, obj + 0x24u, 0x03E80000u);                      /* 0x19C346 */
    Obj_18495E(m, obj);                                     /* 0x19C34E */

    {
        uint32_t hp = rd32(m, obj + 0x24u);                 /* 0x19C354 */
        wr32(m, F3_A5 + 0x4B0Cu, hp);
        wr32(m, F3_A5 + 0x4B10u, hp);
        wr32(m, F3_A5 + 0x4B14u, hp);
        uint32_t tenth = hp / 0x0Au;                        /* 0x19C364 */
        wr32(m, F3_A5 + 0x4B18u, tenth);
        wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 3u);
        wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - tenth);
    }

    wr16(m, F3_A5 + 0x4BC2u, 0u);                           /* 0x19C388 */

    static const struct { uint32_t fn, arg; } parts16[16] = {
        { 0x0017A8D0u, 0x0019E752u },
        { 0x0017A8D0u, 0x0019E762u },
        { 0x0017A8D0u, 0x0019E732u },
        { 0x0017A8D0u, 0x0019E742u },
        { 0x0017A8D0u, 0x0019E712u },
        { 0x0017A8D0u, 0x0019E722u },
        { 0x0017A8D0u, 0x0019E6F2u },
        { 0x0017A8D0u, 0x0019E702u },
        { 0x0017A8D0u, 0x0019E6D2u },
        { 0x0017A8D0u, 0x0019E6E2u },
        { 0x0017A8D0u, 0x0019E672u },
        { 0x0017A8D0u, 0x0019E682u },
        { 0x0017A8D0u, 0x0019E692u },
        { 0x0017A8D0u, 0x0019E6B2u },
        { 0x0017A8D0u, 0x0019E6C2u },
        { 0x0017AAEEu, 0x0019E6A2u },
    };
    for (int i = 0; i < 16; i++)
        (void)Obj_Spawn(m, obj, parts16[i].fn, 3u, 2u, parts16[i].arg);

    uint32_t d7_hi = r->d[7] & 0xFFFF0000u;
    for (int16_t d7 = 0x13; d7 >= 0; d7--)                  /* 0x19C56C..0x19C58A */
        (void)Obj_Spawn(m, obj, 0x0019D7D2u, 3u, 2u, d7_hi | (uint16_t)d7);

    wr32(m, F3_A5 + 0x4B40u, Obj_Spawn(m, obj, 0x0019D610u, 0x0Bu, 1u, 0u)); /* 0x19C58E */
    Pal_QueueUpload(m, 0x0001E740u, 0x6D00u, 0x0020u);      /* 0x19C5AE */
    (void)Obj_Spawn(m, obj, 0x0019D1DAu, 3u, 2u, 0u);       /* 0x19C5CA */

    static const uint32_t args3[3] = { 0x0019EF5Au, 0x0019EF82u, 0x0019EF96u };
    uint32_t d0 = 0u;
    for (int i = 0; i < 3; i++)
        d0 = Obj_Spawn(m, obj, 0x0017A664u, 3u, 2u, args3[i]);
    r->d[0] = d0;

    wr32(m, obj + 0x00u, 0x0019C648u);                      /* 0x19C640 */
    return 0;
}
