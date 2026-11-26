/* ROM 0x192CAA: boss encounter init after +0x06 countdown reaches 0 (A0 = object).
 * Decrements +0x06 and re-arms 0x192CAA while > 0. Once <= 0, resets boss
 * globals (0x1847E8), uploads boss palettes (0x17B3AE), loads +0x64=0x193248,
 * A5+0x4B98=0x1947CC, A5+0x4B38=1, and queues palettes 0x195476->0x5380 and
 * 0x1954F6->0x5300 from 0x192C58. Initializes boss flags/bounds/HP (0x03E80000
 * scaled by 0x18495E, sub-bars A5+0x4B0C/4B10/4B14/4B18), spawns 9x 0x193EE0,
 * 2x 0x17A8D0, and 0x1937F2, sets +0x06=2, and arms 0x192F02. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

int Fn_192CAA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x192CAA */
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {                                        /* 0x192CAE -> 0x192CA2 */
            wr32(m, obj + 0x00u, 0x00192CAAu);
            return 0;
        }
    }

    Boss_ResetGlobals(m, obj);                              /* 0x192CD6 (0x1847E8) */
    Boss_UploadPalettes(m);                                 /* 0x192CDC (0x17B3AE) */

    {
        uint32_t a1 = 0x00192C58u;                          /* 0x192CB0 lea (-0x5A,PC),A1 */
        wr32(m, obj + 0x64u, rd32(m, a1)); a1 += 4u;        /* 0x192CE2 */
        wr32(m, F3_A5 + 0x4B98u, rd32(m, a1)); a1 += 4u;    /* 0x192CE6 */
        wr16(m, F3_A5 + 0x4B38u, rd16(m, a1)); a1 += 2u;    /* 0x192CEA */
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u)); /* 0x192CEE */
        Pal_QueueUpload(m, rd32(m, a1), 0x5380u, 0x0020u); a1 += 4u; /* 0x192CF4..0x192D02 */
        Pal_QueueUpload(m, rd32(m, a1), 0x5300u, 0x0020u); a1 += 4u; /* 0x192D0C..0x192D1A */
        r->a[1] = a1;
    }

    wr16(m, obj + 0x62u, 0u);                               /* 0x192D24 */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                      /* 0x192D28 */
    wr16(m, F3_A5 + 0x4B32u, 0x0020u);                      /* 0x192D2E */
    wr16(m, obj + 0x50u, 0xFF9Cu);                          /* 0x192D34 (-0x64) */
    wr16(m, obj + 0x54u, 0xFF9Cu);                          /* 0x192D3A (-0x64) */
    wr16(m, F3_A5 + 0x4B96u, 0u);                           /* 0x192D40 */
    wr16(m, F3_A5 + 0x4B9Cu, 0xFFFFu);                      /* 0x192D44 */

    wr32(m, obj + 0x24u, 0x03E80000u);                      /* 0x192D4A */
    Obj_18495E(m, obj);                                     /* 0x192D52 */

    {
        uint32_t hp = rd32(m, obj + 0x24u);                 /* 0x192D58 */
        wr32(m, F3_A5 + 0x4B0Cu, hp);                       /* 0x192D5C */
        wr32(m, F3_A5 + 0x4B10u, hp);                       /* 0x192D60 */
        wr32(m, F3_A5 + 0x4B14u, hp);                       /* 0x192D64 */
        uint32_t tenth = hp / 0x0Au;                        /* 0x192D68 */
        wr32(m, F3_A5 + 0x4B18u, tenth);                    /* 0x192D70 */
        wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 5u); /* 0x192D74..0x192D80 */
        wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - rd32(m, F3_A5 + 0x4B18u)); /* 0x192D84..0x192D88 */
    }

    for (uint32_t arg = 0x00194A40u; arg <= 0x00194AC0u; arg += 0x10u) /* 0x192D8C..0x192E96 */
        (void)Obj_Spawn(m, obj, 0x00193EE0u, 3u, 2u, arg);

    (void)Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x001949D0u); /* 0x192E9A */
    (void)Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x001949E0u); /* 0x192EB8 */
    r->d[0] = Obj_Spawn(m, obj, 0x001937F2u, 3u, 2u, 0u);   /* 0x192ED6 */

    wr16(m, obj + 0x06u, 2u);                               /* 0x192EF2 (0x192EF8 beq folded) */
    wr32(m, obj + 0x00u, 0x00192F02u);                      /* 0x192EFA */
    return 0;
}
