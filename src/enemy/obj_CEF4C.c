#include "obj_CEF4C.h"
/* ROM 0x0CEF4C: the #imm32 is written as one longword; the 16-bit signature
 * emits 0x00CEF54, so the next tick runs those bytes as code (0xCEF56 BRA.S
 * lands on the 0x0CEF54 body). */
int Obj_CEF4C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x000CEF54u);  /* 0x0CEF4C move.l #0xcef54,(A0) */
    return 0;
}
