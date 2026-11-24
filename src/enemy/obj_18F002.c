#include "obj_18F002.h"

/* ROM 0x18F002 (boss 0: arm the intro / stats, then run 0x18F012)
 *
 * Bytes re-decoded from work/maincpu.bin:
 *   18F002 317c 000a 0006        move.w #0xa,(0x6,A0)   (Z = 0)
 *   18F008 6700 000e             beq.b 0x18F018          (never taken)
 *   18F00C 20bc 0018 f012        move.l #0x18F012,(A0)
 *   18F012 4e75                 rts
 *
 * The DEAD 0x18F018 fall-through is the shared boss intro (0x18F05C..0x18F1C0,
 * identical to 0x18F012's body, see obj_18F012.c). Every captured tick here is
 * the plain 10-frame arm plus the tick-vector write. */

/* ROM 0x18F002 */
int Obj_18F002_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18F002: move.w #0xa,(0x6,A0) ; 0x18F008 beq.b (never taken) */
    wr16(m, obj + 0x06u, 0x000Au);

    /* 0x18F00C: move.l #0x18F012,(A0) */
    wr32(m, obj + 0x00u, 0x0018F012u);
    return 0;
}
