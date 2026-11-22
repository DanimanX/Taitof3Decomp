#include "obj_125DAA.h"

/* ROM 0x125d38: clear field +0x22 on caller, then chain to OBJ_125D44 */
static void sub_125d38(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x00u, 0x00125D44u);
}

/* ROM 0x125da2: self-loop — set fn pointer to OBJ_125DAA */
static void sub_125da2(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00125DAEu);
}

/* ROM 0x125DAA: test bit (0xf) of word at A5+0x30EE; if not set, self-loop.
 * Otherwise clear field and chain to next state. */
int Obj_125DAA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t w = rd16(m, F3_A5 + 0x30EEu);
    unsigned bp = (w >> 0xf) & 1;

    if (!(w & (1u << bp))) {
        sub_125da2(m, obj);
        return 0;
    }

    sub_125d38(m, obj);
    return 0;
}
