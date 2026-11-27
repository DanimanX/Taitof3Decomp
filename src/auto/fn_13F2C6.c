/* ROM 0x13F2C6: UNVERIFIED (no capture): Task_StageTimeline (kernel slot 6).
 * The pre-yield init runs the stage-start wrapper 0x13F428. Each frame advances
 * the 16.16 scroll position, and whenever its integer part crosses a script
 * entry it spawns the wave objects (0x0CC11C); when the end marker (negative
 * word at data+4) is reached it runs the stage-end party (0x13D156), spawns
 * 0x11907C/0x119038/0x11912C, waits, runs the second party (0x13D19E) and the
 * 0x13F5B8 outro, then waits on the A5-0x144B bit-2 handshake. */
#include "fn_auto.h"
#include "../stage/stage_timeline.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x13D1E6: copy bytes 1..3 of each of the 32 longs at A2 to 0x41B58E,
 * scaling each by 0x0B/0x10 (mulu.w #0xB ; lsr.w #4). Bit-field offsets are
 * counted from the MSB of the big-endian long, so byte 0 (bits 31..24) stays. */
static void party_scale_13D1E6(const f3mem_t *m, uint32_t a2, uint32_t a1)
{
    for (int16_t d0 = 0x1F; d0 >= 0; d0--) {
        uint32_t off = (uint32_t)d0 * 4u;
        uint32_t s = rd32(m, a2 + off);
        uint32_t dd = rd32(m, a1 + off);
        uint8_t v;

        v = (uint8_t)((s >> 16) & 0xFFu);                       /* bfextu {0x8:0x8} */
        dd = (dd & 0xFF00FFFFu) | ((uint32_t)(uint8_t)((v * 0x0Bu) >> 4) << 16);
        v = (uint8_t)((s >> 8) & 0xFFu);                        /* bfextu {0x10:0x8} */
        dd = (dd & 0xFFFF00FFu) | ((uint32_t)(uint8_t)((v * 0x0Bu) >> 4) << 8);
        v = (uint8_t)(s & 0xFFu);                               /* bfextu {0x18:0x8} */
        dd = (dd & 0xFFFFFF00u) | (uint32_t)(uint8_t)((v * 0x0Bu) >> 4);
        wr32(m, a1 + off, dd);
    }
}

/* ROM 0x13D156: walk the 6-byte {ptr.l, weight.w} list selected by the stage
 * (table 0x13D226) until a negative word; scale ptr into 0x41B58E and queue
 * the ramp 0x41B58E <- ptr. A0/A1/A2/D0/D1 are saved and restored. */
static void party_13D156(const f3mem_t *m, uint16_t stage)
{
    uint32_t a0 = rd32(m, 0x0013D226u + (uint32_t)((int32_t)(int16_t)stage * 4));
    uint32_t a1 = 0x0041B58Eu;

    for (;;) {
        if ((int16_t)rd16(m, a0) < 0)                           /* tst.w (A0) ; bmi */
            break;
        uint32_t a2 = rd32(m, a0);                              /* movea.l (A0)+,A2 */
        a0 += 4u;
        party_scale_13D1E6(m, a2, a1);                          /* 0x13D174 */
        uint16_t d0 = rd16(m, a0);                              /* move.w (A0)+,D0w */
        a0 += 2u;
        (void)Pal_QueueRamp(m, a2, a1, d0, 0x3Cu, 0x20u);       /* 0x13D18C -> 0x183C */
    }
}

/* ROM 0x13D19E: same walk/list as 0x13D156 but the ramp runs the other way
 * (queue 0x41B58E <- ptr). */
static void party_13D19E(const f3mem_t *m, uint16_t stage)
{
    uint32_t a0 = rd32(m, 0x0013D226u + (uint32_t)((int32_t)(int16_t)stage * 4));
    uint32_t a1 = 0x0041B58Eu;

    for (;;) {
        if ((int16_t)rd16(m, a0) < 0)
            break;
        uint32_t a2 = rd32(m, a0);
        a0 += 4u;
        party_scale_13D1E6(m, a2, a1);
        uint16_t d0 = rd16(m, a0);
        a0 += 2u;
        (void)Pal_QueueRamp(m, a1, a2, d0, 0x3Cu, 0x20u);       /* 0x13D1D4 -> 0x183C */
    }
}

/* TRAP #5 delay loop `move.w #n,D7 ; subq.w #1,D7 ; L: trap #5 ;
 * btst.b #1,A5-0x144B ; bne L ; dbf D7,L`. n yields; a set pause bit suspends
 * the task (the model reports the yield with 1). */
static int delay_yield(const f3mem_t *m, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)
            return 1;
    }
    return 0;
}

