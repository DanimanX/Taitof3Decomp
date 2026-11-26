/* ROM 0x19F0B2: boss sub-object/effect initializer (A0 = object).
 * Clears common fields via 0x17A7FA, sets +0x1C = 0x16E, +0x20 = 0, +0x42 =
 * 0x200, initializes 0x179A5C animation from 0x19F1F2, installs script
 * 0x19F266 (+0x44/+0x48/+0x4A=0), copies parent (+0x08)->+0x50 to +0x50,
 * sets +0x54 = 0xD1, and arms 0x19F1A2. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_19F0B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                                  /* 0x19F0B2 */
    wr16(m, obj + 0x1Cu, 0x016Eu);                       /* 0x19F0B8 */
    wr16(m, obj + 0x20u, 0x0000u);                       /* 0x19F0BE */
    wr16(m, obj + 0x42u, 0x0200u);                       /* 0x19F0C4 */

    uint32_t a1 = 0x0019F1F2u;                           /* 0x19F0CA -> 0x19F15A */
    uint16_t anim_idx = rd16(m, a1); a1 += 2u;           /* 0x19F160 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | anim_idx;
    wr16(m, obj + 0x40u, anim_idx);                      /* 0x19F162 */
    wr32(m, obj + 0x3Au, a1);                            /* 0x19F166 */
    if (Obj_AnimInit(m, obj, anim_idx) < 0)              /* 0x19F16A..0x19F174 */
        return -1;

    a1 = 0x0019F266u;                                    /* 0x19F176 */
    r->a[1] = a1;
    wr32(m, obj + 0x44u, a1);                            /* 0x19F17C */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));              /* 0x19F180 */
    wr16(m, obj + 0x4Au, 0u);                            /* 0x19F186 */

    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19F18A */
    r->a[4] = a4;
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));           /* 0x19F18E */
    wr16(m, obj + 0x54u, 0x00D1u);                       /* 0x19F194 */
    wr32(m, obj + 0x00u, 0x0019F1A2u);                   /* 0x19F19A */
    return 0;
}
