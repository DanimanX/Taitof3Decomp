/* Darius Gaiden (Taito F3) — coin/service dev-board credit stepper.
 * ROM: 0x000E00  DevBoard_ServiceTask
 * ROM: 0x000ECA  Dev_StepChannel   (bit test on d3, shared by both channels)
 * ROM: 0x000F04  Dev_PhaseTimer
 * ROM: 0x000F1C  Dev_ServiceBlink
 *
 * 0x000E00 (task body; it runs when bit0 of io_ch[1].cur, 0x402220, is set -
 * 0x000E00 btst.b #0,-$5de0(a5) ; 0x000E06 beq 0x000F54):
 *   if (credit > 9)   -> 0x000F3E (error/CRC path, not modelled -> return 0)
 *   bsr 0x000FA8 (pending coin-sound queue, Fn_000FA8)
 *   d1 = io_ch[1].cur (0x402220) ; d3 = ~io_ch[1].released (0x402223)
 *   step ch0: a0=0x400096 a2=0x4000A4 a4=0x400092 d2=4
 *   step ch1: a0=0x40009A a2=*(0x40009E) a4=0x4000A8 d2=5
 *   if (!(d3 & 2) || credit >= 9) skip; else credit++, frame++, queue flag
 *     ..., jsr 0x000B8C (sound cmd 0xE5,0xFF - deferred through the 0x0BF6
 *     queue, only visible when the box's command ring is free)
 *   if (enable 0x402239): service-blink(ch0, out 0x402234, bit 2) and
 *     service-blink(ch1, out 0x402234, bit 3); each only if chan[1] != 0
 *   if (enable): commit the two output bits as credit < 9 (set) / >= 9 (clear)
 */
#include "devboard.h"
#include "../io/input.h"

/* 0x000E18/0x000E1C read channel 1's sampled and released bytes. */
#define DEV_IO1_CUR      (F3_A5 - 0x5DE0u) /* 0x402220 io_ch[1].cur */
#define DEV_IO1_RELEASED (F3_A5 - 0x5DDDu) /* 0x402223 io_ch[1].released */

#define DEV_CH0_A4    (F3_A5 - 0x7F6Eu) /* 0x400092, 0x000E2C lea */
#define DEV_CH_STATE  0x0u /* A5-0x7F6A / 0x7F66 +0 */
#define DEV_CH_ACCEPT 0x1u
#define DEV_CH_BLINK  0x2u
#define DEV_PHASE_MAX 0x1Eu

typedef struct { uint32_t a0; uint32_t a2; uint32_t a4; int d2; } dev_call_t;

/* 0x000F04: the phase timer tests bit d2 of the caller's d1 (io_ch[1].cur).
 * Returns -1 when it reaches 0x000F3E (timer past 0x1E, service enabled). */
static int dev_phase_timer(const f3mem_t *m, uint32_t a0, int d2, uint8_t d1)
{
    if ((d1 >> d2) & 1) {                   /* btst.l d2,d1 -> bne 0x000F18 */
        wr8(m, a0 + DEV_CH_STATE, 0);
        return 0;
    }
    if (!rd8(m, DEV_ENABLE)) {              /* tst.b A5-0x5DC7 -> beq 0x000F18 */
        wr8(m, a0 + DEV_CH_STATE, 0);
        return 0;
    }
    uint8_t v = (uint8_t)(rd8(m, a0 + DEV_CH_STATE) + 1u);
    wr8(m, a0 + DEV_CH_STATE, v);
    return v > DEV_PHASE_MAX ? -1 : 0;      /* 0x000F14 bhi 0x000F3E: error path */
}

int Dev_StepChannel(const f3mem_t *m, uint32_t a0, uint32_t a2, uint32_t a4, int d2,
                    uint8_t d1, uint8_t d3)
{
    if (dev_phase_timer(m, a0, d2, d1) < 0) return -1;
    if (!((d3 >> (uint32_t)d2) & 1u)) return 0; /* btst.l d2,d3 tests longword bit d2 */
    wr8(m, a0 + DEV_CH_ACCEPT, (uint8_t)(rd8(m, a0 + DEV_CH_ACCEPT) + 1u));
    /* jsr 0x000B76: queue sound command 0xE4, 0xFF (deferred, see header) */
    uint16_t hit = (uint16_t)(rd16(m, a4) + 1u);
    wr16(m, a4, hit);
    if (rd16(m, a2) != hit) return 0;
    wr16(m, a4, 0);
    wr8(m, F3_A5 - 0x7F51u, 1);
    wr8(m, F3_A5 - 0x7F52u, (uint8_t)(rd8(m, F3_A5 - 0x7F52u) + 1u));
    uint32_t credit = (uint32_t)rd16(m, DEV_CREDIT) + rd16(m, a2 + 2u);
    if (credit > 9) credit = 9;
    wr16(m, DEV_CREDIT, (uint16_t)credit);
    return 0;
}

