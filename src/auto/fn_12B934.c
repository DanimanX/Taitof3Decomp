#include "fn_auto.h"
/* ROM 0x12B934: initializes the ricochet emitter and spawns its first projectile. */
#include "fn_call_frames.h"

int Fn_12B934(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 8u);
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u));

    r->d[0] = rd32(m, obj + 0xCu); /* 0x12B944: packed X, Y, heading */
    r->d[1] = (uint32_t)(int32_t)(int8_t)(r->d[0] >> 24);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + r->d[1]));
    r->d[1] = (uint32_t)(int32_t)(int8_t)(r->d[0] >> 16);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + r->d[1]));
    r->d[1] = (uint8_t)(r->d[0] >> 8);
    wr16(m, obj + 0x20u, (uint16_t)r->d[1]);

    wr16(m, obj + 0x2Cu, 3u);
    wr16(m, obj + 0x2Eu, 0xCu);
    wr16(m, obj + 0x30u, 3u);
    wr16(m, obj + 0x32u, 0xCu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr16(m, obj + 0x22u, 7u);

    call_spawn(m, r, 0x12B99Eu, 0x12B9C2u, 9u, 2u, rd32(m, obj + 0xCu)); /* 0x12B986 */
    wr16(m, obj + 6u, 4u);
    wr32(m, obj, 0x12B9B2u); /* 0x12B9AA */
    r->a[7] += 4u;
    return 0;
}
