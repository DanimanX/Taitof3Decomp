#include "obj_18B4B8.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x18B4B8: init boss part from 0x18B5B8 / 0x18B600 and arm 0x18B4E8 */
int Obj_18B4B8_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_1848D6(m, obj, 0x0018B5B8u);
    uint32_t a1 = 0x0018B600u;
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;
    wr32(m, obj + 0x00u, 0x0018B4E8u);
    return 0;
}
