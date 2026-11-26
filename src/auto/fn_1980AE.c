/* ROM 0x1980AE: boss encounter init after +0x06 countdown reaches 0 (A0 = object).
 * Decrements +0x06 and re-arms 0x1980AE while > 0. Once <= 0, resets boss
 * globals (0x1847E8), uploads boss palettes (0x17B3AE), loads +0x64=0x19905C
 * and A5+0x4B38=1 from 0x198098, initializes boss flags/bounds/HP (0x03E80000
 * scaled by 0x18495E, sub-bars A5+0x4B0C/4B10/4B14/4B18), spawns boss parts
 * (0x198CD2, 0x19B646 x2, 0x19853C, 0x1987E0, 0x199B98, 0x17B056, 0x17A664 x2),
 * queues 4 extra palettes (0x6D00/0x6E80/0x6580/0x5280), and arms 0x1982DE. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_1980AE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x1980AE */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {                                        /* 0x1980B2 */
            wr32(m, obj + 0x00u, 0x001980AEu);              /* 0x1980A6 */
            return 0;
        }
    }

    Boss_ResetGlobals(m, obj);                              /* 0x1980BC (0x1847E8) */
    Boss_UploadPalettes(m);                                 /* 0x1980C2 (0x17B3AE) */

    {
        uint32_t a1 = 0x00198098u;                          /* 0x1980B4 */
        wr32(m, obj + 0x64u, rd32(m, a1)); a1 += 4u;        /* 0x1980C8 */
        wr16(m, F3_A5 + 0x4B38u, rd16(m, a1)); a1 += 2u;    /* 0x1980CC */
        r->a[1] = a1;
    }

    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u)); /* 0x1980D0 */
    wr16(m, obj + 0x62u, 0u);                               /* 0x1980D6 */
    wr16(m, F3_A5 + 0x4BA8u, 0u);                           /* 0x1980DA */
    wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x10u)); /* 0x1980DE */
    wr16(m, obj + 0x50u, 0xFF9Cu);                          /* 0x1980E4 (-0x64) */
    wr16(m, obj + 0x54u, 0xFF9Cu);                          /* 0x1980EA (-0x64) */
    wr8(m, F3_A5 + 0x4BA8u, (uint8_t)(rd8(m, F3_A5 + 0x4BA8u) | 0x04u)); /* 0x1980F0 */
    wr16(m, F3_A5 + 0x4BB2u, 3u);                           /* 0x1980F6 */
    wr16(m, F3_A5 + 0x4B2Au, 0x0050u);                      /* 0x1980FC */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0064u);                      /* 0x198102 */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                      /* 0x198108 */
    wr16(m, F3_A5 + 0x4B32u, 0x0020u);                      /* 0x19810E */

    wr32(m, obj + 0x24u, 0x03E80000u);                      /* 0x198114 */
    Obj_18495E(m, obj);                                     /* 0x19811C */

    {
        uint32_t hp = rd32(m, obj + 0x24u);                 /* 0x198122 */
        wr32(m, F3_A5 + 0x4B0Cu, hp);                       /* 0x198126 */
        wr32(m, F3_A5 + 0x4B10u, hp);                       /* 0x19812A */
        wr32(m, F3_A5 + 0x4B14u, hp);                       /* 0x19812E */
        uint32_t tenth = hp / 0x0Au;                        /* 0x198132 */
        wr32(m, F3_A5 + 0x4B18u, tenth);                    /* 0x19813A */
        wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 3u); /* 0x19813E..0x19814A */
        wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - rd32(m, F3_A5 + 0x4B18u)); /* 0x19814E..0x198152 */
    }

    (void)Obj_Spawn(m, obj, 0x00198CD2u, 3u, 2u, 0u);       /* 0x198168 */
    (void)Obj_Spawn(m, obj, 0x0019B646u, 3u, 2u, 0x0019B72Cu); /* 0x198186 */
    (void)Obj_Spawn(m, obj, 0x0019B646u, 3u, 2u, 0x0019B73Cu); /* 0x1981A4 */
    wr32(m, F3_A5 + 0x4BAAu, Obj_Spawn(m, obj, 0x0019853Cu, 3u, 2u, 0x001989DAu)); /* 0x1981C2..0x1981CC */
    wr32(m, F3_A5 + 0x4BAEu, Obj_Spawn(m, obj, 0x001987E0u, 3u, 2u, 0x001989F2u)); /* 0x1981E4..0x1981EE */
    (void)Obj_Spawn(m, obj, 0x00199B98u, 3u, 2u, 0u);       /* 0x198204 */
    (void)Obj_Spawn(m, obj, 0x0017B056u, 3u, 2u, 0u);       /* 0x198220 */

    Pal_QueueUpload(m, 0x0001E740u, 0x6D00u, 0x0020u);      /* 0x19823C */
    Pal_QueueUpload(m, 0x0001AC00u, 0x6E80u, 0x0020u);      /* 0x198258 */
    Pal_QueueUpload(m, 0x0001E980u, 0x6580u, 0x0020u);      /* 0x198274 */
    Pal_QueueUpload(m, 0x00018180u, 0x5280u, 0x0020u);      /* 0x198290 */

    (void)Obj_Spawn(m, obj, 0x0017A664u, 3u, 2u, 0x0019C2CCu); /* 0x1982AE */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A664u, 3u, 2u, 0x0019C2E0u); /* 0x1982CC */

    wr32(m, obj + 0x00u, 0x001982DEu);                      /* 0x1982D6 */
    return 0;
}
