#include "obj_101FE6.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x101FE6: init via Obj_0CEDE4 with table 0x1086FC, arm 0x101FF8 */
int Obj_101FE6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x001086FCu);
    wr32(m, obj + 0x00u, 0x00101FF8u);
    return 0;
}
