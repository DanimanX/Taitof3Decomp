#include "obj_1253D0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"
#include "../kernel/rng.h"

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8).
 * The scaled threshold is byte-swapped by `swap D1`, so its low word is the
 * shifted product; callers test the low word of the swapped quotient. */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint32_t d1 = (uint32_t)(uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    d1 = (uint32_t)(uint16_t)d0w * d1;                 /* mulu.w */
    d1 = (d1 << 8) & 0xFFFFFFFFu;                      /* lsl.l #8 */
    d1 = (d1 << 16) | (d1 >> 16);                      /* swap */
    uint16_t thr = (uint16_t)d1;
    if (thr == 0u)
        thr = 1u;                                      /* 0x0CC2D0 */
    uint32_t num = rd16(m, F3_A5 + 0x0878u);           /* divu.w D1w,D0 */
    uint32_t d0 = ((num % thr) << 16) | ((num / thr) & 0xFFFFu);
    d0 = (d0 << 16) | (d0 >> 16);                      /* swap D0 */
    return (uint16_t)d0;
}

/* ROM 0x1253D0: the mid-boss fan. While the game flag A5-0x144B bit 5 is
 * clear it either forces the "wind-up" sprite 0x03A210 (when it has not been
 * armed yet) or walks the 0x125D08 animation; a zero-duration entry arms the
 * +0x34 flag and plays sound 0x1B. With A5+0x30EE bit 12 set and a 9-in-N roll
 * passing it throws four 0x0CCF72 puffs and a 0x0CDCA0 plate, then plays sound
 * 0x3B. Every path ends in the 0x11B06E/0x11B09A pair, then `bra 0x1253C8`
 * rewrites +0x0 with 0x1253D0 (the self-loop captured in the golden pairs). */
int Obj_1253D0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1253D0: movea.l (0x8,A0),A1 ; tst.b (0x59,A1) ; bne 0x125520 */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        if (rd8(m, a1 + 0x59u) != 0u)
            goto kill_125520;
    }

    /* 0x1253DC: bftst (0x30EE,A5){0xB:1} ; bne 0x1253FA */
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0008u) == 0u) {
        /* 0x1253E6: tst.b (0x34,A0) ; bne 0x1253FA */
        if (rd8(m, obj + 0x34u) == 0u) {
            /* 0x1253EE: move.l #0x3A210,(0x10,A0) ; bra 0x125506 */
            wr32(m, obj + 0x10u, 0x0003A210u);
            goto tail_125506;
        }
    }

    /* 0x1253FA: A1 = +0xC, one animation step off the 0x125D08 table */
    {
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        Obj_AnimFrameStep(m, obj, 0x00125D08u);        /* 0x1253FE */
        int16_t dur = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au));
        if (dur > 0)
            goto tail_125506;                          /* 0x125408 */
        if (dur < 0)
            goto idle_1254fa;                          /* 0x12540C */
    }

    /* 0x125410: tst.b (0x34,A0) ; bne 0x125430 */
    if (rd8(m, obj + 0x34u) == 0u) {
        wr8(m, obj + 0x34u, 0xFFu);                    /* 0x125418: st */
        Sound_PlayHooked(m, 0x001Bu);                  /* 0x12541C */
    }

    /* 0x125430: bftst (0x30EE,A5){0xC:1} ; beq 0x1254E8 */
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0010u) != 0u) {
        /* 0x12543A: move.w #9,D0w ; jsr 0x0CC2BC ; tst.w D0w ; bne 0x1254E8 */
        if (sub_0CC2BC(m, 0x0009u) == 0u) {
            /* 0x12544A: Rng_Next, keep the bits of D0w{3:5} and rebase them to
             * 0x7000; the same long is reused (and 0xC00 below it) as the
             * argument of the four 0x0CCF72 children */
            uint32_t d0 = Rng_NextMem(m);
            uint32_t d1 = 0x06070000u;                 /* move.l #0x6070000,D1 */
            uint16_t w = (uint16_t)(((d0 & 0x1F00u) + 0x7000u) & 0xFFFFu);
            d1 = (d1 & 0xFFFF0000u) | (uint32_t)w;     /* 0x12545E: move.w D0w,D1w */
            for (int i = 0; i < 4; i++) {
                Obj_Spawn(m, obj, 0x000CCF72u, 9u, 2u, d1);   /* 0x125470 */
                if (i == 0) {
                    d1 = (d1 & 0xFFFF0000u) |
                         (uint32_t)(uint16_t)((uint16_t)d1 - 0x0C00u);   /* 0x12547A */
                } else if (i == 1) {
                    d1 = (d1 & 0xFFFF0000u) |
                         (uint32_t)(uint16_t)((uint16_t)d1 + 0x1800u);   /* 0x125498 */
                } else {
                    d1 = (d1 & 0xFFFF0000u) |
                         (uint32_t)(uint16_t)((uint16_t)d1 - 0x0C00u);   /* 0x1254B6 */
                }
            }
            Obj_Spawn(m, obj, 0x000CDCA0u, 9u, 2u, d1);          /* 0x1254CA */
            Sound_PlayHooked(m, 0x003Bu);              /* 0x1254D4 */
        }
    }

    /* 0x1254E8: bftst (0x30EE,A5){0xB:1} ; bne 0x125506 */
    if ((rd16(m, F3_A5 + 0x30EEu) & 0x0008u) == 0u)
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));  /* 0x1254F2 */
    goto tail_125506;

idle_1254fa:
    /* 0x1254FA: sf (0x34,A0) ; clr.w (0x3A,A0) ; clr.w (0x3E,A0) */
    wr8(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    /* falls through to 0x125506 */

tail_125506:
    /* 0x125506: jsr 0x11B06E (hazard lists + hit flash) */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u) {
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
    }
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 6u) {
            wr8(m, obj + 0x35u, 0u);
            wr8(m, obj + 0x1Du, 8u);
        } else {
            wr8(m, obj + 0x1Du, 6u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }

    /* 0x12550C: movea.l (0x8,A0),A1 ; move.b (0x1D,A1),(0x1D,A0) */
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    }

    /* 0x125516: jsr 0x11B09A -- position from the scroll anchor, draw while
     * on screen, then take the anchor back off */
    {
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) +
                                 rd16(m, F3_A5 + 0x30D2u));
        wr16(m, obj + 0x14u, d0);
        d0 = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) +
                        rd16(m, F3_A5 + 0x30D6u));
        wr16(m, obj + 0x18u, d0);

        if (!((int16_t)rd16(m, obj + 0x18u) >= 0xE8 ||
              (int16_t)rd16(m, obj + 0x14u) >= 0x140)) {
            uint32_t a1 = rd32(m, obj + 0x10u);
            d0 = (uint16_t)((uint16_t)(rd16(m, a1) << 4) + rd16(m, obj + 0x18u));
            if ((int16_t)d0 > 0) {
                d0 = (uint16_t)((uint16_t)(rd16(m, a1 + 2u) << 4) + rd16(m, obj + 0x14u));
                if ((int16_t)d0 > 0)
                    Obj_DrawSprite(m, obj);            /* ROM 0x001282 */
            }
        }

        d0 = rd16(m, obj + 0x54u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        d0 = rd16(m, obj + 0x56u);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
    }

    wr32(m, obj + 0x00u, 0x001253D0u);                 /* 0x1253C8: bra */
    return 0;

kill_125520:
    /* 0x125520: free self */
    Obj_Free(m, obj);
    return 0;
}
