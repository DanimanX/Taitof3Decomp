#include "obj_191486.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x191486: init boss sub-part with parent offset and anim, arm 0x1914E6 */
int Obj_191486_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_17A7FA(m, obj);
    Obj_InheritOffsetPos(m, obj);
    wr16(m, obj + 0x54u, 0x00BAu);
    uint32_t a1 = 0x00191546u;
    uint16_t d0 = 0x0018u;
    if (rd16(m, F3_A5 + 0x32DCu) == 0x000Fu) {
        a1 = 0x00191586u;
        d0 = 0x0020u;
    }
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + d0));
    uint16_t idx = rd16(m, a1); a1 += 2u;
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    wr16(m, obj + 0x1Cu, 0x007Au);
    wr16(m, obj + 0x42u, (uint16_t)-0x200);
    wr32(m, obj + 0x00u, 0x001914E6u);
    return 0;
}
