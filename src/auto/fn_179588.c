/* ROM 0x179588: register-level wrapper of Obj_179588_Tick (src/enemy/obj_179588.c) for its capture.
 *
 * Entry: A0 = obj. Body:
 *   0x179588  movea.l $3a(a0),a1
 *   0x17958C  move.w  (a1)+,$3e(a0)
 *   0x179590  move.l  (a1)+,$10(a0)
 *   0x179594  move.w  (a1)+,d0
 *   0x179596  move.b  d0,$1c(a0)
 *   0x17959A  move.l  (a1)+,$2c(a0)
 *   0x17959E  move.l  (a1)+,$30(a0)
 *   0x1795A2  rts
 * so at the rts A1 is 0x10 past the record start (2+4+2+4+4) and D0 holds the
 * record's +6 word with its upper half cleared (move.w into D0w). A0 and every
 * other register are untouched. */
#include "fn_auto.h"
#include "../enemy/obj_179588.h"

int Fn_179588(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);      /* 0x179588 movea.l $3a(a0),a1 */

    if (Obj_179588_Tick(m, obj) < 0)
        return -1;

    r->d[0] = rd16(m, rec + 6u);              /* 0x179594 move.w (a1)+,d0 */
    r->a[1] = rec + 0x10u;                    /* 0x1795A2 rts */
    return 0;
}
