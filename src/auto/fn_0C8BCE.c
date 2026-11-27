#include "fn_auto.h"
/* ROM 0x0C8BCE: verified on a poke-forced capture (scripts/zcap_poke_weak.sh): Clear five entries in each of two adjacent tilemap rows. */

int Fn_0C8BCE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, r->a[2]); /* 0x0C8BCE MOVEM predecrement */
    wr32(m, sp - 8u, r->a[1]);
    wr32(m, sp - 12u, r->d[0]);

    /* 0x0C8BDC: extension 0x0320 is a full-format signed D0.w
     * index scaled by two, with word displacement 0x057C and no
     * indirection. The base is 0x0C8BDE, so the table is at 0x0C915A. */
    int32_t index = (int16_t)rd16(m, r->a[0] + 0x22u);
    int16_t offset = (int16_t)rd16(m, 0x0C915Au + (uint32_t)(2 * index));
    uint32_t tile = 0x61C000u + (uint32_t)(int32_t)offset;
    for (unsigned i = 0; i < 5u; ++i, tile += 4u) {
        wr32(m, tile + 0x80u, 0u); /* 0x0C8BEA */
        wr32(m, tile, 0u);         /* 0x0C8BEE */
    }

    r->d[0] = rd32(m, sp - 12u); /* 0x0C8BF4 MOVEM postincrement */
    r->a[1] = rd32(m, sp - 8u);
    r->a[2] = rd32(m, sp - 4u);
    return 0;
}
