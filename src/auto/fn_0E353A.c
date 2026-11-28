#include "fn_auto.h"
/* ROM 0x0E353A: moving part tick (A0 = object). Nudges +0x14.W by -2, steps the
 * animation/script (0x0D0690) and returns while the +0x4C timer is non-zero.
 * Once it expires the object unlinks itself from its doubly linked list:
 * A1 = +0x50 (next), A2 = +0x54 (prev); A1+0x54 = A2, and if A2 is not
 * negative A2+0x50 = A1. Either way it then dies (0x0CEE74). */
#include "../objects/obj_common.h"   /* Obj_StartDying (0x0CEE74) */

/* ROM 0x0D0690 with D0/A1 register tracking (exact per the bytes: D0 is only
 * ever written by `move.w (A1)+,D0`, so its high half is cleared). */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)                                       /* 0x0D0694 beq ; 0x0D0698 rts */
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;          /* 0x0D069A movea.l ; adda.w #0xC */
    wr32(m, obj + 0x48u, a1);                            /* 0x0D06A2 */
    uint16_t d0 = rd16(m, a1);                           /* 0x0D06A6 move.w (A1)+,D0 */
    a1 += 2u;
    if ((int16_t)d0 < 0) {                               /* 0x0D06A8 bmi */
        int16_t back = (int16_t)rd16(m, a1);             /* 0x0D06C0 tst.w (A1) */
        if (back == 0) {                                 /* 0x0D06C2 beq 0x0D06DE */
            *d0_io = d0;
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;                   /* 0x0D06C6 suba.w (A1),A1 */
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));               /* 0x0D06CC */
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = pal;                                    /* 0x0D06D0 move.w (A1)+,D0 */
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);                            /* 0x0D06AC */
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = pal;                                        /* 0x0D06B0 move.w (A1)+,D0 */
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

int Fn_0E353A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    wr16(m, a0 + 0x14u, (uint16_t)(rd16(m, a0 + 0x14u) - 2u)); /* 0x0E353A subq.w #2 */
    step_script_regs(m, a0, &d0, &a1);                   /* 0x0E353E jsr 0x0D0690 */

    if (rd16(m, a0 + 0x4Cu) != 0u) {                     /* 0x0E3544 tst.w ; beq */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;                                        /* 0x0E354C rts */
    }

    uint32_t next = rd32(m, a0 + 0x50u);                 /* 0x0E354E */
    uint32_t prev = rd32(m, a0 + 0x54u);                 /* 0x0E3552 */
    wr32(m, next + 0x54u, prev);                         /* 0x0E3556 move.l A2,(0x54,A1) */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = next;
    r->a[2] = prev;

    if ((int32_t)prev >= 0)                              /* 0x0E355A bmi 0x0CEE74 */
        wr32(m, prev + 0x50u, next);                     /* 0x0E3560 move.l A1,(0x50,A2) */
    Obj_StartDying(m, a0);                               /* 0x0E3564 bra 0x0CEE74 */
    return 0;
}
