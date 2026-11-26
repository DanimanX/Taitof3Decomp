/* ROM 0x1897D8: boss #4 initialization/intro countdown tick (A0 = object).
 * Counts down +0x06; while > 0 re-arms 0x1897D8. When <= 0, loads config
 * table at 0x189780, resets boss globals (0x1847E8) and uploads 8 boss
 * palettes (0x17B3AE), sets +0x64 = 0x18A61E, A5+0x4B38 = 1, clears
 * A5+0x4B26 bit 4, queues 9 palette uploads from 0x189786..0x1897A6 to
 * 0x7480..0x1A00, resets +0x62, sets A5+0x4B72 = -1, sets A5+0x4B3A bit 1,
 * clears A5+0x4B7C, configures A5+0x4B2A..0x4B32, initializes line RAM via
 * 0x14D70C, spawns 0x18A4D4 (stored in A5+0x4B74) and three 0x118D12
 * sub-objects, queues palette 0x1A880 -> 0x5780, and advances to 0x1899B0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x176198: fill 0xE8 words backwards from buf+(A5-0x5D56)+0x1D0. */
static void sub_176198(const f3mem_t *m, uint32_t buf, uint16_t val)
{
    uint32_t a0 = buf + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u) + 0x01D0u;
    for (int i = 0; i < 0xE8; i++) {
        a0 -= 2u;
        wr16(m, a0, val);
    }
}

/* ROM 0x14D70C: set line RAM bit 14 on 0x62B200 and fill 0x627600/626000/626200. */
static void sub_14D70C(const f3mem_t *m, uint32_t *d0_io, uint32_t *d1_io)
{
    uint32_t a0 = 0x0062B200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint16_t d1w = 0x4000u;
    uint16_t d0w = 0u;
    for (int16_t d7 = 0x00E7; d7 >= 0; d7--) {          /* 0x1766B6 */
        d0w = (uint16_t)((rd16(m, a0 + (uint32_t)d7 * 2u) & 0x3FFFu) | d1w);
        wr16(m, a0 + (uint32_t)d7 * 2u, d0w);
    }
    *d0_io = setw(*d0_io, d0w);
    *d1_io = setw(*d1_io, d1w);
    sub_176198(m, 0x00627600u, 0xDDD9u);                /* 0x14D71C (-0x2227) */
    sub_176198(m, 0x00626000u, 0x02BFu);                /* 0x14D730 */
    sub_176198(m, 0x00626200u, 0xBBBBu);                /* 0x14D744 (-0x4445) */
}

int Fn_1897D8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x1897D8 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {
        wr32(m, obj, 0x001897D8u);                      /* 0x1897D0 */
        return 0;
    }

    /* 0x1897DE..0x1899A8: init boss from config table at 0x189780 */
    a1 = 0x00189780u;
    Boss_ResetGlobals(m, obj);                          /* 0x1897E6 */
    Boss_UploadPalettes(m);                             /* 0x1897EC */

    wr32(m, obj + 0x64u, rd32(m, a1));                  /* 0x1897F2 */
    a1 += 4u;
    wr16(m, F3_A5 + 0x4B38u, rd16(m, a1));              /* 0x1897F6 */
    a1 += 2u;
    wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u); /* 0x1897FA */

    static const uint16_t dsts[9] = {
        0x7480u, 0x7400u, 0x7E00u, 0x7E80u, 0x7F00u,
        0x5D00u, 0x1980u, 0x7D00u, 0x1A00u
    };
    for (int i = 0; i < 9; i++) {
        Pal_QueueUpload(m, rd32(m, a1), dsts[i], 0x0020u);
        a1 += 4u;
    }

    wr16(m, obj + 0x62u, 0u);                           /* 0x1898D8 */
    wr16(m, F3_A5 + 0x4B72u, 0xFFFFu);                  /* 0x1898DC */
    wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) | 0x02u); /* 0x1898E2 {14:1} */
    wr16(m, F3_A5 + 0x4B7Cu, 0u);                       /* 0x1898E8 */
    wr16(m, F3_A5 + 0x4B2Au, 0x003Cu);                  /* 0x1898EC */
    wr16(m, F3_A5 + 0x4B2Cu, 0x0046u);                  /* 0x1898F2 */
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);                  /* 0x1898F8 */
    wr16(m, F3_A5 + 0x4B32u, 0x0010u);                  /* 0x1898FE */

    sub_14D70C(m, &d0, &d1);                            /* 0x189908 */

    d0 = Obj_Spawn(m, obj, 0x0018A4D4u, 3u, 2u, 0u);    /* 0x189924 */
    wr32(m, F3_A5 + 0x4B74u, d0);                       /* 0x18992E */
    d0 = Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018C3E8u); /* 0x189946 */
    d0 = Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018C3F6u); /* 0x189964 */
    d0 = Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018C404u); /* 0x189982 */
    Pal_QueueUpload(m, 0x0001A880u, 0x5780u, 0x0020u);  /* 0x18999E */

    wr32(m, obj, 0x001899B0u);                          /* 0x1899A8 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
