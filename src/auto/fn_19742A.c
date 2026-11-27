/* ROM 0x19742A: object countdown step. Decrements the word at +0x06; while it
 * stays positive the object re-arms itself with 0x19742A, otherwise it falls
 * through to 0x197430 (Obj_Free of A0) and returns. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19742A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x19742A */
    wr16(m, obj + 0x06u, (uint16_t)n);

    if (n > 0)
        wr32(m, obj + 0x00u, 0x0019742Au);   /* 0x19742E bgt -> 0x197422 */
    else
        Obj_Free(m, obj);                    /* 0x197430 move.l a0,-(sp) ; jsr 0x105EC */
    return 0;
}
