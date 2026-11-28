#include "fn_auto.h"
/* ROM 0x0E2C4C: burst spawner (A0 = obj). If the parent (A0+0x08) has +0x06
 * == 0 the object starts dying (0x0CEE74). Otherwise it decrements the byte
 * +0x23; while it stays non-zero the tick just returns. When it hits zero it
 * spawns 0x0E2CA0 (group 9, count 2, arg 0) through 0x01057C, dies when the
 * allocation fails, else copies the byte +0x21 into the child's +0x10,
 * decrements +0x21 (dies when it reaches 0) and reloads +0x23 = 2. */
#include "../objects/obj_common.h"
#include "fn_call_frames.h"

int Fn_0E2C4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    r->a[1] = rd32(m, obj + 0x08u);              /* 0x0E2C4C movea.l (8,A0),A1 */
    if (rd8(m, r->a[1] + 0x06u) == 0u) {         /* 0x0E2C54 beq.l 0x0CEE74 */
        Obj_StartDying(m, obj);
        return 0;
    }

    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u)); /* 0x0E2C5A */
    if (rd8(m, obj + 0x23u) != 0u)               /* 0x0E2C5E bne.w 0x0E2C9E */
        return 0;                                /* 0x0E2C9E rts */

    /* 0x0E2C62..0x0E2C7A: pea 0 ; move.w #2,-(SP) ; move.w #9,-(SP) ;
     * move.l #0x0E2CA0,-(SP) ; jsr 0x01057C ; lea (0xC,SP),SP */
    call_spawn(m, r, 0x000E2C7Au, 0x000E2CA0u, 9u, 2u, 0u);

    if ((int32_t)r->d[0] < 0) {                  /* 0x0E2C80 bmi.l 0x0CEE74 */
        Obj_StartDying(m, obj);
        return 0;
    }

    r->a[1] = r->d[0];                           /* 0x0E2C86 movea.l D0,A1 */
    wr8(m, r->a[1] + 0x10u, rd8(m, obj + 0x21u)); /* 0x0E2C88 */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u)); /* 0x0E2C8E */
    if (rd8(m, obj + 0x21u) == 0u) {             /* 0x0E2C92 beq.l 0x0CEE74 */
        Obj_StartDying(m, obj);
        return 0;
    }
    wr8(m, obj + 0x23u, 2u);                     /* 0x0E2C98 */
    return 0;                                    /* 0x0E2C9E rts */
}
