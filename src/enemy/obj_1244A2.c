#include "obj_1244A2.h"
#include "obj_E8B72.h"  /* Sound_PlayHooked */

/* ROM 0x124B3A: shared phase start -- sound 0x14, A5+0x30EE.l = {15:1} only,
 * A5+0x30F2 = 0, A5+0x30F3 = 0xFF, next tick 0x124B68. */
int Obj_124B3A(const f3mem_t *m, uint32_t obj)
{
    Sound_PlayHooked(m, 0x0014u);
    wr32(m, F3_A5 + 0x30EEu, 0u);                       /* clr.l */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x01u)); /* bfset {15:1} */
    wr8(m, F3_A5 + 0x30F2u, 0x00u);                     /* sf */
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);                     /* st */
    wr32(m, obj + 0x00u, 0x00124B68u);
    return 0;
}

/* ROM 0x1244A2: slide the global +0x30D2 down by 0x30000 per tick and count the
 * +0x3E timer down. When the timer hits zero compare the +0x30DA word: at or
 * above 0xC8 raise {2:1},{1:1},{14:1}, re-arm +0x3E = 0x24 and hand over to
 * 0x1244DA; at or below 0xC8 arm the 0x12457E follow-up and jump to the shared
 * 0x124B3A phase start. */
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

    if ((int16_t)(uint16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x00C8) {
        /* 0x1244B6 ble 0x12456A: +0x30DA is at or below 0xC8. Arm the
         * 0x12457E follow-up in +0x0C and jmp to the shared 0x124B3A phase
         * start (not reached by the capture) */
        wr16(m, obj + 0x06u, 0x0050u);
        wr32(m, obj + 0x0Cu, 0x0012457Eu);
        return Obj_124B3A(m, obj);              /* 0x124578: jmp (PC) */
    }

    /* 0x1244BA: three bit-field sets on the shared flag word */
    /* memory bit fields count from the MSB of the byte at A5+0x30EE:
     * {2:1} = 0x2000, {1:1} = 0x4000, {14:1} = 0x0002 of the word */
    wr16(m, F3_A5 + 0x30EEu,
         (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x2000u));  /* {2:1} */
    wr16(m, F3_A5 + 0x30EEu,
         (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x4000u));  /* {1:1} */
    wr16(m, F3_A5 + 0x30EEu,
         (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x0002u));  /* {14:1} */
    wr16(m, obj + 0x3Eu, 0x0024u);
    wr32(m, obj + 0x00u, 0x001244DAu);
    return 0;
}
