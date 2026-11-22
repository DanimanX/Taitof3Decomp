#include "obj_124532.h"

/* bfset (0x30ee,A5){0x5:0x1} — extract 1 bit at pos 5 from word, use as bit index */
static void bfset_bit5(const f3mem_t *m)
{
    uint16_t w = rd16(m, F3_A5 + 0x30EEu);
    unsigned bp = (w >> 5) & 1;
    wr16(m, F3_A5 + 0x30EEu, (uint16_t)(w | (1u << bp)));
}

/* ROM 0x12452a: jump-table entry — set fn pointer to OBJ_124532 */
static void sub_12452a(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00124532u);
}

/* ROM 0x124548: set fn pointer to OBJ_124550 */
static void sub_124548(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00124550u);
}

/* ROM 0x124532: subtract 0x60000 from A5+0x30D2 counter; loop until <= 0x28,
 * then set flag bit and chain to next state. */
int Obj_124532_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x30D2u,
         (uint32_t)((int32_t)rd32(m, F3_A5 + 0x30D2u) - 0x60000));

    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x28) {
        sub_12452a(m, obj);
        return 0;
    }

    bfset_bit5(m);
    sub_124548(m, obj);
    return 0;
}
