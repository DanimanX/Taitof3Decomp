#include "obj_11AAF0.h"

/* ROM 0x11aae8: jump-table entry — set fn pointer to OBJ_11AAF0 */
static void sub_11aae8(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011AAF0u);
}

/* ROM 0x11ab0e: loop body — transform 233 word pairs from two ROM tables */
static void sub_11ab0e(const f3mem_t *m, uint32_t a1, uint32_t a2)
{
    int i;
    for (i = 0; i < 0xE7; i++) {
        uint16_t d1 = rd16(m, a1);
        uint16_t d2 = rd16(m, a2);
        wr16(m, a2++, (uint16_t)(d2 | 0xF000u));
        wr16(m, a1++, (uint16_t)(d1 & ~86));
    }
}

/* ROM 0x11ab36: set fn pointer to OBJ_11AB3E */
static void sub_11ab36(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011AB3Eu);
}

/* ROM 0x11AAF0: decrement timer; when <= 0 copy/transform two ROM tables,
 * update global flag and clear/reset object fields, advance to next state. */
int Obj_11AAF0_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));

    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        sub_11aae8(m, obj);
        return 0;
    }

    uint32_t a1 = F3_ROM_BASE + 0x626000u
                + (uint16_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a2 = F3_ROM_BASE + 0x627400u
                + (uint16_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    sub_11ab0e(m, a1, a2);

    wr16(m, F3_A5 - 0xCA4u,
         (uint16_t)(rd16(m, F3_A5 - 0xCA4u) | 0x12));

    wr32(m, obj + 0x14u, 0u);
    wr16(m, obj + 0x06u, 0x50u);

    sub_11ab36(m, obj);
    return 0;
}
