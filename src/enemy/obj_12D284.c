#include "obj_12D284.h"

/* bfset/bfclr (0x30ee,A5){n:1} (offset from MSB) */
static void bfset_30ee(const f3mem_t *m, unsigned n)
{
    uint32_t a = F3_A5 + 0x30EEu + n / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (n % 8u))));
}

/* ROM 0x12D27C: hold in this state */
static void sub_12D27C(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D284u);
}

/* ROM 0x12D284: count down, flag the boss, slide the view back and chain */
int Obj_12D284_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x32A6u, (uint16_t)(rd16(m, F3_A5 + 0x32A6u) - 1u));
    if ((int16_t)rd16(m, F3_A5 + 0x32A6u) > 0) {
        /* 0x12D288: bgt 0x12D27C */
        sub_12D27C(m, obj);
        return 0;
    }

    /* 0x12D28A */
    wr16(m, obj + 0x3Eu, 3u);
    bfset_30ee(m, 0);
    wr16(m, F3_A5 + 0x32A6u, 0u);
    wr16(m, F3_A5 + 0x32A8u, 3u);
    wr16(m, F3_A5 + 0x32A4u, 0xF4C0u);   /* -0xB40 */

    /* 0x12D2A6: beq.b 0x12D2AC never taken with 0x3C */
    wr16(m, obj + 0x06u, 0x003Cu);

    /* 0x12D2AC */
    wr32(m, obj + 0x00u, 0x0012D2B4u);
    return 0;
}
