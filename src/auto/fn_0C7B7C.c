/* ROM 0x0C7B7C: player game-over/continue countdown. Each tick decrements the
 * +0x06 delay and re-arms 0x0C7B7C while it stays positive. When it expires the
 * +0x34 "active" flag is cleared, the score A5+0x88A gains 0x12C, A5+0x87A is
 * reduced by 1/16 of itself, A5+0x882 is set to 0x990000 and 0x0C6D0E respawns
 * the player. Then the +0x4C continue counter is decremented: while it is still
 * >= 0 the next state 0x0C7946 is installed; once it goes negative the +0x34
 * flag is set, the 0x4AF countdown/label words are stored, and 0x0009E0 queues
 * the "CONTINUE?" style command built from +0x23/+0x48/A5+0x32DE before control
 * falls into 0x0C7480, which prints the 0x0C95D3 string at
 * 0x61C000 + word table 0x0C915A[+0x22] and one row pair below it, then arms
 * 0x0C74CC. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"                  /* Text_DrawString (0x005666) */
#include "../common/fn_leaf1.h"                   /* Leaf_0C660A */

/* DIVU.W keeps the dividend when the quotient would overflow. */
static uint32_t divu_word(uint32_t dividend, uint16_t divisor)
{
    uint32_t quotient = dividend / divisor;
    if (quotient > 0xFFFFu)
        return dividend;
    return ((dividend % divisor) << 16) | quotient;
}

/* ROM 0x0C6D0E: respawn the player (shot object, position, HP, invulnerability,
 * power/multiplier penalties, arm pairs, 0x0C8E1A + 0x0C8BCE, and the
 * A5+0x3318 continue bonus), saving D0/D1/A1 around it. */
