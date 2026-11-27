/* ROM 0x13A0C0: reset the per-frame fields, clear the two words +0x54/+0x56,
 * set +0x30 = 8, +0x32 = 0x47, +0x28 = 1, the frame byte +0x1C = 1 and hand
 * the object to the next state 0x13A0EE. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_13A0C0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);            /* ROM 0x13A0C0: jsr 0x000CBD86 */
    wr16(m, obj + 0x54u, 0u);           /* ROM 0x13A0C6: clr.w (0x54,A0) */
    wr16(m, obj + 0x56u, 0u);           /* ROM 0x13A0CA: clr.w (0x56,A0) */
    wr16(m, obj + 0x30u, 0x0008u);      /* ROM 0x13A0CE: move.w #8,(0x30,A0) */
    wr16(m, obj + 0x32u, 0x0047u);      /* ROM 0x13A0D4: move.w #0x47,(0x32,A0) */
    wr16(m, obj + 0x28u, 0x0001u);      /* ROM 0x13A0DA: move.w #1,(0x28,A0) */
    wr8(m, obj + 0x1Cu, 0x01u);         /* ROM 0x13A0E0: move.b #1,(0x1C,A0) */
    wr32(m, obj, 0x0013A0EEu);          /* ROM 0x13A0E6: move.l #0x13a0ee,(A0) */
    return 0;                           /* ROM 0x13A0EC rts */
}
