/* ROM 0x0CEFF8: allocate D1.w objects of function A1 in A5+0x2862[] (group 5, 2).
 * Clears +0x4E on each allocated slot, copies A0+0x4E to the last slot, and
 * returns D0.w = 0. If any allocation fails, frees all allocated slots in
 * A5+0x2862[] and returns D0 < 0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_0CEFF8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t fn = r->a[1];
    int16_t d1w = (int16_t)((uint16_t)r->d[1] - 1u);
    uint32_t a2 = F3_A5 + 0x2862u;
    uint32_t a3 = r->a[3];
    uint32_t d0 = r->d[0];

    do {
        d0 = Obj_Spawn(m, obj, fn, 5u, 2u, 0u);
        wr32(m, a2, d0);
        a2 += 4u;
        if ((int32_t)d0 < 0) {
            a2 = F3_A5 + 0x2862u;
            for (;;) {
                d0 = rd32(m, a2);
                a2 += 4u;
                if ((int32_t)d0 < 0) {
                    r->d[0] = d0;
                    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)d1w;
                    r->a[2] = a2;
                    r->a[3] = a3;
                    return 0;
                }
                Obj_Free(m, d0);
            }
        }
        a3 = d0;
        wr8(m, a3 + 0x4Eu, 0u);
    } while (d1w-- != 0);

    wr8(m, a3 + 0x4Eu, rd8(m, obj + 0x4Eu));
    r->d[0] = d0 & 0xFFFF0000u;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0xFFFFu;
    r->a[2] = a2;
    r->a[3] = a3;
    return 0;
}
