/* ROM 0x1841D8: boss phase-list step (A0 = object, A2 = phase record list).
 * Decrements the +0x06 timer and re-arms 0x1841D8 while it stays > 0. When it
 * expires, three longwords are taken from the record list at A2: (A2)+ is the
 * stat record for 0x184888, (A2)+ is the animation list whose leading word
 * becomes the 0x179A5C index (+0x40, body at +0x3A) and whose init half runs,
 * and (A2)+ becomes the phase script pointer (+0x44, its +0x04 word into
 * +0x48, +0x4A cleared). The next state is 0x17BE1C.
 *
 * Note: the 0x179A5C init half is the shared Obj_AnimInit; only index 0 (the
 * one the capture reaches) leaves D0 alone, which is what this routine keeps
 * across the call, as 0x17FE9A does. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

#define ANIM_TABLE 0x00179A5Cu

int Fn_1841D8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x1841D8 */
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {                                 /* 0x1841DC bgt */
        wr32(m, obj, 0x001841D8u);                            /* 0x1841D0 re-arm */
        return 0;
    }

    uint32_t a2 = r->a[2];
    r->a[1] = rd32(m, a2);                                    /* 0x1841DE movea.l (A2)+,A1 */
    a2 += 4u;
    Obj_184888(m, obj, r->a[1]);                              /* 0x1841E0 jsr 0x184888 */
    r->d[0] = rd32(m, obj + 0x24u);                           /* D0 after 0x184888 divul.l */

    r->a[1] = rd32(m, a2);                                    /* 0x1841E6 movea.l (A2)+,A1 */
    a2 += 4u;
    uint16_t anim_idx = rd16(m, r->a[1]);                     /* 0x1841E8 move.w (A1)+,D0w */
    r->a[1] += 2u;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | anim_idx;
    wr16(m, obj + 0x40u, anim_idx);                           /* 0x1841EA */
    wr32(m, obj + 0x3Au, r->a[1]);                            /* 0x1841EE */

    r->a[1] = rd32(m, ANIM_TABLE + (uint32_t)anim_idx * 8u);   /* 0x1841F2..0x1841F8 */
    if (Obj_AnimInit(m, obj, anim_idx) < 0)                   /* 0x1841FC jsr (A1) */
        return -1;

    r->a[1] = rd32(m, a2);                                    /* 0x1841FE movea.l (A2)+,A1 */
    a2 += 4u;
    wr32(m, obj + 0x44u, r->a[1]);                            /* 0x184200 */
    wr16(m, obj + 0x48u, rd16(m, r->a[1] + 0x04u));           /* 0x184204 */
    wr16(m, obj + 0x4Au, 0u);                                 /* 0x18420A clr.w */
    wr32(m, obj, 0x0017BE1Cu);                                /* 0x18420E -> 0x17BE14 */

    r->a[2] = a2;
    return 0;
}