int Fn_13F2C6(const f3mem_t *m, f3regs_t *r)
{
    /* 0x13F2C6 bsr.w 0x13F428; the next instruction is the 0x13F2CA yield. */
    if (Fn_13F428(m, r) < 0)
        return -1;
    return 0;
}

void TaskStageTimeline_Frame(const f3mem_t *m)
{
    /* 0x13F2CA trap #5 */
    if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)                        /* 0x13F2CC btst/bne */
        return;

    /* 0x13F2D4..0x13F340: scroll advance + wave spawns; 1 = end marker. */
    if (!StageTimeline_Step(m, 0))
        return;

    /* 0x13F342: A0 = the end entry's data pointer (script ptr + 2). */
    {
        uint32_t a0 = rd32(m, TL_SCRIPT_PTR) + 2u;

        party_13D156(m, rd16(m, F3_A5 + 0x32DCu));              /* 0x13F342 */
        Obj_Spawn(m, a0, 0x0011907Cu, 0x0Bu, 1u, 0u);           /* 0x13F348 */
        Obj_Spawn(m, a0, 0x00119038u, 0x0Bu, 1u, 0u);           /* 0x13F364 */
    }
    if (delay_yield(m, 0x14u))                                  /* 0x13F380 */
        return;
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x20u)); /* bset.b #5 */
    if (delay_yield(m, 0x64u))                                  /* 0x13F39A */
        return;
    {
        uint32_t a0 = rd32(m, TL_SCRIPT_PTR) + 2u;
        Obj_Spawn(m, a0, 0x0011912Cu, 0x0Bu, 1u, 0u);           /* 0x13F3AE */
    }
    if (delay_yield(m, 0xF0u))                                  /* 0x13F3CA */
        return;
    party_13D19E(m, rd16(m, F3_A5 + 0x32DCu));                  /* 0x13F3DE */
    if (delay_yield(m, 0x3Cu))                                  /* 0x13F3E4 */
        return;
    {
        f3regs_t tmp;                                           /* 13F5B8 ignores its regs */

        for (unsigned i = 0; i < 8u; i++) {
            tmp.d[i] = 0u;
            tmp.a[i] = 0u;
        }
        /* 0x13F3F8. Fn_13F5B8 returns > 0 when it yields at its own TRAP #5
         * loop (it then continues where it stopped); the frame ends here. */
        if (Fn_13F5B8(m, &tmp) != 0)
            return;
    }
    Fx_StartA(m, 5u);                                           /* 0x13F3FC move.w #5; jsr 0x15D0 */
    wr8(m, F3_A5 + 0x1D1Cu, 0xFFu);                             /* 0x13F406 st */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x04u)); /* bset.b #2 */
    if (rd8(m, F3_A5 - 0x144Bu) & 0x04u)                        /* 0x13F410 btst.b #2/bne */
        return;
    wr8(m, F3_A5 + 0x1D1Cu, 0x00u);                             /* 0x13F41A sf */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xDFu)); /* bclr.b #5 */
    /* 0x13F424 bra.w 0x13F2CA: next frame */
}
