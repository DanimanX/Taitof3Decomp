/* ROM 0x198F6E: wave emitter. Sets the emitter position (+0x50 = 0xDC,
 * +0x54 = 0xB4) and a 0x10-tick timer (+0x22), then per tick draws the next
 * delay for +0x06 from the LCG (0x1794F0, D0=5) and spawns a 0x183268 child
 * (group 9) with a random arg 0x88+rand(0x20); if the delay is non-zero it
 * arms 0x198FBE and returns. When the timer runs out it re-arms +0x22 = 0x46,
 * +0x16 = 0x20, +0x1A = 0 and loops 0x198FDC: folds the owner position into
 * +0x50/+0x54, spawns a 0x183308 child whose arg is two signed LCG draws
 * (0x1794A4, D0=0x40 then 0x20) packed into a long, and frees itself once the
 * 0x46-tick timer expires (0x105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x1794A4 / 0x1794F0: D0.w = a value derived from the LCG state at
 * A5+0x4B3C; both variants always advance that state with
 * `state = swap(state*5 - 1)`. 0x1794A4 XORs the sign of bit 31 of the state
 * into the result (btst.b #0 at A5+0x4B3C), 0x1794F0 does not. The mask table
 * 0x179530 is indexed by (bitpos(D0) - 16). */
static uint32_t rng_mask_d0(const f3mem_t *m, uint32_t d0, int allow_neg)
{
    uint16_t r = (uint16_t)d0;
    if (d0 != 0u) {
        int off = 0;
        while (!(d0 & (0x80000000u >> off)))
            off++;
        int16_t d1w = (int16_t)(off - 16);
        uint16_t mask = rd16(m, 0x00179530u + (uint32_t)(int32_t)d1w * 2u);
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & mask);
        if ((int16_t)r < (int16_t)v)
            v = (uint16_t)(v - r);
        r = v;
        if (allow_neg && (rd8(m, F3_A5 + 0x4B3Cu) & 1u) != 0u)
            r = (uint16_t)(0u - r);
    }
    uint32_t s = rd32(m, F3_A5 + 0x4B3Cu) * 5u - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (s << 16) | (s >> 16));
    return (d0 & 0xFFFF0000u) | r;
}

int Fn_198F6E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint16_t c;

    wr16(m, obj + 0x50u, 0x00DCu);                   /* 0x198F6E */
    wr16(m, obj + 0x54u, 0x00B4u);                   /* 0x198F74 */
    wr16(m, obj + 0x22u, 0x0010u);                   /* 0x198F7A */

    for (;;) {                                       /* 0x198F80 */
        d0 = rng_mask_d0(m, 0x0020u, 0);              /* 0x198F80 moveq #0x20 */
        d0 = (d0 & 0xFFFFFF00u)
           | (uint8_t)((uint8_t)d0 + 0x88u);          /* 0x198F88 addi.b #-0x78 */
        Obj_Spawn(m, obj, 0x00183268u, 9u, 2u, d0);   /* 0x198F9C jsr 0x1057C */

        d0 = rng_mask_d0(m, 0x0005u, 0);              /* 0x198FA6 moveq #5 */
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 5u); /* 0x198FAE */
        wr16(m, obj + 0x06u, (uint16_t)d0);           /* 0x198FB0 */
        if ((uint16_t)d0 != 0u) {                     /* 0x198FB4 beq */
            wr32(m, obj + 0x00u, 0x00198FBEu);        /* 0x198FB6 */
            goto done;                                /* 0x198FBC rts */
        }

        c = (uint16_t)(rd16(m, obj + 0x22u) - 1u);    /* 0x198FC4 subq.w */
        wr16(m, obj + 0x22u, c);
        if ((int16_t)c > 0)                           /* 0x198FC8 bgt */
            continue;

        wr16(m, obj + 0x22u, 0x0046u);                /* 0x198FCA */
        wr16(m, obj + 0x16u, 0x0020u);                /* 0x198FD0 */
        wr16(m, obj + 0x1Au, 0x0000u);                /* 0x198FD6 */

        for (;;) {                                    /* 0x198FDC */
            uint32_t a4 = rd32(m, obj + 0x08u);       /* 0x198FDC */
            d0 = (d0 & 0xFFFF0000u) | rd16(m, a4 + 0x50u);   /* 0x198FE0 */
            d1 = (d1 & 0xFFFF0000u) | rd16(m, a4 + 0x54u);   /* 0x198FE4 */
            d0 = (d0 & 0xFFFF0000u)
               | (uint16_t)((uint16_t)d0 + rd16(m, obj + 0x16u)); /* 0x198FE8 */
            d1 = (d1 & 0xFFFF0000u)
               | (uint16_t)((uint16_t)d1 + rd16(m, obj + 0x1Au)); /* 0x198FEC */
            wr16(m, obj + 0x50u, (uint16_t)d0);       /* 0x198FF0 */
            wr16(m, obj + 0x54u, (uint16_t)d1);       /* 0x198FF4 */

            d1 = 0u;                                  /* 0x198FF8 clr.l D1 */
            d0 = rng_mask_d0(m, 0x0040u, 1);          /* 0x198FFA moveq #0x40 */
            d1 = (d1 & 0xFFFFFF00u) | (d0 & 0xFFu);   /* 0x199002 move.b D0b,D1b */
            d1 = (d1 << 8);                           /* 0x199004 lsl.l #8 */
            d0 = rng_mask_d0(m, 0x0020u, 1);          /* 0x199006 moveq #0x20 */
            d1 = (d1 & 0xFFFFFF00u) | (d0 & 0xFFu);   /* 0x19900E move.b D0b,D1b */
            d1 = (d1 >> 16) | (d1 << 16);             /* 0x199010 swap D1 */

            Obj_Spawn(m, obj, 0x00183308u, 9u, 2u, d1); /* 0x199022 jsr 0x1057C */

            d0 = rng_mask_d0(m, 0x0005u, 0);          /* 0x19902C moveq #5 */
            wr16(m, obj + 0x06u, (uint16_t)d0);       /* 0x199034 */
            /* 0x199038 beq.b 0x199048: the Z flag is the one 0x1794F0 leaves
             * behind (its last flag op is `swap D1`, i.e. the new LCG state
             * being zero), not a test of the delay. */
            if (rd32(m, F3_A5 + 0x4B3Cu) != 0u) {
                wr32(m, obj + 0x00u, 0x00199042u);    /* 0x19903A */
                goto done;                            /* 0x199040 rts */
            }

            c = (uint16_t)(rd16(m, obj + 0x22u) - 1u); /* 0x199048 subq.w */
            wr16(m, obj + 0x22u, c);
            if ((int16_t)c > 0)                       /* 0x19904C bgt */
                continue;

            Obj_Free(m, obj);                         /* 0x199050 jsr 0x105EC */
            goto done;                                /* 0x19905A rts */
        }
    }

done:
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
