/* ROM 0x137F9C: multi-segment enemy head/chain initializer (A0 = object).
 * Calls Obj_ResetFields (0x0CBD86), clears +0x4F, sets +0x4C=0x1388, loads
 * +0x50/+0x52/+0x20 from +0x0C parameter record, clears +0x44/+0x48/+0x22,
 * spawns 5 chained 0x1380B4 segments (group 3, 2 groups; each receiving the
 * previous spawned handle as arg, starting with A0), sets HP +0x24=0x64,
 * +0x28=1, hitbox [-9..9]x[-9..9], speed +0x42=0x30, and arms 0x13801E. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_137F9C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);                                  /* 0x137F9C (0x0CBD86) */
    wr8(m, obj + 0x4Fu, 0u);                                  /* 0x137FA2 */
    wr16(m, obj + 0x4Cu, 0x1388u);                            /* 0x137FA6 */

    uint32_t a1 = rd32(m, obj + 0x0Cu);                       /* 0x137FAC */
    wr16(m, obj + 0x50u, rd16(m, a1)); a1 += 2u;              /* 0x137FB0 */
    wr16(m, obj + 0x52u, rd16(m, a1)); a1 += 2u;              /* 0x137FB4 */
    wr16(m, obj + 0x20u, rd16(m, a1)); a1 += 2u;              /* 0x137FB8 */
    r->a[1] = a1;

    wr32(m, obj + 0x44u, 0u);                                 /* 0x137FBC */
    wr32(m, obj + 0x48u, 0u);                                 /* 0x137FC0 */
    wr16(m, obj + 0x22u, 0u);                                 /* 0x137FC4 */

    uint32_t d0 = obj;                                        /* 0x137FC8 */
    for (int i = 4; i >= 0; i--)                              /* 0x137FCA..0x137FE8 */
        d0 = Obj_Spawn(m, obj, 0x001380B4u, 3u, 2u, d0);
    r->d[0] = d0;
    r->d[1] = setw(r->d[1], 0xFFFFu);

    wr16(m, obj + 0x24u, 0x0064u);                            /* 0x137FEC */
    wr16(m, obj + 0x28u, 0x0001u);                            /* 0x137FF2 */
    wr16(m, obj + 0x2Cu, 0xFFF7u);                            /* 0x137FF8 (-9) */
    wr16(m, obj + 0x30u, 0xFFF7u);                            /* 0x137FFE (-9) */
    wr16(m, obj + 0x2Eu, 0x0009u);                            /* 0x138004 */
    wr16(m, obj + 0x32u, 0x0009u);                            /* 0x13800A */
    wr16(m, obj + 0x42u, 0x0030u);                            /* 0x138010 */
    wr32(m, obj, 0x0013801Eu);                                /* 0x138016 */
    return 0;
}
