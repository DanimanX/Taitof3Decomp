#include "obj_E78A6.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "obj_E8B72.h"

/* ROM 0x0E78A6: init of the 0x0E78EC column. Stats 0x0E79FA, first frame
 * 0x0E7A2A, the parent's side flag, a 0x1C-frame animation timer, phase 3,
 * and sound 0x2A. */
int Obj_E78A6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x000E79FAu);
    Obj_SetFrame(m, obj, 0x000E7A2Au);              /* 0x0CFA86 */
    wr8(m, obj + 0x22u, rd8(m, rd32(m, obj + 0x08u) + 0x22u));
    wr8(m, obj + 0x23u, 0x1Cu);
    wr8(m, obj + 0x06u, 0x03u);
    Sound_PlayHooked(m, 0x002Au);                   /* 0x13F238 + 0x0039D8 */
    wr32(m, obj + 0x00u, 0x000E78ECu);
    return 0;
}
