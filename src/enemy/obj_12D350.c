#include "obj_12D350.h"

/* bfset (0x30ee,A5){n:1} */
static void bfset_30ee(const f3mem_t *m, unsigned n)
{
    uint32_t a = F3_A5 + 0x30EEu + n / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (n % 8u))));
}

/* ROM 0x12D348: hold in this state */
static void sub_12D348(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D350u);
}

/* ROM 0x12D350: hold, set the boss exit flag, hand over to the long hold */
int Obj_12D350_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12D354: bgt 0x12D348 */
        sub_12D348(m, obj);
        return 0;
    }

    /* 0x12D356 */
    bfset_30ee(m, 1);
    wr16(m, obj + 0x06u, 0x00AAu);
    if (rd16(m, obj + 0x06u) != 0) {   /* beq.b 0x12D372: never taken */
        /* 0x12D364 */
        wr32(m, obj + 0x00u, 0x0012D36Cu);
        return 0;
    }

    /* 0x12D372 */
    wr16(m, obj + 0x06u, 0x003Cu);
    if (rd16(m, obj + 0x06u) != 0) {   /* beq.b 0x12D388: never taken */
        /* 0x12D37A */
        wr32(m, obj + 0x00u, 0x0012D382u);
        return 0;
    }

    /* 0x12D388 */
    wr16(m, obj + 0x06u, 0x000Fu);
    wr16(m, F3_A5 + 0x32A8u, 1u);

    /* 0x12D394 */
    wr32(m, obj + 0x00u, 0x0012D39Cu);
    return 0;
}
