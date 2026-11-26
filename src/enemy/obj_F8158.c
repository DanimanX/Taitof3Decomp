#include "obj_F8158.h"
#include "obj_E8B72.h"

/* ROM 0x0F8158: enemy init -- mover record 0x0EC8EE, class colour, script
 * 0x0F817E, then the shared 0x0EC71A tail: behaviour record 0x0F81B4,
 * +0x34 bit 1 off / bit 2 on, arm 0x0EC734. */
int Obj_F8158_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000EC8EEu);             /* 0x0CEC4C */
    Obj_InitPalette(m, obj);                        /* 0x0CEE2C */
    Obj_SetScript(m, obj, 0x000F817Eu);             /* lea (0x12,PC) ; 0x0CF1D8 */
    Obj_SetAnim(m, obj, 0x000F81B4u);               /* lea (0x3E,PC) ; 0x0D0678 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000EC734u);
    return 0;
}
