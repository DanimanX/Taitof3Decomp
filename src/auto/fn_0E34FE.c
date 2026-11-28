#include "fn_auto.h"
/* ROM 0x0E34FE: initialise a scripted part and hand it to 0x0E353A (A0 =
 * object). 0x0CEDE4 installs the mover from 0x0DC84E, 0x0D0678 the animation
 * from 0x0E356A (D0 leaves as that record's colour word), then the position is
 * taken from the object two levels up: A0+0x08 -> +0x08, whose +0x14/+0x18
 * longs are copied and their high words biased by 0x0E and 0x16, and the tick
 * becomes 0x0E353A. A1 ends as that parent pointer. */
#include "../enemy/obj_E8B72.h"      /* Obj_InitMoverB (0x0CEDE4), Obj_SetAnim (0x0D0678) */

int Fn_0E34FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t rec = 0x000E356Au;                          /* 0x0E3508 lea (0x60,PC) */

    Obj_InitMoverB(m, a0, 0x000DC84Eu);                  /* 0x0E34FE lea (-0x6CB2,PC) */
    Obj_SetAnim(m, a0, rec);                             /* 0x0E350C jsr 0x0D0678 */
    r->d[0] = rd16(m, rec + 2u);                         /* 0x0D0680 move.w (A1)+,D0 */

    uint32_t a1 = rd32(m, a0 + 0x08u);                   /* 0x0E3512 */
    a1 = rd32(m, a1 + 0x08u);                            /* 0x0E3516 */

    wr32(m, a0 + 0x14u, rd32(m, a1 + 0x14u));            /* 0x0E351A */
    wr16(m, a0 + 0x14u, (uint16_t)(rd16(m, a0 + 0x14u) + 0x000Eu)); /* 0x0E3520 */
    wr32(m, a0 + 0x18u, rd32(m, a1 + 0x18u));            /* 0x0E3526 */
    wr16(m, a0 + 0x18u, (uint16_t)(rd16(m, a0 + 0x18u) + 0x0016u)); /* 0x0E352C */

    wr32(m, a0, 0x000E353Au);                            /* 0x0E3532 */
    r->a[1] = a1;
    return 0;                                            /* 0x0E3538 rts */
}
