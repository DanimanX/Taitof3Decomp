/* ROM 0x0F4C8C: enemy state/part initializer (A0 = object).
 * Spawns group-5 object 0x0F4846 (retires via 0x0CEE74 on failure), seeds
 * params from 0x0F471A (0x0CEC4C), optionally sets state block +0x08 = 0xB4
 * when Obj_Next4Way byte is non-negative, installs script 0x0F4CD4 (0x0CF1D8)
 * and behaviour 0x0F47CA (0x0D0678), sets +0x06 = 1, +0x34 |= 4, and arms
 * 0x0F4586. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0F4C8C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = r->a[1];

    uint32_t sp = Obj_Spawn(m, obj, 0x000F4846u, 5u, 2u, 0u); /* 0x0F4C9E */
    d0 = sp;
    if ((int32_t)sp < 0) {                              /* 0x0F4CA8..0x0F4CAA */
        Obj_StartDying(m, obj);
        goto done;
    }

    a1 = 0x000F471Au;                                   /* 0x0F4CB0 */
    uint32_t a2 = Obj_SeedFromParams(m, obj, a1);       /* 0x0F4CB4 */
    uint16_t rnd = Obj_Next4Way(m);                     /* 0x0F4CBA */
    d0 = setw(d0, rnd);
    if ((int8_t)(rnd & 0xFFu) >= 0)                     /* 0x0F4CC0..0x0F4CC2 */
        wr16(m, a2 + 0x08u, 0x00B4u);                   /* 0x0F4CC6 */

    Obj_StartScript(m, obj, 0x000F4CD4u);               /* 0x0F4CCC..0x0F4562 */
    a1 = 0x000F47CAu;                                   /* 0x0F4568 */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F456C */
    d0 = setw(d0, rd16(m, a1 + 2u));
    a1 += 8u;
    wr8(m, obj + 0x06u, 1u);                            /* 0x0F4572 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* 0x0F4578 */
    wr32(m, obj, 0x000F4586u);                          /* 0x0F457E */

done:
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
