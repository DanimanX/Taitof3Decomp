/* ROM 0x0CF098: group-9 object batch spawner (A0 = parent, A1 = tick fn, D1.w = count).
 * Spawns D1.w group-9 objects (ngroups=2, arg=0) with tick A1, storing each
 * pointer into A5+0x2862[] and clearing +0x4E. On allocation failure, frees all
 * previously spawned objects in A5+0x2862[] and returns negative D0. On success,
 * copies parent +0x4E to the last spawned object and returns D0.w = 0, D1.w = 0xFFFF. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0CF098(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = r->a[1];
    uint16_t d1w = (uint16_t)(r->d[1] - 1u);            /* 0x0CF098 */
    uint32_t a2 = F3_A5 + 0x2862u;                      /* 0x0CF09A */
    uint32_t a3 = 0u;

    for (;;) {
        uint32_t sp = Obj_Spawn(m, obj, a1, 9u, 2u, 0u); /* 0x0CF0AC */
        wr32(m, a2, sp);                                /* 0x0CF0B6 */
        a2 += 4u;
        if ((int32_t)sp < 0) {                          /* 0x0CF0B8 */
            a2 = F3_A5 + 0x2862u;                       /* 0x0CF0D0 */
            for (;;) {
                uint32_t p = rd32(m, a2);               /* 0x0CF0D4 */
                a2 += 4u;
                if ((int32_t)p < 0) {                   /* 0x0CF0D6 */
                    r->d[0] = p;
                    r->d[1] = setw(r->d[1], d1w);
                    return 0;
                }
                Obj_Free(m, p);                         /* 0x0CF0DA */
            }
        }
        a3 = sp;                                        /* 0x0CF0BC */
        wr8(m, a3 + 0x4Eu, 0u);                         /* 0x0CF0BE */
        if (d1w == 0u)                                  /* 0x0CF0C2 */
            break;
        d1w--;
    }

    wr8(m, a3 + 0x4Eu, rd8(m, obj + 0x4Eu));            /* 0x0CF0C6 */
    r->d[0] = setw(r->d[0], 0u);                        /* 0x0CF0CC */
    r->d[1] = setw(r->d[1], 0xFFFFu);
    return 0;
}
