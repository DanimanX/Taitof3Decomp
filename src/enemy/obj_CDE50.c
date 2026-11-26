#include "obj_CDE50.h"
#include "obj_CDF68.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0CE132: unpack packed trail history slot from parent +0x44. */
static uint32_t unpack_trail(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t idx = (uint16_t)((rd16(m, a1 + 0x3Au) - 2u) & 3u);
    uint32_t d0 = rd32(m, a1 + 0x4Cu + (uint32_t)idx * 4u);

    wr16(m, obj + 0x14u, (uint16_t)((int32_t)d0 >> 22));
    wr16(m, obj + 0x18u, (uint16_t)((int32_t)(d0 << 10) >> 23));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 5) & 0xFFu));
    wr16(m, obj + 0x28u, (uint16_t)((d0 >> 4) & 1u));
    wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | ((d0 >> 2) & 3u)));
    return d0;
}

/* ROM 0x0CE16E: pack object state into trail ring buffer at +0x4C, preserving
 * the low 2 bits of d0 (from the caller's D0) in the bfins chain. */
static void pack_trail(const f3mem_t *m, uint32_t obj, uint32_t d0_low2)
{
    uint32_t d0 =
        (((uint32_t)rd16(m, obj + 0x14u) & 0x3FFu) << 22) |
        (((uint32_t)rd16(m, obj + 0x18u) & 0x1FFu) << 13) |
        (((uint32_t)rd16(m, obj + 0x20u) & 0x0FFu) << 5)  |
        (((uint32_t)rd16(m, obj + 0x28u) & 0x001u) << 4)  |
        (((uint32_t)rd8(m, obj + 0x3Fu)  & 0x003u) << 2)  |
        (d0_low2 & 3u);

    uint16_t idx = rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x4Cu + (uint32_t)idx * 4u, d0);
    wr16(m, obj + 0x3Au, (uint16_t)((idx + 1u) & 3u));
}

/* ROM 0x0CDE50: linked-segment missile/beam initialization and first tick. */
int Obj_CDE50_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);                       /* 0x0CDE50 */

    uint32_t d0 = rd32(m, obj + 0x0Cu);
    if (d0 != 0u)
        wr32(m, d0 + 0x48u, obj);                  /* 0x0CDE60 */
    wr32(m, obj + 0x44u, d0);                      /* 0x0CDE64 */
    wr32(m, obj + 0x48u, 0u);                      /* 0x0CDE68 */

    uint32_t a1 = rd32(m, obj + 0x08u);
    uint16_t seg_idx = rd16(m, a1 + 0x22u);
    wr16(m, a1 + 0x22u, (uint16_t)(seg_idx + 1u));
    wr16(m, obj + 0x22u, seg_idx);

    uint32_t d0_trail;
    if (seg_idx == 0u) {
        wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x10u));
        d0_trail = 0u;
    } else {
        d0_trail = unpack_trail(m, obj);           /* 0x0CDEA2 */
    }

    for (int i = 0; i < 4; i++)
        pack_trail(m, obj, d0_trail);              /* 0x0CDEAA */

    wr32(m, obj + 0x28u, 0x00010000u);             /* 0x0CDEB2 */
    wr16(m, obj + 0x2Cu, (uint16_t)-4);            /* 0x0CDEBA */
    wr16(m, obj + 0x30u, (uint16_t)-4);            /* 0x0CDEC0 */
    wr16(m, obj + 0x2Eu, 3u);                      /* 0x0CDEC6 */
    wr16(m, obj + 0x32u, 3u);                      /* 0x0CDECC */
    wr16(m, obj + 0x42u, 0x0015u);                 /* 0x0CDED2 */
    wr8(m, obj + 0x3Eu, 0u);                       /* 0x0CDED8 */
    wr8(m, obj + 0x3Fu, 0u);                       /* 0x0CDEDC */
    wr8(m, obj + 0x1Du, (uint8_t)(seg_idx * 2u + 0xB0u)); /* 0x0CDEEA */
    wr32(m, obj + 0x0Cu, rd32(m, F3_A5 + 0x1D28u)); /* 0x0CDEEE */

    return Obj_CDEF4_Tick(m, obj);                 /* falls through to 0x0CDEF4 */
}
