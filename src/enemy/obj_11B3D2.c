#include "obj_11B3D2.h"
#include "../objects/obj_pool.h"

/* ROM 0x11B3D2: fragment spawned by 0x11B472 (whose tail 0x11B434 leaves
 * fn = 0x11B472 behind). Position from the parent, heading from the spawn arg
 * plus a random +-0x2000 wobble, then {hit box 2..0xD, speed +0x4C = 0x64,
 * timer +0x42 = 0x14, contact damage +0x28 = 0x27100000}, screen anchor
 * +0x50/+0x52 = 0x94/0x4C + rng[0..0x3F] and the life timer +0x06.
 * Arg layout, as 0x11B472 spawns it: high word -> +0x22, low word -> +0x20. */
int Obj_11B3D2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x0CBD86: clear flip/zoom/flags/anim ptr/timer and two longs */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);

    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));

    uint32_t d0 = rd32(m, obj + 0x0Cu);          /* spawn arg  */
    wr16(m, obj + 0x20u, (uint16_t)d0);          /* heading    */
    wr16(m, obj + 0x22u, (uint16_t)(d0 >> 16));  /* fragment n */

    uint16_t r = (uint16_t)(Rng_NextMem(m) & 0x3FFFu);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)(r - 0x2000u)));

    wr16(m, obj + 0x42u, 0x14u);
    wr16(m, obj + 0x2Cu, 0x0002u);
    wr16(m, obj + 0x2Eu, 0x000Du);
    wr16(m, obj + 0x30u, 0x0002u);
    wr16(m, obj + 0x32u, 0x000Du);
    wr16(m, obj + 0x4Cu, 0x0064u);
    wr32(m, obj + 0x28u, 0x27100000u);

    /* ROM 0x11B434 */
    r = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    wr16(m, obj + 0x50u, (uint16_t)(r + 0x94u));
    r = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    wr16(m, obj + 0x52u, (uint16_t)(r + 0x4Cu));
    r = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    wr16(m, obj + 0x06u, (uint16_t)(r + 0x46u));

    wr32(m, obj + 0x00u, 0x11B472u);
    return 0;
}
