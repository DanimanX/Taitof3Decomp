#include "fn_auto.h"
/* ROM 0x12325A: boss-part timer tick (A0 = object). While +0x06 stays > 0 the
 * tick re-arms 0x12325A. On expiry it doubles the word at (A0+0x08)+0x4C,
 * clears +0x24 of the record at (A0+0x48), copies +0x36/+0x35 into it, plays
 * hooked sound 0x35, points +0x08 at itself and turns the object into the
 * 0x13E4EE explosion: +0x0C = (spr dims)<<3 packed by 0x13E4D0 with D0 = 1,
 * then jumps into 0x13E4EE (tail call, its rts is this routine's rts). */
#include "../enemy/obj_E8B72.h"

/* ROM 0x13E4EE (register-level mirror of src/enemy/obj_13E4EE.c so that D0,
 * D1, D2 and A1/A2 keep the values the ROM leaves at rts). */
static void tail_13E4EE(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);          /* 0x13E4EE movea.l (8,A0),A1 */
    wr16(m, obj + 0x1Cu, 0u);                    /* 0x13E4F2 */
    wr16(m, obj + 0x4Cu, rd16(m, a1 + 0x14u));   /* 0x13E4F6 */
    wr16(m, obj + 0x50u, rd16(m, a1 + 0x18u));   /* 0x13E4FC */
    wr16(m, obj + 0x1Eu, 0u);                    /* 0x13E502 */

    uint32_t d0 = Rng_NextMem(m);                /* 0x13E506 jsr 0x001D8A */
    uint16_t d0w = (uint16_t)(d0 & 0x0007u);     /* 0x13E50C andi.w #7 */
    uint16_t d1w = d0w;                          /* 0x13E510 move.w D0w,D1w */
    d0 = Rng_NextMem(m);                         /* 0x13E512 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)(d0 & 0x00FFu); /* 0x13E518 andi.w #0xFF */
    uint16_t ang = (uint16_t)d0;
    int16_t t = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u); /* 0x13E522 */
    int32_t prod = (int32_t)t * (int32_t)(int16_t)d1w;  /* 0x13E528 muls.w D1w,D2 */
    wr32(m, obj + 0x44u, (uint32_t)prod);        /* 0x13E52A */
    t = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u); /* 0x13E52E */
    prod = (int32_t)t * (int32_t)(int16_t)d1w;   /* 0x13E534 */
    wr32(m, obj + 0x48u, (uint32_t)prod);        /* 0x13E536 */

    d0 = rd32(m, obj + 0x0Cu);                   /* 0x13E53A move.l (0xC,A0),D0 */
    int8_t dx = (int8_t)(d0 >> 24);              /* 0x13E53E bfexts {0:8} */
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) + (uint16_t)(int16_t)dx)); /* 0x13E542 */
    int8_t dy = (int8_t)((d0 >> 16) & 0xFFu);    /* 0x13E546 bfexts {8:8} */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)(int16_t)dy)); /* 0x13E54A */
    uint16_t kind = (uint16_t)d0;                /* 0x13E54E bfextu {16:16} */
    uint32_t d1 = kind;                          /* bfextu clears the whole D1 */

    if ((int16_t)kind >= 0x0002 && (int16_t)kind <= 0x0004) { /* 0x13E552..0x13E55E */
        d0 = Rng_NextMem(m);                     /* 0x13E562 */
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)(d0 & 0x000Fu); /* 0x13E568 */
        if ((uint16_t)d0 != 0u) {                /* 0x13E56C beq 0x13E586 */
            d1 = 2u;                             /* 0x13E570 moveq #2,D1 */
            if ((int16_t)(uint16_t)d0 > 3) {     /* 0x13E572..0x13E576 */
                d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 + 1u); /* 0x13E57A addq.w #1 */
                if ((int16_t)(uint16_t)d0 > 9)   /* 0x13E57C..0x13E580 */
                    d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 + 1u); /* 0x13E584 */
            }
        }
    }

    /* 0x13E586 lea (0x9A,PC),A2 -> 0x13E622 ; 0x13E58C movea.l (0,A2,D1w*4),A2 */
    uint32_t a2 = rd32(m, 0x0013E622u + (uint32_t)(int32_t)(int16_t)(uint16_t)d1 * 4u);
    wr32(m, obj + 0x0Cu, a2);                    /* 0x13E590 move.l A2,(0xC,A0) */

    d0 = Rng_NextMem(m);                         /* 0x13E594 */
    d0 = (d0 & 0xFFFFFF00u) | (uint16_t)(d0 & 0x0003u); /* 0x13E59A andi.b #3 */
    wr8(m, obj + 0x1Cu, (uint8_t)(d0 & 0xFFu));  /* 0x13E59E move.b D0b,(0x1C,A0) */
    wr16(m, obj + 0x3Au, 0u);                    /* 0x13E5A2 */
    wr16(m, obj + 0x3Eu, 0u);                    /* 0x13E5A6 */
    wr32(m, obj, 0x0013E5B2u);                   /* 0x13E5AA */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = (uint32_t)prod;                    /* last muls.w D1w,D2 */
    r->a[1] = a1;
    r->a[2] = a2;
}

int Fn_12325A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x12325A subq.w #1 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                /* 0x12325E bgt.b 0x123252 */
        wr32(m, obj, 0x0012325Au);                         /* 0x123252 */
        return 0;                                          /* 0x123258 rts */
    }

    uint32_t a1 = rd32(m, obj + 0x08u);                    /* 0x123260 movea.l (8,A0),A1 */
    wr16(m, a1 + 0x4Cu, (uint16_t)(rd16(m, a1 + 0x4Cu) << 1)); /* 0x123264 lsl.w (0x4C,A1) */
    a1 = rd32(m, obj + 0x48u);                             /* 0x123268 */
    wr32(m, a1 + 0x24u, 0u);                               /* 0x12326C clr.l (0x24,A1) */
    wr32(m, a1 + 0x36u, rd32(m, obj + 0x36u));             /* 0x123270 */
    wr8(m, a1 + 0x35u, rd8(m, obj + 0x35u));               /* 0x123276 */
    Sound_PlayHooked(m, 0x0035u);                          /* 0x12327C..0x12328C */
    wr32(m, obj + 0x08u, obj);                             /* 0x123290 move.l A0,(8,A0) */

    /* 0x123294 moveq #1,D0 ; 0x123296 jsr 0x0013E4D0 ; 0x12329C move.l D0,(0xC,A0) */
    {
        uint32_t spr = rd32(m, obj + 0x10u);
        uint32_t w0 = ((uint32_t)rd16(m, spr) << 3) & 0xFFu;
        uint32_t w1 = ((uint32_t)rd16(m, spr + 2u) << 3) & 0xFFu;
        uint32_t d0 = 1u | (w0 << 16) | (w1 << 24);
        wr32(m, obj + 0x0Cu, d0);
    }

    tail_13E4EE(m, r, obj);                                /* 0x1232A0 jmp 0x0013E4EE */
    r->a[0] = obj;
    return 0;
}
