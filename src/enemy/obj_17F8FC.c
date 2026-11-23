#include "obj_17F8FC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_1795F6.h"
#include "../enemy/obj_17960A.h"

/* ROM 0x13F238 + 0x3B66: forced sound command from a word on the stack; the
 * pre-cmd hook shifts 1..0x40 by +0x53 when A5+0x3306 is set. */
static void play_sound_forced(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmdForced(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmdForced(&tmp_m, cmd);
    }
}

/* Frame-script op table 0x179A5C as used from here: op -> {init, tick}. The
 * init routines are the declared ROM entry points (obj_1795F6.h / obj_17960A.h);
 * the tick bodies of op 0 (0x179664, with its 0x179550 reload), op 3
 * (0x179700, with its 0x179572 reload) and ops 1/2 (0x179880 / 0x1798BA) are
 * re-emitted below because the library copies are static: only the init entry
 * points are exported. */
extern int Obj_1795F6_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x1795F6 */
extern int Obj_17960A_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x17960A */

/* ROM 0x179550 (op 0 init): $3E = rec.dur.w, $10 = rec.spr.l */
static void rec_op0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
}

/* ROM 0x179572 (op 3 init): {dur.w, spr.l, 2C.l, 30.l} */
static void rec_op3(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
}

/* ROM 0x179880 (op 1 tick): one of 16 {sprite.l} at +0x3A, (b20 + 8) >> 4 */
static void tick_op1(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    wr32(m, obj + 0x10u, rd32(m, rd32(m, obj + 0x3Au) + (uint32_t)idx * 4u));
}

/* ROM 0x1798BA (op 2 tick): one of 32 {sprite.l} at +0x3A, (b20 + 4) >> 3 */
static void tick_op2(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    wr32(m, obj + 0x10u, rd32(m, rd32(m, obj + 0x3Au) + (uint32_t)idx * 4u));
}

/* ROM 0x179A5C init half: jsr ((0x0,A1,D0w*8)) */
static int anim_init(const f3mem_t *m, uint32_t obj, uint16_t op)
{
    switch (op) {
    case 0u: rec_op0(m, obj); return 0;              /* ROM 0x179550 */
    case 1u: return Obj_1795F6_Tick(m, obj);         /* ROM 0x1795F6 */
    case 2u: return Obj_17960A_Tick(m, obj);         /* ROM 0x17960A */
    case 3u: rec_op3(m, obj); return 0;              /* ROM 0x179572 */
    default: return -1;
    }
}

/* Shared walk of op 0 / op 3 ticks (0x179664 / 0x179700): a1 = (0x3A,A0) +
 * step and then
 *   (0x3E,A0) > 0 -> subq.w #1 only
 *   (0x3E,A0) < 0 -> rts (hold the frame)
 *   (0x3E,A0) = 0 -> tst.w (a1): >0 -> (0x3A,A0) = a1 and reload the record
 *                                <0 -> subq.w #1 only
 *                                =0 -> a1 = (a1+2), the record's op byte (low
 *                                      nibble) to +0x40, (0x3A,A0) = a1+2 and
 *                                      the op init
 *   then subq.w #1,(0x3E,A0) */
static int tick_walk(const f3mem_t *m, uint32_t obj, uint32_t step, int op3)
{
    int16_t t = (int16_t)rd16(m, obj + 0x3Eu);
    if (t > 0) {                                     /* bgt 0x1796AC */
        wr16(m, obj + 0x3Eu, (uint16_t)(t - 1));
        return 0;
    }
    if (t < 0)
        return 0;                                    /* 0x179670 rts */

    uint32_t a1 = rd32(m, obj + 0x3Au) + step;
    int16_t w = (int16_t)rd16(m, a1);
    if (w > 0) {
        wr32(m, obj + 0x3Au, a1);
        if (op3) rec_op3(m, obj);                    /* bsr 0x179572 */
        else     rec_op0(m, obj);                    /* bsr 0x179550 */
    } else if (w == 0) {
        a1 = rd32(m, a1 + 2u);                       /* movea.l (a1+2),a1 */
        uint16_t op = (uint16_t)(rd16(m, a1) & 0x0Fu);
        wr16(m, obj + 0x40u, op);
        wr32(m, obj + 0x3Au, a1 + 2u);
        if (anim_init(m, obj, op) < 0)
            return -1;
    }
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    return 0;
}

/* ROM 0x179A5C animation op dispatch: D0w = (0x40,A0); jsr ((0x4,A1,D0w*8))[2] */
static int anim_tick(const f3mem_t *m, uint32_t obj)
{
    switch (rd16(m, obj + 0x40u)) {
    case 0u: return tick_walk(m, obj, 6u, 0);        /* ROM 0x179664 */
    case 1u: tick_op1(m, obj); return 0;             /* ROM 0x179880 */
    case 2u: tick_op2(m, obj); return 0;             /* ROM 0x1798BA */
    case 3u: return tick_walk(m, obj, 0x0Eu, 1);     /* ROM 0x179700 */
    default: return -1;
    }
}

/* Drop the fractional part of the 16.16 position pair at +0x14/+0x18. */
static void pos_frac_clear(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x16u, 0u);
    wr16(m, obj + 0x1Au, 0u);
}

/* ROM 0x17F8FC: the 0x17F6C8 twin that also appends itself to the hazard list
 * A5+0x906 on every drawn frame. Same frame-script loop, same countdown at
 * +0x64 (re-arm 0x17F8F4, stop 0x17F732: forced sound 0x22 then 0x17F74E). */
int Obj_17F8FC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17F8FC: btst.b #0,(0x409D58).l ; bne 0x17F732 */
    if (rd8(m, 0x409D58u) & 0x01u) {
        play_sound_forced(m, 0x22);
        wr32(m, obj + 0x00u, 0x0017F74Eu);
        return 0;
    }
    /* 0x17F908: btst.b #4,(-0x144C,A5) */
    if (rd8(m, F3_A5 - 0x144Cu) & 0x10u) {
        play_sound_forced(m, 0x22);
        wr32(m, obj + 0x00u, 0x0017F74Eu);
        return 0;
    }

    /* 0x17F912: velocity integration on the parent position */
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = rd16(m, a4 + 0x50u);
    uint16_t d1 = rd16(m, a4 + 0x54u);
    d0 = (uint16_t)(d0 + rd16(m, obj + 0x16u));
    d1 = (uint16_t)(d1 + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    /* 0x17F936: jsr (table[+0x40].tick) */
    if (anim_tick(m, obj) < 0)
        return -1;

    /* 0x17F946: centre the frame: (0x14,A0) -= (A1)+ << 3, (0x18,A0) -= next */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t w1 = (uint16_t)(rd16(m, a1) << 3);        /* (A1)+ */
    uint16_t w0 = (uint16_t)(rd16(m, a1 + 2u) << 3);   /* (A1)+ */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - w0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - w1));
    pos_frac_clear(m, obj);

    Obj_DrawSprite(m, obj);                            /* ROM 0x001282 */

    Coll_AddHazard(m, obj);                            /* 0x17F960 -> 0x0D0DCC */

    /* 0x17F96A: subq.w #1,(0x64,A0); bgt 0x17F8F4 (tests the NEW value) */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x64u) - 1u);
    wr16(m, obj + 0x64u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0017F8FCu);             /* 0x17F8F4 */
        return 0;
    }

    /* 0x17F970: bra.w 0x17F732 */
    play_sound_forced(m, 0x22);
    wr32(m, obj + 0x00u, 0x0017F74Eu);                 /* 0x17F746 */
    return 0;
}