static void dev_service_blink(const f3mem_t *m, uint32_t a0, uint32_t a4, int d1)
{
    wr8(m, a0 + DEV_CH_BLINK, (uint8_t)(rd8(m, a0 + DEV_CH_BLINK) + 1u));
    uint8_t d0 = (uint8_t)(rd8(m, a0 + DEV_CH_BLINK) - 5u);
    if ((int8_t)d0 < 0) {                   /* bge fails: < 5 while counting */
        uint8_t o = rd8(m, a4);
        wr8(m, a4, (uint8_t)(o | (1u << d1)));
        return;
    }
    d0 = (uint8_t)(d0 - 5u);
    if ((int8_t)d0 < 0) {                   /* second window 5..9 */
        uint8_t o = rd8(m, a4);
        wr8(m, a4, (uint8_t)(o & ~(1u << (uint32_t)(d1 & 7))));
        return;
    }
    wr8(m, a0 + DEV_CH_BLINK, 0);           /* >= 10: restart, one less press */
    wr8(m, a0 + DEV_CH_ACCEPT, (uint8_t)(rd8(m, a0 + DEV_CH_ACCEPT) - 1u));
}

int DevBoard_ServiceTask(const f3mem_t *m)
{
    /* 0x000E18 move.b -$5de0(a5),d1 / 0x000E1C move.b -$5ddd(a5),d3 ; not.b d3 */
    uint8_t d1 = rd8(m, DEV_IO1_CUR);
    uint8_t d3 = (uint8_t)~rd8(m, DEV_IO1_RELEASED);

    if (!(d1 & 1u)) return 0;               /* btst.b #0,-$5de0(a5) -> beq 0x000F54 */

    if (rd16(m, DEV_CREDIT) > 9u) return 0; /* bhi 0x000F3E */

    uint32_t win1 = rd32(m, DEV_WIN1_PTR);  /* 0x000E3E movea.l -$7f62(a5),a4 */
    if (Dev_StepChannel(m, DEV_CH0, DEV_WIN0, DEV_CH0_A4, 4, d1, d3) < 0) return 0;
    if (Dev_StepChannel(m, DEV_CH1, win1, DEV_WIN1, 5, d1, d3) < 0) return 0;

    if ((d3 & 0x02u) && rd16(m, DEV_CREDIT) < 9u) {
        wr16(m, DEV_CREDIT, (uint16_t)(rd16(m, DEV_CREDIT) + 1u));
        wr16(m, F3_A5 - 0x5D54u, (uint16_t)(rd16(m, F3_A5 - 0x5D54u) + 1u));
        wr8(m, F3_A5 - 0x7F51u, 1);
        wr8(m, F3_A5 - 0x7F52u, (uint8_t)(rd8(m, F3_A5 - 0x7F52u) + 1u));
        /* 0x000B8C: queue sound commands 0xE5, 0xFF */
    }

    if (rd8(m, DEV_ENABLE)) {
        if (rd8(m, DEV_CH0 + DEV_CH_ACCEPT))
            dev_service_blink(m, DEV_CH0, IO_OUT_4A0004, 2);
        if (rd8(m, DEV_CH1 + DEV_CH_ACCEPT))
            dev_service_blink(m, DEV_CH1, IO_OUT_4A0004, 3);
    }

    if (rd8(m, DEV_ENABLE)) {
        uint8_t o = rd8(m, IO_OUT_4A0004);
        if (rd16(m, DEV_CREDIT) >= 9u) o &= (uint8_t)~0x03u;
        else                           o |= 0x03u;
        wr8(m, IO_OUT_4A0004, o);
    }
    return 1;
}
