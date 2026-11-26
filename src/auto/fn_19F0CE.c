/* ROM 0x19F0CE: boss spark/debris initializer (A0 = object).
 * Clears fields via 0x17A7FA, sets +0x1C = 0x6E, picks random +0x20.b in
 * 0x40 +/- 0x0F and +0x42.w in 0x180 +/- 0x4F via 0x1794A4, installs anim list
 * 0x19F226 (index 0 -> 0x179550), script 0x19F266 (+0x44/+0x48/+0x4A), copies
 * parent (+0x08) +0x50/+0x54 jittered by 0x1794A4(0x30) / -0x14+0x1794A4(0x30),
 * and arms 0x19F1A2. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_19F0CE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Obj_17A7FA(m, obj);                                  /* 0x19F0CE */
    wr16(m, obj + 0x1Cu, 0x006Eu);                       /* 0x19F0D4 */

    uint16_t d0w = Obj_RandRange(m, 0x0010u);            /* 0x19F0DA..0x19F0DC */
    d0w = (d0w & 0xFF00u) | (uint8_t)(d0w + 0x40u);      /* 0x19F0E2 */
    wr8(m, obj + 0x20u, (uint8_t)d0w);                   /* 0x19F0E6 */

    d0w = (uint16_t)(Obj_RandRange(m, 0x0050u) + 0x0180u); /* 0x19F0EA..0x19F0F2 */
    wr16(m, obj + 0x42u, d0w);                           /* 0x19F0F6 */

    uint32_t a1 = 0x0019F226u;                           /* 0x19F0FA */
    uint16_t anim_idx = rd16(m, a1); a1 += 2u;           /* 0x19F100 */
    wr16(m, obj + 0x40u, anim_idx);                      /* 0x19F102 */
    wr32(m, obj + 0x3Au, a1);                            /* 0x19F106 */
    Obj_AnimInit(m, obj, anim_idx);                      /* 0x19F10A..0x19F114 */

    a1 = 0x0019F266u;                                    /* 0x19F116 */
    wr32(m, obj + 0x44u, a1);                            /* 0x19F11C */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));              /* 0x19F120 */
    wr16(m, obj + 0x4Au, 0u);                            /* 0x19F126 */

    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19F12A */
    r->a[4] = a4;
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));           /* 0x19F12E */
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));           /* 0x19F134 */

    d0w = Obj_RandRange(m, 0x0030u);                     /* 0x19F13A..0x19F13C */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + d0w)); /* 0x19F142 */

    d0w = (uint16_t)(Obj_RandRange(m, 0x0030u) - 0x0014u); /* 0x19F146..0x19F14E */
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + d0w)); /* 0x19F152 */

    wr32(m, obj, 0x0019F1A2u);                           /* 0x19F19A */
    r->d[0] = (uint32_t)d0w;
    r->a[1] = a1;
    return 0;
}
