#include "obj_1244DA.h"

/* ROM 0x1244D2: per-tick loop entry: stay on 0x1244DA */
static void sub_1244D2(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x001244DAu);
}

/* ROM 0x12448E: after the +0x6 loop: arm (0x30EE){0:1}, +0x3E = 0x1E,
 * chain to 0x1244A2 */
static void sub_12448E(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u)); /* bfset {0:1} */
    wr16(m, obj + 0x3Eu, 0x001Eu);
    wr32(m, obj + 0x00u, 0x001244A2u);                               /* 0x12449A */
}

/* ROM 0x12452A: toggle finished and +0x6 == 0: next phase 0x124532 */
static void sub_12452A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00124532u);
}

/* ROM 0x1244DA: pull the global +0x30D2 towards 0x12C0000 by (D0 >> 4),
 * then move the global +0x30D6 by +-0x16000 while the +0x3E timer runs */
int Obj_1244DA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, F3_A5 + 0x30D2u) - 0x012C0000u;
    d0 = (uint32_t)((int32_t)d0 >> 4);                               /* asr.l #4 */
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - d0);

    d0 = 0x00016000u;
    if (rd8(m, obj + 0x3Au) != 0u)
        d0 = (uint32_t)(-(int32_t)d0);                               /* 0x1244F8 neg.l */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + d0);

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x3Eu) - 1u);
    wr16(m, obj + 0x3Eu, (uint16_t)n);
    if (n > 0) {
        sub_1244D2(m, obj);
        return 0;
    }
    n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_12448E(m, obj);
        return 0;
    }
    /* 0x12450A */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & 0x3Fu)); /* bfclr {0e:1} */
    wr8(m, obj + 0x3Au, (uint8_t)~rd8(m, obj + 0x3Au));              /* not.b */
    wr16(m, obj + 0x06u, 0x001Eu);
    /* 0x12451A beq.b -> the "not.b" result is always != 0, so 0x12452A
     * (0x12451C) is not reachable from here */
    sub_12452A(m, obj);
    return 0;
}
