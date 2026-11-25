#include "obj_FCBC4.h"
#include "obj_FB526.h"

/* ROM 0x0FCBC4 / 0x0FCBCE / 0x0FCBD8: `move.w #n,(0x10,A0) ; bra.w 0x0FB52A`,
 * the variants 1..3 of the 0x0FB526 init (which uses n = 0). */
int Obj_FCBC4_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x10u, 0x0001u);
    return Obj_FB52A_Body(m, obj);
}

int Obj_FCBCE_Tick(const f3mem_t *m, uint32_t obj) /* ROM 0x0FCBCE */
{
    wr16(m, obj + 0x10u, 0x0002u);
    return Obj_FB52A_Body(m, obj);
}

int Obj_FCBD8_Tick(const f3mem_t *m, uint32_t obj) /* ROM 0x0FCBD8 */
{
    wr16(m, obj + 0x10u, 0x0003u);
    return Obj_FB52A_Body(m, obj);
}
