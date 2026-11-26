/* ROM 0x0F1E84: burst spawner tick (A0 = object).
 * Dies (0x0CEE74) if bomb flag (A5-0x144B bit 5) is set or retries (+0x22) == 0.
 * When shots left (+0x21) > 0, counts down +0x23 and every 10 frames spawns
 * 0x0F1F10 (group 5, count 2) at (0x140, 0x74 + (Obj_Next4Way - 0x80)); copies
 * +0x4E on the final shot. On spawn failure, decrements +0x22. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

int Fn_0F1E84(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u ||            /* 0x0F1E84 */
        rd8(m, obj + 0x22u) == 0u) {                          /* 0x0F1E90 */
        Obj_StartDying(m, obj);                               /* 0x0CEE74 */
        return 0;
    }

    if (rd8(m, obj + 0x21u) == 0u)                            /* 0x0F1E9A */
        return 0;

    {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);      /* 0x0F1EA2 */
        wr8(m, obj + 0x23u, t);
        if (t != 0u)
            return 0;
    }

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u)); /* 0x0F1EAA */
    wr8(m, obj + 0x23u, 0x0Au);                               /* 0x0F1EAE */

    {
        uint32_t child = Obj_Spawn(m, obj, 0x000F1F10u, 5u, 2u, 0u); /* 0x0F1EB4..0x0F1EC6 */
        r->d[0] = child;
        if ((int32_t)child < 0) {                             /* 0x0F1ED0 */
            wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u)); /* 0x0F1F0A */
            return 0;
        }
        r->a[1] = child;                                      /* 0x0F1ED6 */
        wr32(m, child + 0x14u, 0x01400000u);                  /* 0x0F1ED8 */
        wr32(m, child + 0x18u, 0x00740000u);                  /* 0x0F1EE0 */
        {
            uint16_t rval = (uint16_t)(Obj_Next4Way(m) - 0x0080u); /* 0x0F1EE8..0x0F1EEE */
            r->d[0] = (r->d[0] & 0xFFFF0000u) | rval;
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + rval)); /* 0x0F1EF2 */
        }
        wr8(m, child + 0x4Eu, 0u);                            /* 0x0F1EF6 */
        if (rd8(m, obj + 0x21u) == 0u)                        /* 0x0F1EFA */
            wr8(m, child + 0x4Eu, rd8(m, obj + 0x4Eu));       /* 0x0F1F02 */
    }

    return 0;
}
