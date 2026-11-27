#include "fn_auto.h"
/* ROM 0x12B9B2: Count down between child spawns, then set up the next movement state. */
#include "../objects/obj_pool.h"

int Fn_12B9B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t delay = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(delay - 1u)); /* 0x12B9B2 */
    if ((int16_t)delay <= 1) {
        uint16_t count = rd16(m, obj + 0x22u);
        wr16(m, obj + 0x22u, (uint16_t)(count - 1u)); /* 0x12B9B8 */
        if ((int16_t)count <= 1) {
            /* 0x12B9BE branches to the movement setup at 0x12B9E4. */
            wr16(m, obj + 0x1Eu, 0u);
            wr16(m, obj + 0x1Cu, 0u);
            wr32(m, obj + 0x24u, 0x640000u);
            wr32(m, obj + 0x28u, 0x10000u);
            wr16(m, obj + 0x42u, 0xEu);
            wr16(m, obj + 0x34u, 0u);
            wr8(m, obj + 6u, 0u);
            wr32(m, obj, 0x12BA12u);
            r->a[7] += 4u;
            return 0;
        }

        /* 0x12B986..0x12B99E: push arguments and save 0x01057C's MOVEM area. */
        uint32_t arg = rd32(m, obj + 0xCu);
        uint32_t sp = r->a[7];
        wr32(m, sp - 4u, arg);
        wr16(m, sp - 6u, 2u);
        wr16(m, sp - 8u, 9u);
        wr32(m, sp - 12u, 0x12B9C2u);
        wr32(m, sp - 16u, 0x12B99Eu);
        for (unsigned i = 3u; i > 0u; --i)
            wr32(m, sp - 32u + 4u * i, r->a[i - 1u]);
        for (unsigned i = 4u; i > 0u; --i)
            wr32(m, sp - 48u + 4u * i, r->d[i]);
        r->d[0] = Obj_Spawn(m, obj, 0x12B9C2u, 9u, 2u, arg);
        wr16(m, obj + 6u, 4u); /* 0x12B9A2 clears Z. */
    }
    wr32(m, obj, 0x12B9B2u); /* 0x12B9AA */
    r->a[7] += 4u;
    return 0;
}
