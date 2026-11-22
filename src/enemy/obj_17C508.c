#include "obj_17C508.h"

/* ROM 0x17C508: bra.b 0x17C500 -- one-shot state 0x17C500 whose body is
 * move.l #0x17C508,(A0); rts, so the tick is a bare tail call (the next tick
 * re-reads A0) and changes nothing. */
int Obj_17C508_Tick(const f3mem_t *m, uint32_t obj)
{
    (void)m;
    (void)obj;
    return 0;
}
