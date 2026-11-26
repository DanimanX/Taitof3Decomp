/* ROM 0x0E1072: linked-list spawner init (A0 = object).
 * Seeds from 0x0DC84E (0x0CEC4C), sets +0x23.b = 7, +0x54.l = -1,
 * +0x06.b = 1, and arms tick 0x0E1098. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E1072(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_SeedFromParams(m, obj, 0x000DC84Eu);                 /* 0x0E1072..0x0E1076 */
    uint16_t d0w = (uint16_t)rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0w = (uint16_t)(d0w + 0x20u);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
    r->a[1] = 0x000DC84Eu + 0x34u;

    wr8(m, obj + 0x23u, 7u);                                 /* 0x0E107C */
    wr32(m, obj + 0x54u, 0xFFFFFFFFu);                       /* 0x0E1082 */
    wr8(m, obj + 0x06u, 1u);                                 /* 0x0E108A */
    wr32(m, obj, 0x000E1098u);                               /* 0x0E1090 */
    return 0;
}
