#include "fn_auto.h"

/* ROM 0x0E437E: initialize the trail segment and arm its first tick. */
#include "fn_call_frames.h"

/* ROM 0x0CEC4C: initialize the object and its slot's movement parameters. */
static void sub_0CEC4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t params = r->a[1] + 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, params));
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, params + 2u));
    wr32(m, obj + 0x2Cu, rd32(m, params + 4u));
    wr32(m, obj + 0x30u, rd32(m, params + 8u));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 2u));
    wr16(m, obj + 6u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t slot = rd8(m, obj + 5u);
    if (rd8(m, obj + 4u) != 5u) /* 0x0CEC9E */
        slot += 0x20u;
    r->d[0] = set_word(r->d[0], slot);
    /* 0x0CECA6: brief extension 0x043C, D0.W * 4 at PC + 0x3C. */
    r->a[2] = rd32(m, 0x0CECE4u + 4u * slot);
    wr32(m, obj + 0xCu, r->a[2]);
    params += 12u;
    uint32_t chunk = r->a[2] + 8u;
    for (unsigned i = 0; i < 4u; i++, chunk += 8u, params += 6u) {
        wr32(m, chunk, 0u);
        wr16(m, chunk, rd16(m, params));
        wr32(m, chunk + 4u, rd32(m, params + 2u));
    }
    r->a[1] = params - 4u; /* 0x0CECDE reads the last long without advancing. */
    r->a[7] += 4u;
}

int Fn_0E437E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = 0x0DC84Eu; /* Extension PC 0x0E4380 - 0x7B32. */
    push_long(m, r, 0x0E4388u);
    sub_0CEC4C(m, r);
    wr8(m, obj + 0x23u, 7u);
    wr32(m, obj + 0x54u, 0xFFFFFFFFu);
    wr8(m, obj + 6u, 1u);
    wr32(m, obj, 0x0E43A4u);
    r->a[7] += 4u;
    return 0;
}
