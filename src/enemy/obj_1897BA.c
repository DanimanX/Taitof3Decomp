#include "obj_1897BA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

/* ROM 0x176198: fill 0xE8 words of line RAM starting at base + (A5 - 0x5D56) */
static void sub_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t i = 0; i < 0xE8u; i++)
        wr16(m, a0 + i * 2u, val);
}

/* ROM 0x14D70C: set bit 14 on 0x62B200 line RAM and fill 0x627600/0x626000/0x626200 */
static void sub_14D70C(const f3mem_t *m)
{
    uint32_t a0 = 0x0062B200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t i = 0; i < 0xE8u; i++) {
        uint16_t w = (uint16_t)((rd16(m, a0 + i * 2u) & 0x3FFFu) | 0x4000u);
        wr16(m, a0 + i * 2u, w);
    }
    sub_176198(m, 0x00627600u, (uint16_t)-0x2227);
    sub_176198(m, 0x00626000u, 0x02BFu);
    sub_176198(m, 0x00626200u, (uint16_t)-0x4445);
}

/* ROM 0x1897BA: countdown +0x06, then init boss globals/palettes/parts */
int Obj_1897BA_Tick(const f3mem_t *m, uint32_t obj)
{
    static const uint16_t pal_dsts[9] = {
        0x7480u, 0x7400u, 0x7E00u, 0x7E80u, 0x7F00u,
        0x5D00u, 0x1980u, 0x7D00u, 0x1A00u
    };

    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {
        wr32(m, obj + 0x00u, 0x001897BAu);
        return 0;
    }

    /* 0x1897C0: A1 = 0x001897C2 - 0x6C = 0x00189756 */
    uint32_t a1 = 0x00189756u;

    /* 0x1897E6 */
    Boss_ResetGlobals(m, obj);
    Boss_UploadPalettes(m);
    wr32(m, obj + 0x64u, rd32(m, a1));
    a1 += 4u;
    wr16(m, F3_A5 + 0x4B38u, rd16(m, a1));
    a1 += 2u;
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));

    for (uint32_t i = 0; i < 9u; i++) {
        Pal_QueueUpload(m, rd32(m, a1), pal_dsts[i], 0x20u);
        a1 += 4u;
    }

    wr16(m, obj + 0x62u, 0u);
    wr16(m, F3_A5 + 0x4B72u, 0xFFFFu);
    wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x02u));
    wr16(m, F3_A5 + 0x4B7Cu, 0u);
    wr16(m, F3_A5 + 0x4B2Au, 0x003Cu);
    wr16(m, F3_A5 + 0x4B2Cu, 0x0046u);
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);
    wr16(m, F3_A5 + 0x4B32u, 0x0010u);

    sub_14D70C(m);

    wr32(m, F3_A5 + 0x4B74u, Obj_Spawn(m, obj, 0x0018A4D4u, 3u, 2u, 0u));
    Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018C3E8u);
    Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018C3F6u);
    Obj_Spawn(m, obj, 0x00118D12u, 3u, 2u, 0x0018C404u);
    Pal_QueueUpload(m, 0x0001A880u, 0x5780u, 0x20u);

    wr32(m, obj + 0x00u, 0x001899B0u);
    return 0;
}
