#include "obj_C9B3E.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C9B3E: shield "petal" (state 0x0C9B6A): tracks its parent, reads a
 * frame from the 6-byte record script at obj+0x44 and derives the sprite size /
 * offsets from it. Freed when the parent's +0x5A counter runs out or +0x65 bit
 * 0 is set. */
int Obj_C9B3E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0C9B3E */
    wr8(m, obj + 0x22u, 0x02u);
    wr32(m, obj + 0x44u, 0x000C9B6Au);
    wr32(m, obj + 0x24u, 0x27100000u);
    wr32(m, obj + 0x28u, 0x0000B800u);
    wr8(m, obj + 0x1Fu, 0u);
    wr8(m, obj + 0x1Eu, 0x80u);

    /* 0x0C9BCA */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x5Au, (uint16_t)(rd16(m, a1 + 0x5Au) - 1u));
    if ((int16_t)rd16(m, a1 + 0x5Au) < 0 || (rd8(m, a1 + 0x65u) & 0x01u) != 0u) {
        /* 0x0C9D68 */
        wr16(m, a1 + 0x5Au, (uint16_t)(rd16(m, a1 + 0x5Au) + 1u));
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x0C9BE0 */
    wr32(m, obj + 0x54u, 0u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x28u));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0Fu));

    uint32_t a2 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a2)));
    a2 += 2u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a2));
    a2 += 1u;
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a2));
    a2 += 1u;
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a2));
    a2 += 1u;
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a2));
    a2 += 1u;
    wr32(m, obj + 0x44u, a2);

    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);

    /* 0x0C9C3A: per-object motion record at obj+0x0C */
    a2 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a2)));
    a2 += 1u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a2)));
    a2 += 1u;
    wr32(m, obj + 0x48u, (uint32_t)((int32_t)(int16_t)rd16(m, a2) << 8));
    a2 += 2u;
    wr32(m, obj + 0x4Cu, (uint32_t)((int32_t)(int16_t)rd16(m, a2) << 8));

    wr32(m, obj + 0x00u, 0x000C9C6Au);
    return 0;
}
