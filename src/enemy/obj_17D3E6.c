#include "obj_17D3E6.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x184888: initialise the object's stats from the record at A1 */
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

/* ROM 0x17D3E6: spawn-wait tail. On expiry it places the object at the
 * parent's +0x50/+0x54 plus its own velocity, plays sound 0x2A, seeds stats
 * from 0x17D4C2, starts animation index 0, spawns the 0x17C4D4 child (group 3,
 * two groups, arg 0) and re-derives its velocity from the sprite record. */
int Obj_17D3E6_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        /* 0x17D3EA bgt.b -> 0x17D3DE, back into the wait body */
        wr32(m, obj + 0x00u, 0x0017D3E6u);
        return 0;
    }

    /* 0x17D3EC: follow the parent */
    {
        uint32_t parent = rd32(m, obj + 0x08u);
        uint16_t d0 = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t d1 = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
        wr16(m, obj + 0x14u, d0);
        wr16(m, obj + 0x18u, d1);
    }

    Sound_PlayCmd(m, 0x2A);                    /* 0x17D410 */
    sub_184888(m, obj, 0x0017D4C2u);           /* 0x17D42A */

    {
        uint32_t a1 = 0x0017D4D8u;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }

    wr32(m, obj + 0x44u, 0x0017D4E0u);         /* 0x17D44C */
    wr16(m, obj + 0x48u, rd16(m, 0x0017D4E4u));
    wr16(m, obj + 0x4Au, 0x0000u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 0x10u));
    wr16(m, obj + 0x20u, 0x8000u);

    /* 0x17D46C: Obj_Spawn(parent = this, 0x17C4D4, group 3, n = 2, arg 0) */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 3u, 2u, 0x00000000u);
        wr32(m, obj + 0x62u, child);
        if ((int32_t)child <= 0) {
            /* 0x17D48E ble.w -> 0x17C78E, not decompiled */
            return -1;
        }
    }

    /* 0x17D492: velocity from the sprite record, two words, each << 3 */
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
    wr32(m, obj + 0x0Cu, 0x0017D4C0u);

    /* 0x17D4BC bra.w 0x17C238: only installs 0x17C240 and returns */
    wr32(m, obj + 0x00u, 0x0017C240u);
    return 0;
}
