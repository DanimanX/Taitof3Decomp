/* ROM 0x0CF138: spawn D1.w chained objects (A0 = parent, A1 = tick fn).
 * Allocates D1.w objects (group 3, ngroups 2, arg 0) via 0x01057C, storing
 * their pointers at A5+0x2862. Copies parent +0x4E to the last spawned
 * object and clears D0.w. On allocation failure, frees all already-spawned
 * slots via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0CF138(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = r->a[1];
    uint32_t d0 = r->d[0];
    uint16_t d1w = (uint16_t)(r->d[1] - 1u);            /* 0x0CF138 */
    uint32_t a2 = F3_A5 + 0x2862u;                      /* 0x0CF13A */
    uint32_t a3 = r->a[3];

    do {
        d0 = Obj_Spawn(m, obj, a1, 3u, 2u, 0u);         /* 0x0CF13E..0x0CF14C */
        wr32(m, a2, d0);                                /* 0x0CF156 */
        a2 += 4u;
        if ((int32_t)d0 < 0) {                          /* 0x0CF158 */
            a2 = F3_A5 + 0x2862u;                       /* 0x0CF170 */
            for (;;) {
                d0 = rd32(m, a2);                       /* 0x0CF174 */
                a2 += 4u;
                if ((int32_t)d0 < 0)                    /* 0x0CF176 */
                    break;
                Obj_Free(m, d0);                        /* 0x0CF17A */
            }
            r->d[0] = d0;
            r->d[1] = setw(r->d[1], d1w);
            return 0;
        }
        a3 = d0;                                        /* 0x0CF15C */
        wr8(m, a3 + 0x4Eu, 0u);                         /* 0x0CF15E */
    } while (d1w-- != 0u);                              /* 0x0CF162 */

    wr8(m, a3 + 0x4Eu, rd8(m, obj + 0x4Eu));            /* 0x0CF166 */
    d0 = setw(d0, 0u);                                  /* 0x0CF16C */

    r->d[0] = d0;
    r->d[1] = setw(r->d[1], d1w);
    return 0;
}
