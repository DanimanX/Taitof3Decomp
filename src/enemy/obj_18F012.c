#include "obj_18F012.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"

/* ROM 0x18F012: boss countdown + init globals, palettes, HP, children, arm 0x18F1C2 */
int Obj_18F012_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0018F012u);
        return 0;
    }
    uint32_t a1 = 0x0018EFCCu;
    Boss_ResetGlobals(m, obj);
    wr32(m, obj + 0x64u, rd32(m, a1)); a1 += 4u;
    wr16(m, F3_A5 + 0x4B38u, rd16(m, a1)); a1 += 2u;
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
    Pal_QueueUpload(m, rd32(m, a1), 0x5E00u, 0x20u); a1 += 4u;
    Pal_QueueUpload(m, rd32(m, a1), 0x5E80u, 0x20u); a1 += 4u;
    Pal_QueueUpload(m, rd32(m, a1), 0x5F00u, 0x20u);
    wr16(m, obj + 0x62u, 0xFFFFu);
    wr16(m, F3_A5 + 0x4B2Au, 0x0050u);
    wr16(m, F3_A5 + 0x4B2Cu, 0x0032u);
    wr16(m, F3_A5 + 0x4B2Eu, 0x0028u);
    wr16(m, F3_A5 + 0x4B30u, 0x2020u);
    wr16(m, F3_A5 + 0x4B32u, 0x0010u);
    wr16(m, F3_A5 + 0x4B90u, 0u);
    wr32(m, obj + 0x24u, 0x07D00000u);
    Obj_18495E(m, obj);
    uint32_t d0 = rd32(m, obj + 0x24u);
    wr32(m, F3_A5 + 0x4B0Cu, d0);
    wr32(m, F3_A5 + 0x4B10u, d0);
    wr32(m, F3_A5 + 0x4B14u, d0);
    uint32_t tenth = d0 / 10u;
    wr32(m, F3_A5 + 0x4B18u, tenth);
    wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - tenth * 5u);
    wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - tenth);
    wr16(m, F3_A5 + 0x4B80u, 0u);
    wr16(m, F3_A5 + 0x4B82u, 0u);
    wr16(m, F3_A5 + 0x4B46u, 0u);
    wr16(m, F3_A5 + 0x4B44u, 0xFFFFu);
    wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x08u));
    wr8(m, F3_A5 + 0x4B8Eu, 7u);
    wr16(m, F3_A5 + 0x4B92u, 0u);
    (void)Obj_Spawn(m, obj, 0x0019090Cu, 6u, 1u, 0u);
    (void)Obj_Spawn(m, obj, 0x00118D12u, 5u, 2u, 0x001927AAu);
    (void)Obj_Spawn(m, obj, 0x0017A664u, 3u, 2u, 0x001928B8u);
    Pal_QueueUpload(m, 0x00019500u, 0x6400u, 0x20u);
    wr32(m, obj + 0x00u, 0x0018F1C2u);
    return 0;
}
