#include "obj_12D36C.h"

/* ROM 0x12D364: hold in this state */
static void sub_12D364(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D36Cu);
}

/* ROM 0x12D36C: long hold, then the short hold */
int Obj_12D36C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12D370: bgt 0x12D364 */
        sub_12D364(m, obj);
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
