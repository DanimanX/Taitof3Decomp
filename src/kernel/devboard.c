/* Darius Gaiden (Taito F3) — coin/service dev-board credit stepper.
 * ROM: 0x000E00  DevBoard_ServiceTask
 * ROM: 0x000ECA  Dev_StepChannel   (bit test on d3, shared by both channels)
 * ROM: 0x000F04  Dev_PhaseTimer
 * ROM: 0x000F1C  Dev_ServiceBlink
 *
 * 0x000E00 (task run while the active-low service switch, io_ch[0].cur bit0 at
 * 0x40221C, reads 0):
 *   if (credit > 9)   -> 0x000F3E (error/CRC path, not modelled -> return 0)
 *   bsr 0x000FA8 ; d1 = cur ; d3 = ~prev   (D3 = cur OR prev, see 0x001B8E)
 *   step ch0: a0=0x400096 a2=0x4000A4 a4=0x40009E d2=4
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

#define DEV_CH_STATE  0x0u /* A5-0x7F6A / 0x7F66 +0 */
#define DEV_CH_ACCEPT 0x1u
#define DEV_CH_BLINK  0x2u
#define DEV_PHASE_MAX 0x1Eu

typedef struct { uint32_t a0; uint32_t a2; uint32_t a4; int d2; } dev_call_t;

static uint8_t dev_d3; /* cur | prev of io_ch[0], kept from 0x001A78 */
static int dev_fault;

static void dev_phase_timer(const f3mem_t *m, uint32_t a0, int d2)
{
    if ((dev_d3 >> d2) & 1) {               /* btst.l d2,d1 -> bne 0x000F18 */
        wr8(m, a0 + DEV_CH_STATE, 0);
        return;
    }
    if (!rd8(m, DEV_ENABLE)) {              /* tst.b A5-0x5DC7 -> beq 0x000F18 */
        wr8(m, a0 + DEV_CH_STATE, 0);
        return;
    }
    uint8_t v = (uint8_t)(rd8(m, a0 + DEV_CH_STATE) + 1u);
    wr8(m, a0 + DEV_CH_STATE, v);
    if (v > DEV_PHASE_MAX) { dev_fault = 1; } /* 0x000F3E: error path */
}

void Dev_StepChannel(const f3mem_t *m, uint32_t a0, uint32_t a2, uint32_t a4, int d2)
{
    dev_phase_timer(m, a0, d2);
    if (dev_fault) return;
    if (!((dev_d3 >> (uint32_t)d2) & 1u)) return; /* btst.l d2,d3 tests longword bit d2 */
    wr8(m, a0 + DEV_CH_ACCEPT, (uint8_t)(rd8(m, a0 + DEV_CH_ACCEPT) + 1u));
    /* jsr 0x000B76: queue sound command 0xE4, 0xFF (deferred, see header) */
    uint16_t hit = (uint16_t)(rd16(m, a4) + 1u);
    wr16(m, a4, hit);
    if (rd16(m, a2) != hit) return;
    wr16(m, a4, 0);
    wr8(m, F3_A5 - 0x7F51u, 1);
    wr8(m, F3_A5 - 0x7F52u, (uint8_t)(rd8(m, F3_A5 - 0x7F52u) + 1u));
    uint32_t credit = (uint32_t)rd16(m, DEV_CREDIT) + rd16(m, a2 + 2u);
    if (credit > 9) credit = 9;
    wr16(m, DEV_CREDIT, (uint16_t)credit);
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
    dev_d3 = (uint8_t)(rd8(m, IO_BASE) | rd8(m, IO_BASE + 2u));
    dev_fault = 0;
    if (!(rd8(m, IO_BASE) & 1u)) return 0;  /* btst #0,cur -> beq 0x000F54 */

    if (rd16(m, DEV_CREDIT) > 9u) return 0; /* bhi 0x000F3E */

    uint32_t win1 = rd32(m, DEV_WIN1_PTR);
    Dev_StepChannel(m, DEV_CH0, DEV_WIN0, DEV_WIN1_PTR, 4);
    if (dev_fault) return 0;
    Dev_StepChannel(m, DEV_CH1, win1, DEV_WIN1, 5);
    if (dev_fault) return 0;

    if ((dev_d3 & 0x02u) && rd16(m, DEV_CREDIT) < 9u) {
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