static void sub_0C6D0E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t p = r->a[7];
    uint32_t a0 = r->a[0];

    /* 0x0C6D0E: predecrement MOVEM {A0,A1,D1} (mask 0xC040) -> [P-4]=A1,
     * [P-8]=D1, [P-12]=D0. */
    wr32(m, p - 4u, r->a[1]);
    wr32(m, p - 8u, r->d[1]);
    wr32(m, p - 12u, r->d[0]);
    r->a[7] = p - 12u;                                       /* MOVEM moves SP */

    /* 0x0C6D12..0x0C6D24: Obj_Spawn(parent, 0x0C95DE, group 0x0B, 1, parent) */
    call_spawn(m, r, 0x000C6D2Au, 0x000C95DEu, 0x0Bu, 1u, rd32(m, a0 + 0x62u));

    wr8(m, a0 + 0x65u, 0u);                                  /* 0x0C6D2E sf */
    uint32_t a1 = 0x000C926Eu                              /* 0x0C6D32 (0x253A,PC) */
        + (uint32_t)(rd16(m, a0 + 0x22u) * 4u);              /* 0x0C6D38 mulu.w */
    wr16(m, a0 + 0x14u, rd16(m, a1));                        /* 0x0C6D42 */
    wr16(m, a0 + 0x18u, rd16(m, a1 + 2u));                   /* 0x0C6D46 */
    wr32(m, a0 + 0x24u, 0x00010000u);                        /* 0x0C6D4A */
    wr32(m, a0 + 0x28u, 0x00010000u);                        /* 0x0C6D52 */
    wr16(m, a0 + 0x50u, 0x00D2u);                            /* 0x0C6D5A */

    /* 0x0C6D5E: take the /3 quotient off the multiplier and the remainder. */
    uint32_t d0 = divu_word((uint32_t)rd16(m, a0 + 0x54u), 3u);
    if ((uint16_t)d0 != 0u) {                                /* 0x0C6D68 tst.w / beq */
        wr16(m, a0 + 0x54u, (uint16_t)(rd16(m, a0 + 0x54u) - 3u));
        wr16(m, a0 + 0x54u,
             (uint16_t)(rd16(m, a0 + 0x54u) - (uint16_t)((d0 << 16) | (d0 >> 16))));
    }
    /* 0x0C6D78: only the /5 remainder is subtracted. */
    d0 = divu_word((uint32_t)rd16(m, a0 + 0x58u), 5u);
    wr16(m, a0 + 0x58u,
         (uint16_t)(rd16(m, a0 + 0x58u) - (uint16_t)((d0 << 16) | (d0 >> 16))));

    wr8(m, a0 + 0x34u, 0u);                                  /* 0x0C6D8A sf */
    wr32(m, a0 + 0x3Au, 0u);                                 /* 0x0C6D8E clr.l */
    wr16(m, a0 + 0x3Eu, 0u);                                 /* 0x0C6D92 */
    wr16(m, a0 + 0x40u, 0u);                                 /* 0x0C6D96 */
    wr16(m, a0 + 0x4Eu, 0u);                                 /* 0x0C6D9A move.w #0 */
    wr16(m, a0 + 0x60u, rd16(m, F3_A5 + 0x1D3Cu));           /* 0x0C6DA0 */
    wr16(m, a0 + 0x6Au, rd16(m, a0 + 0x56u));                /* 0x0C6DA6 */
    wr16(m, a0 + 0x6Cu, 0xFFFFu);                            /* 0x0C6DAE */
    wr16(m, a0 + 0x7Au, rd16(m, a0 + 0x58u));                /* 0x0C6DB4 */
    wr16(m, a0 + 0x7Cu, 0xFFFFu);                            /* 0x0C6DBC */

    push_long(m, r, 0x000C6DC6u);                            /* 0x0C6DC2 bsr */
    Fn_0C8E1A(m, r);
    r->a[7] = p - 12u;
    push_long(m, r, 0x000C6DCAu);                            /* 0x0C6DC6 bsr */
    Fn_0C8BCE(m, r);
    r->a[7] = p - 12u;

    uint16_t bonus = (uint16_t)(rd16(m, F3_A5 + 0x3318u) - 0x708u); /* 0x0C6DCA */
    if ((int16_t)bonus < 0)                                  /* 0x0C6DD0 bge */
        bonus = 0;
    wr16(m, F3_A5 + 0x3318u, bonus);

    r->d[0] = 0x00050000u;                                   /* 0x0C6DD8 */
    r->d[1] = a0;                                            /* 0x0C6DDE move.l A0,D1 */
    Leaf_0C660A(m, r);                                       /* 0x0C6DE0 jsr 0x0C660A */

    r->a[1] = rd32(m, p - 4u);                               /* 0x0C6DE6 movem restore */
    r->d[1] = rd32(m, p - 8u);
    r->d[0] = rd32(m, p - 12u);
    r->a[7] = p;
}

/* ROM 0x0009E0 caller frame: movem {A0,A1}, then the byte/long/word arguments
 * of 0x0C7BC6..0x0C7BCA, which 0x0009E0 reads through its LINK frame. */
static void call_0009E0(const f3mem_t *m, f3regs_t *r, uint8_t arg0,
                        uint32_t arg1, uint16_t arg2)
{
    uint32_t p = r->a[7];
    uint32_t obj = r->a[0];

    /* 0x0C7BBA: predecrement MOVEM {A0,A1} (mask 0x00C0) -> [P-4]=A1, [P-8]=A0 */
    wr32(m, p - 4u, r->a[1]);
    wr32(m, p - 8u, obj);
    wr16(m, p - 10u, arg2);                                  /* 0x0C7BBE move.w */
    wr32(m, p - 14u, arg1);                                  /* 0x0C7BC2 move.l */
    wr8(m, p - 16u, arg0);                                   /* 0x0C7BC6 move.b */
    r->a[7] = p - 20u;                                       /* jsr pushes the return */
    wr32(m, r->a[7], 0x000C7BD0u);
    Fn_0009E0(m, r);

    /* 0x0C7BD0 addq.l #8,SP ; 0x0C7BD2 movem.l (SP)+,{A0,A1} */
    r->a[0] = rd32(m, p - 8u);
    r->a[1] = rd32(m, p - 4u);
    r->a[7] = p;
}

