#include "obj_12D2FC.h"

/* bfclr (0x30ee,A5){n:1} */
static void bfclr_30ee(const f3mem_t *m, unsigned n)
{
    uint32_t a = F3_A5 + 0x30EEu + n / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(uint8_t)(0x80u >> (n % 8u))));
}

/* ROM 0x12D2F4: hold in this state */
static void sub_12D2F4(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D2FCu);
}

/* ROM 0x12D2FC: hold, then clear the boss flag and chain */
int Obj_12D2FC_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12D300: bgt 0x12D2F4 */
        sub_12D2F4(m, obj);
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
