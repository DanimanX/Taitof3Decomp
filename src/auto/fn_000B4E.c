/* ROM 0x000B4E: queue serial command 0xE2, the caller's byte argument, and the
 * 0xFF terminator, then drain the sound ring through 0x000BF6. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../kernel/soundque.h"

/* ROM 0x000BF6 Sound_PushCmd, register level (see src/auto/fn_000B30.c). */
static int push_cmd(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t idx = rd16(m, a5 - 0x7F7Eu);

    if (idx == rd16(m, a5 - 0x7F80u)) {           /* 0x0BFA cmp.w ; 0x0BFE beq */
        r->d[0] = 0;                              /* 0x0C2C moveq #0,D0 */
        return 0;
    }
    r->a[0] = a5 - 0x8000u;                       /* 0x0C02 lea */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, r->a[0] + idx); /* 0x0C06 move.b */
    r->d[0] = (r->d[0] & 0xFFFF0000u)
            | (uint16_t)((uint16_t)((uint16_t)r->d[0] << 1) | 0x0200u); /* 0x0C0A/0x0C0C */
    Sound_EnqueueExact(m, (uint16_t)r->d[0]);      /* 0x0C12 bsr 0x000910 */
    r->a[0] = a5 - 0x5DCAu;                       /* 0x0910 lea -0x5DCA(A5),A0 */
    r->a[1] = 0x004A0013u;                        /* 0x0914 lea 0x4A0013.l,A1 */
    r->d[1] = 0xFFFFu;                            /* 0x091E moveq #9 ; 0x093E dbra */
    idx = (uint16_t)(idx + 1u);                   /* 0x0C1A addq.w #1,D0 */
    if (idx == 0x80u)                             /* 0x0C1C cmpi.w #0x80 */
        idx = 0;                                  /* 0x0C22 moveq #0,D0 */
    wr16(m, a5 - 0x7F7Eu, idx);                   /* 0x0C24 move.w D0,-0x7F7E(A5) */
    r->d[0] = 1;                                  /* 0x0C28 moveq #1,D0 */
    return 1;
}

int Fn_000B4E(const f3mem_t *m, f3regs_t *r)
{
    uint8_t arg;

    r->a[7] -= 2u;                                /* 0x000B4E move.b #-0x1E,-(SP) */
    wr8(m, r->a[7], 0xE2u);
    push_long(m, r, 0x000B56u);                   /* 0x000B52 bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B56 addq.l #2,SP */

    arg = rd8(m, r->a[7] + 4u);                   /* 0x000B58 move.b (0x4,SP),-(SP) */
    r->a[7] -= 2u;
    wr8(m, r->a[7], arg);
    push_long(m, r, 0x000B60u);                   /* 0x000B5C bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B60 addq.l #2,SP */

    r->a[7] -= 2u;                                /* 0x000B62 move.b #-0x1,-(SP) */
    wr8(m, r->a[7], 0xFFu);
    push_long(m, r, 0x000B6Au);                   /* 0x000B66 bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B6A addq.l #2,SP */

    while (push_cmd(m, r) != 0)                   /* 0x000B6C bsr 0x000BF6 ; tst.l */
        ;                                         /* 0x000B72 bne 0x000B6C */
    return 0;                                     /* 0x000B74 rts, D0 = 0 */
}
