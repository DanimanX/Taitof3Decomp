/* ROM 0x187246: the idle half of the 0x187370 spawner family -- park the object
 * on its own (0x18724E) behaviour. Reached from 0x18724E when the freeze bit
 * (A5+0x4B3A bit 13) is set, from every spawn line at 0x187348, and from
 * 0x1873D6 once the frame countdown runs out. */
#include "obj_187246.h"

int Obj_187246_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0018724Eu);                  /* move.l #0x18724E,(A0) */
    return 0;                                   /* rts */
}
