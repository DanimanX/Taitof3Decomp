/* ROM 0x0E1134: linked-list child init (A0 = object).
 * Seeds from 0x0DC84E (0x0CEDE4), installs behaviour 0x0E11A0 (0x0D0678),
 * copies grandparent (+0x08->+0x08) position (+0x3A/+0x3E) with (+0x0B, +0x26)
 * offset into +0x14/+0x18, and arms tick 0x0E1170. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E1134(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000DC84Eu);                         /* 0x0E1134..0x0E1138 */
    Obj_StartBehaviour(m, obj, 0x000E11A0u);                 /* 0x0E113E..0x0E1142 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x000E11A2u);

    uint32_t a1 = rd32(m, obj + 0x08u);                      /* 0x0E1148 */
    a1 = rd32(m, a1 + 0x08u);                                /* 0x0E114C */
    r->a[1] = a1;

    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));               /* 0x0E1150 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));               /* 0x0E1156 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x000Bu)); /* 0x0E115C */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0026u)); /* 0x0E1162 */
    wr32(m, obj, 0x000E1170u);                               /* 0x0E1168 */
    return 0;
}
