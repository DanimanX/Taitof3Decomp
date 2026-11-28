#include "fn_auto.h"
/* ROM 0x0CC1A2: short-lived text object (A0 = object). Counts +0x06 down while
 * > 0 (re-arming 0x0CC1A2); on expiry erases eight glyphs at 0x61C7A0 with the
 * blank string at 0x0CC1D7 (attribute 8, via 0x005666) and frees the object
 * (0x0105EC). */
#include "../enemy/obj_1194C2.h"
#include "../objects/obj_pool.h"

int Fn_0CC1A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t timer = (int16_t)(rd16(m, obj + 6u) - 1u);   /* 0x0CC1A2 subq.w */
    wr16(m, obj + 6u, (uint16_t)timer);
    if (timer > 0) {                                     /* 0x0CC1A6 bgt.b 0x0CC19A */
        wr32(m, obj, 0x0CC1A2u);                         /* 0x0CC19A */
        return 0;
    }

    /* 0x0CC1A8 move.w #8,-(SP) ; pea 0x61C7A0 ; pea (0x23,PC) = 0x0CC1D7. */
    r->a[7] -= 2u;
    wr16(m, r->a[7], 8u);
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x0061C7A0u);
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x0CC1D7u);
    r->d[0] = Text_DrawString(m, rd32(m, r->a[7]), rd32(m, r->a[7] + 4u),
                              rd16(m, r->a[7] + 8u));    /* 0x0CC1B8 */
    r->a[7] += 0x0Au;                                    /* 0x0CC1BE lea (0xa,SP),SP */

    /* 0x0CC1C2 move.l A0,-(SP) ; jsr 0x0105EC ; lea (4,SP),SP (preserves D0/A0/A1). */
    r->a[7] -= 4u;
    wr32(m, r->a[7], obj);
    Obj_Free(m, obj);                                    /* 0x0CC1C4 */
    r->a[7] += 4u;                                       /* 0x0CC1CA */

    return 0;                                            /* 0x0CC1CE rts */
}
