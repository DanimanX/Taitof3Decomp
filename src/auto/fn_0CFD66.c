/* ROM 0x0CFD66: register-level wrapper of sub_0CFD66 (src/enemy/obj_105EDE.c)
 * for its capture.
 *   D0.w = table 0x0CFD80[D0.w]  (indexed with the sign-extended word, *2)
 *   A1  += (int16)D0.w           then `(a1)+`, `(a1)+`
 *   D0.w = the frame word at that A1  (upper half survives)
 * The frame word is read before the call: the helper writes through A0, which
 * can point into the same record. */
#include "fn_auto.h"
#include "fn_helpers.h"

int Fn_0CFD66(const f3mem_t *m, f3regs_t *r)
{
    uint16_t idx = (uint16_t)r->d[0];
    int16_t off = (int16_t)rd16(m, 0x000CFD80u + (uint32_t)(int32_t)(int16_t)idx * 2u);
    uint32_t a1 = r->a[1] + (uint32_t)(int32_t)off;   /* 0x0CFD6E adda.w d0,a1 */
    uint16_t frame = rd16(m, a1);                     /* 0x0CFD70 move.w (a1)+,d0 */

    sub_0CFD66(m, r->a[0], r->a[1], idx);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | frame;
    r->a[1] = a1 + 6u;                                /* (a1)+, (a1)+ */
    return 0;                                         /* 0x0CFD7E rts */
}