/* ROM 0x0C7480: print the string 0x0C95D3 twice in text RAM at
 * 0x61C000 + word table 0x0C915A[+0x22] and one row pair (0x80) below, then
 * arm 0x0C74CC. */
static void sub_0C7480(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint16_t idx = rd16(m, a0 + 0x22u);                      /* 0x0C7486 move.w */
    r->d[0] = idx;                                           /* move.w zero-extends */
    r->a[2] = 0x000C915Au;                                   /* 0x0C748A (0x1CCE,PC) */
    r->a[1] = 0x0061C000u
        + (uint32_t)(int32_t)(int16_t)rd16(m, 0x000C915Au + 2u * (uint32_t)(int16_t)idx);
    r->d[0] = Text_DrawString(m, 0x000C95D3u, r->a[1], 0u);   /* 0x0C74A0 */
    r->a[1] += 0x80u;                                        /* 0x0C74AA */
    r->d[0] = Text_DrawString(m, 0x000C95D3u, r->a[1], 0u);   /* 0x0C74BA */
    wr32(m, a0, 0x000C74CCu);                                /* 0x0C74C4 */
}

int Fn_0C7B7C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];

    uint16_t delay = (uint16_t)(rd16(m, a0 + 6u) - 1u);       /* 0x0C7B7C */
    wr16(m, a0 + 6u, delay);
    if ((int16_t)delay > 0) {                                 /* 0x0C7B80 bgt 0x0C7B74 */
        wr32(m, a0, 0x000C7B7Cu);
        return 0;                                             /* 0x0C7B7A rts */
    }

    wr8(m, a0 + 0x34u, 0u);                                   /* 0x0C7B82 sf */
    wr16(m, F3_A5 + 0x88Au, (uint16_t)(rd16(m, F3_A5 + 0x88Au) + 0x12Cu)); /* 0x0C7B86 */
    r->d[0] = rd32(m, F3_A5 + 0x87Au);                        /* 0x0C7B8C */
    r->d[0] = (uint32_t)((int32_t)r->d[0] >> 4);              /* 0x0C7B90 asr.l #4 */
    wr32(m, F3_A5 + 0x87Au, rd32(m, F3_A5 + 0x87Au) - r->d[0]); /* 0x0C7B92 */
    wr32(m, F3_A5 + 0x882u, 0x00990000u);                     /* 0x0C7B96 */
    sub_0C6D0E(m, r);                                         /* 0x0C7B9E bsr */

    uint16_t cont = (uint16_t)(rd16(m, a0 + 0x4Cu) - 1u);      /* 0x0C7BA2 */
    wr16(m, a0 + 0x4Cu, cont);
    if ((int16_t)cont >= 0) {                                 /* 0x0C7BA6 bge 0x0C7BDA */
        wr32(m, a0, 0x000C7946u);                             /* 0x0C7BDA -> 0x0C793E */
        return 0;
    }

    wr8(m, a0 + 0x34u, 0xFFu);                                /* 0x0C7BAA st */
    wr16(m, a0 + 0x62u, 0x04AFu);                             /* 0x0C7BAE */
    wr16(m, F3_A5 + 0x1D3Eu, 0x04AFu);                        /* 0x0C7BB4 */
    call_0009E0(m, r, rd8(m, a0 + 0x23u), rd32(m, a0 + 0x48u),
                rd16(m, F3_A5 + 0x32DEu));                    /* 0x0C7BBA..0x0C7BCA */
    sub_0C7480(m, r);                                         /* 0x0C7BD6 bra 0x0C7480 */
    return 0;
}
