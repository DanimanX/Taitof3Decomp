/* ROM 0x134844: reset the per-frame fields, set the two speed words
 * +0x24 = +0x28 = 1, set flags bits 1 and 3 of the byte +0x34 and hand the
 * object to the next state 0x13486A. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_134844(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);            /* ROM 0x134844: jsr 0x000CBD86 */
    wr16(m, obj + 0x28u, 0x0001u);      /* ROM 0x13484A: move.w #1,(0x28,A0) */
    wr16(m, obj + 0x24u, 0x0001u);      /* ROM 0x134850: move.w #1,(0x24,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));  /* ROM 0x134856: bset.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));  /* ROM 0x13485C: bset.b #3 */
    wr32(m, obj, 0x0013486Au);          /* ROM 0x134862: move.l #0x13486a,(A0) */
    return 0;                           /* ROM 0x134868 rts */
}
