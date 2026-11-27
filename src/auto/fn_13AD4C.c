/* ROM 0x13AD4C: object initialiser (A0 = object). Clears +0x1C/+0x1E/+0x34/
 * +0x3A/+0x3E and +0x24/+0x28 via Obj_ResetFields (0x0CBD86), sets the flip
 * byte +0x4F = 0xDE (-0x22), copies the spawn words (+0x0C, +0x0E) to
 * +0x50/+0x52, sets contact damage +0x28.w = 1 and the hitbox +0x2C..+0x32 =
 * {-6, -6, 6, 6}, then re-arms the object with 0x13AD8A. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_13AD4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);                         /* 0x13AD4C -> 0x0CBD86 */
    wr8(m, obj + 0x4Fu, 0xDEu);                      /* 0x13AD52 move.b #-0x22 */
    wr16(m, obj + 0x50u, rd16(m, obj + 0x0Cu));      /* 0x13AD58 */
    wr16(m, obj + 0x52u, rd16(m, obj + 0x0Eu));      /* 0x13AD5E */
    wr16(m, obj + 0x28u, 0x0001u);                   /* 0x13AD64 */
    wr16(m, obj + 0x2Cu, 0xFFFAu);                   /* 0x13AD6A move.w #-6 */
    wr16(m, obj + 0x30u, 0xFFFAu);                   /* 0x13AD70 move.w #-6 */
    wr16(m, obj + 0x2Eu, 0x0006u);                   /* 0x13AD76 */
    wr16(m, obj + 0x32u, 0x0006u);                   /* 0x13AD7C */
    wr32(m, obj + 0x00u, 0x0013AD8Au);               /* 0x13AD82 */
    return 0;
}
