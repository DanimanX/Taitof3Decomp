#include "obj_12D2D4.h"

/* bfclr (0x30ee,A5){n:1} */
static void bfclr_30ee(const f3mem_t *m, unsigned n)
{
    uint32_t a = F3_A5 + 0x30EEu + n / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(uint8_t)(0x80u >> (n % 8u))));
}

/* ROM 0x12D2A6: hold in this state */
static void sub_12D2A6(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x003Cu);
    /* falls through to 0x12D2AC */
    wr32(m, obj + 0x00u, 0x0012D2B4u);
}

/* ROM 0x12D2CC: hold in this state */
static void sub_12D2CC(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D2D4u);
}

/* ROM 0x12D2D4: mirror of 0x12D2B4 sliding the background back down */
int Obj_12D2D4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12D2D4 */
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) + 0x00C0u));
    wr16(m, F3_A5 + 0x32A4u, (uint16_t)(rd16(m, F3_A5 + 0x32A4u) - 0x0060u));

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12D2E4: bgt 0x12D2CC */
        sub_12D2CC(m, obj);
        return 0;
    }

    /* 0x12D2E6: remaining repeats of the slide */
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    if ((int16_t)rd16(m, obj + 0x3Eu) > 0) {
        /* 0x12D2EA: bgt 0x12D2A6 */
        sub_12D2A6(m, obj);
        return 0;
    }

    /* 0x12D2EC: beq.b 0x12D302 never taken with 0x1E */
    wr16(m, obj + 0x06u, 0x001Eu);
    if (rd16(m, obj + 0x06u) != 0) {   /* beq.b 0x12D302 */
        /* 0x12D2F4 */
        wr32(m, obj + 0x00u, 0x0012D2FCu);
        return 0;
    }

    /* 0x12D302 */
    bfclr_30ee(m, 0);
    wr16(m, F3_A5 + 0x32A8u, 2u);
    wr16(m, F3_A5 + 0x32A4u, 0u);

    /* 0x12D312 */
    wr32(m, obj + 0x00u, 0x0012D31Au);
    return 0;
}
