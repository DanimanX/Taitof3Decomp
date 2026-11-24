#include "obj_17E1DA.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17BDA2: the four byte fields of +0x0C drive velocity and angle */
static void sub_17BDA2(const f3mem_t *m, uint32_t obj)
{
    uint32_t f = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x16u, (uint16_t)(int16_t)(int8_t)(f >> 24));
    wr16(m, obj + 0x1Au, (uint16_t)(int16_t)(int8_t)(f >> 16));
    wr8(m, obj + 0x20u, 0x00u);
    wr8(m, obj + 0x20u, (uint8_t)(f & 0xFFu));
}

/* ROM 0x184888: seed stats from the record at A1 */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1 + 0x00u));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x02u));
    wr16(m, obj + 0x24u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x28u, rd16(m, a1 + 0x06u));
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x08u));
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Cu));
    wr16(m, obj + 0x62u, rd16(m, a1 + 0x10u));
    wr32(m, obj + 0x66u, rd32(m, a1 + 0x12u));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);
}

/* ROM 0x17E1DA: spawn-wait tail. On expiry it stamps 8 into +0x0C{16:8},
 * re-derives velocity/angle via 0x17BDA2, plays sound 0x1A, seeds stats from
 * 0x17E290, starts animation index 2 and spawns the 0x17C4D4 child. */
int Obj_17E1DA_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0017E1DAu);
        return 0;
    }

    {
        uint32_t f = rd32(m, obj + 0x0Cu);
        wr32(m, obj + 0x0Cu, (f & 0xFF00FFFFu) | 0x00080000u); /* bfins {16:8} */
    }
    sub_17BDA2(m, obj);

    Sound_PlayCmd(m, 0x1A);
    sub_184888(m, obj, 0x0017E290u);

    {
        uint32_t a1 = 0x0017E2A6u;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }

    wr32(m, obj + 0x44u, 0x0017E328u);
    wr16(m, obj + 0x48u, rd16(m, 0x0017E32Cu));
    wr16(m, obj + 0x4Au, 0x0000u);

    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 3u, 2u, 0x00000007u);
        wr32(m, obj + 0x62u, child);
        if ((int32_t)child <= 0)
            return -1;
    }

    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = rd16(m, a1); a1 += 2u;
        uint16_t d0 = rd16(m, a1);
        wr16(m, obj + 0x16u, (uint16_t)(d0 << 3));
        wr16(m, obj + 0x1Au, (uint16_t)(d1 << 3));
    }
    wr16(m, obj + 0x58u, 0x0000u);
    wr16(m, obj + 0x5Au, 0x0000u);
    wr8(m, obj + 0x34u, 0x02u);
    wr32(m, obj + 0x44u, 0x0017D4C0u);
    wr32(m, obj + 0x0Cu, 0x0017D4C0u);
    wr32(m, obj + 0x00u, 0x0017C240u);
    return 0;
}
