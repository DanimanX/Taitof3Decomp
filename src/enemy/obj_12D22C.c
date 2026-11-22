#include "obj_12D22C.h"
#include "../objects/obj_pool.h"

/* ROM 0x12D224: hold in this state */
static void sub_12D224(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D22Cu);
}

/* ROM 0x12D22C: wait for the boss viewport (+0x56) to scroll out, then chain */
int Obj_12D22C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12D22C: A1 = (*(A5+0x32AA))+0x48 */
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    if ((int16_t)rd16(m, a1 + 0x56u) > -0x10) {
        /* 0x12D23A: bgt 0x12D224 */
        sub_12D224(m, obj);
        return 0;
    }

    /* 0x12D23C */
    wr32(m, obj + 0x00u, 0x0012D244u);
    return 0;
}
