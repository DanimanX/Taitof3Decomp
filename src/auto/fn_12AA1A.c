/* ROM 0x12AA1A: segmented snake/worm boss node initializer (A0 = object).
 * Calls 0x12B386 to unpack speed (+0x42) and script/anim (+0x3A) from +0x0C,
 * assign node index (+0x22) from parent->+0x22++, append to the parent's
 * +0x44/+0x48 doubly-linked chain, and clear state fields. Sets HP/maxHP
 * (+0x28/+0x24) to 0x10000, sets +0x34 bits 1, 3, 4, clears +0x0C/+0x0E,
 * spawns child 0x12B5BA (group 3, ngroups 2) if 1 <= +0x22 <= 5, stores
 * A0 in A5+0x3296 if +0x22 == 4, and arms 0x12AA8E. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x12B386: link object into parent (+0x08)->+0x48 chain and init fields. */
static void sub_12B386_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    wr16(m, obj + 0x20u, 0x4000u);                             /* 0x12B386 */
    uint32_t desc = rd32(m, obj + 0x0Cu);                      /* 0x12B38C */
    wr16(m, obj + 0x42u, rd16(m, desc));                       /* 0x12B390 */
    wr32(m, obj + 0x3Au, rd32(m, desc + 2u));                  /* 0x12B394 */

    uint32_t a1 = rd32(m, obj + 0x08u);                        /* 0x12B398 */
    uint16_t idx = rd16(m, a1 + 0x22u);                        /* 0x12B39C */
    wr16(m, obj + 0x22u, idx);
    wr16(m, a1 + 0x22u, (uint16_t)(idx + 1u));                 /* 0x12B3A2 */

    wr32(m, obj + 0x44u, 0u);                                  /* 0x12B3A8 */
    wr32(m, obj + 0x48u, 0u);                                  /* 0x12B3AC */
    for (;;) {                                                 /* 0x12B3B0 */
        uint32_t next = rd32(m, a1 + 0x48u);
        if (next == 0u)
            break;
        a1 = next;
    }
    wr32(m, obj + 0x44u, a1);                                  /* 0x12B3BC */
    wr32(m, a1 + 0x48u, obj);                                  /* 0x12B3C0 */
    wr32(m, obj + 0x36u, 0u);                                  /* 0x12B3C4 */
    wr16(m, obj + 0x34u, 0u);                                  /* 0x12B3C8 */
    wr16(m, obj + 0x1Eu, 0u);                                  /* 0x12B3CC */
    wr16(m, obj + 0x1Cu, 0u);                                  /* 0x12B3D0 */
    wr8(m, obj + 0x4Fu, 0u);                                   /* 0x12B3D4 */
    wr16(m, obj + 0x54u, 0u);                                  /* 0x12B3D8 */
    wr16(m, obj + 0x56u, 0u);                                  /* 0x12B3DC */

    r->d[0] = 0u;
    r->a[1] = a1;
}

int Fn_12AA1A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_12B386_regs(m, r, obj);                                /* 0x12AA1A */
    wr32(m, obj + 0x28u, 0x00010000u);                         /* 0x12AA1E */
    wr32(m, obj + 0x24u, 0x00010000u);                         /* 0x12AA26 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* 0x12AA2E */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u)); /* 0x12AA34 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x12AA3A */
    wr16(m, obj + 0x0Cu, 0u);                                  /* 0x12AA40 */
    wr16(m, obj + 0x0Eu, 0u);                                  /* 0x12AA44 */

    int16_t k = (int16_t)rd16(m, obj + 0x22u);                 /* 0x12AA48..0x12AA58 */
    if (k >= 1 && k <= 5)
        r->d[0] = Obj_Spawn(m, obj, 0x0012B5BAu, 3u, 2u, 0u);  /* 0x12AA6E */

    if ((int16_t)rd16(m, obj + 0x22u) == 4)                    /* 0x12AA78..0x12AA7E */
        wr32(m, F3_A5 + 0x3296u, obj);                         /* 0x12AA82 */

    wr32(m, obj + 0x00u, 0x0012AA8Eu);                         /* 0x12AA86 */
    return 0;
}
