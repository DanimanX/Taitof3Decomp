#include "obj_CEF44.h"

/* ROM 0x0CEF44: init - set tick function to 0x0CEF4C */
int Obj_CEF44_Tick(const f3mem_t *m, uint32_t obj)
{
    (void)m;
    wr32(m, obj + 0x00u, 0x000CEF4Cu);
    return 0;
}
