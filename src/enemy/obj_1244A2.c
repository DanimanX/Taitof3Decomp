#include "obj_1244A2.h"

/* ROM 0x1244A2: slide the global +0x30D2 down by 0x30000 per tick and count the
 * +0x3E timer down. When the timer hits zero compare the +0x30DA word: at or
 * below 0xC8 raise {2:1},{1:1},{14:1}, re-arm +0x3E = 0x24 and hand over to
 * 0x1244DA; above 0xC8 branch to the shared 0x12456A phase start, which is not
 * decompiled yet (its jmp $124B3A leaves this routine). */
int Obj_1244A2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1244A2 */
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - 0x00030000u);

    int16_t t = (int16_t)(uint16_t)(rd16(m, obj + 0x3Eu) - 1u);
    wr16(m, obj + 0x3Eu, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x001244A2u); /* stay here */
        return 0;
    }

    if ((int16_t)(uint16_t)rd16(m, F3_A5 + 0x30DAu) > 0x00C8) {
        /* 0x12456A: +0x30DA is still above 0xC8. Arm the 0x12457E countdown,
         * stop the +0x30D2 drift and jmp into the shared 0x124B3A phase
         * start (0x124578), which this routine does not cover */
        wr16(m, obj + 0x06u, 0x0050u);
        wr32(m, obj + 0x0Cu, 0x0012457Eu);
        return -1;
    }

    /* 0x1244BA: three bit-field sets on the shared flag word */
    wr16(m, F3_A5 + 0x30EEu,
         (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x0022u));  /* {2:1},{1:1} */
    wr16(m, F3_A5 + 0x30EEu,
         (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x4000u));  /* {14:1} */
    wr16(m, obj + 0x3Eu, 0x0024u);
    wr32(m, obj + 0x00u, 0x001244DAu);
    return 0;
}
