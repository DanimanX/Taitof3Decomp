#include "fn_auto.h"
/* ROM 0x000DFA: update coin channels, credits and the service outputs. */
#include "fn_call_frames.h"

/* 0x000B76 / 0x000B8C: queue the command and its terminator. */
static void queue_credit(const f3mem_t *m, f3regs_t *r, uint8_t cmd,
                         uint32_t ret, uint32_t first_ret)
{
    push_long(m, r, ret);
    r->a[7] -= 2;
    wr8(m, r->a[7], cmd);
    push_long(m, r, first_ret);
    Fn_000C8A(m, r);
    r->a[7] += 2;
    r->a[7] -= 2;
    wr8(m, r->a[7], 0xFF);
    push_long(m, r, first_ret + 10u);
    Fn_000C8A(m, r);
    r->a[7] += 6;                       /* byte argument and wrapper RTS */
}

/* 0x000ECA: step one channel and award its credits on a prescale match. */
static void step_channel(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t a5 = r->a[5];
    push_long(m, r, ret);
    push_long(m, r, 0x000ECE);
    Fn_000F04(m, r);                    /* terminal paths checked at entry */
    if (r->d[3] & (1u << r->d[2])) {
        wr8(m, r->a[0] + 1, rd8(m, r->a[0] + 1) + 1u);
        queue_credit(m, r, 0xE4, 0x000EDA, 0x000B7E);
        wr16(m, r->a[4], rd16(m, r->a[4]) + 1u);
        uint16_t prescale = rd16(m, r->a[2]);
        r->d[0] = set_word(r->d[0], prescale);
        if (prescale == rd16(m, r->a[4])) {
            wr16(m, r->a[4], 0);
            wr8(m, a5 - 0x7F51u, 1);
            wr8(m, a5 - 0x7F52u, rd8(m, a5 - 0x7F52u) + 1u);
            uint16_t credit = rd16(m, a5 - 0x7F70u);
            credit += rd16(m, r->a[2] + 2);
            r->d[0] = (int16_t)credit > 9 ? 9u : set_word(r->d[0], credit);
            wr16(m, a5 - 0x7F70u, (uint16_t)r->d[0]);
        }
    }
    r->a[7] += 4;
}

/* 0x000F1C: pulse the channel's service output for five frames. */
static void service_blink(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    uint32_t chan = r->a[0];
    uint8_t counter = rd8(m, chan + 2) + 1u;
    wr8(m, chan + 2, counter);
    int result = (int8_t)counter - 5;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)result;
    uint8_t mask = (uint8_t)(1u << r->d[1]);
    if (result < 0) {
        wr8(m, r->a[4], rd8(m, r->a[4]) | mask);
    } else {
        result -= 5;
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)result;
        if (result < 0) {
            wr8(m, r->a[4], rd8(m, r->a[4]) & (uint8_t)~mask);
        } else {
            wr8(m, chan + 2, 0);
            wr8(m, chan + 1, rd8(m, chan + 1) - 1u);
        }
    }
    r->a[7] += 4;
}

int Fn_000DFA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5], enable = a5 - 0x5DC7u;
    uint32_t credit = a5 - 0x7F70u;
    uint32_t ch0 = a5 - 0x7F6Au, ch1 = a5 - 0x7F66u;
    uint8_t input = rd8(m, a5 - 0x5DE0u);
    /* 0x000F3E / 0x000F54 enter the hardware halt at 0x01010A. */
    if (rd8(m, enable) &&
        (!(input & 1u) || rd16(m, credit) > 9u ||
         (!(input & 0x10u) && (uint8_t)(rd8(m, ch0) + 1u) > 30u) ||
         (!(input & 0x20u) && (uint8_t)(rd8(m, ch1) + 1u) > 30u)))
        return -1;

    push_long(m, r, 0x000E18);
    Fn_000FA8(m, r);
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | rd8(m, a5 - 0x5DE0u);
    r->d[3] = (r->d[3] & 0xFFFFFF00u) | (uint8_t)~rd8(m, a5 - 0x5DDDu);
    r->d[2] = 4;
    r->a[0] = ch0;
    r->a[2] = a5 - 0x7F5Cu;
    r->a[4] = a5 - 0x7F6Eu;
    step_channel(m, r, 0x000E34);
    r->d[2] = 5;
    r->a[0] = ch1;
    r->a[2] = a5 - 0x7F58u;
    r->a[4] = rd32(m, a5 - 0x7F62u);
    step_channel(m, r, 0x000E46);

    if ((r->d[3] & 2u) && rd16(m, credit) < 9u) {
        wr16(m, credit, rd16(m, credit) + 1u);
        wr16(m, a5 - 0x7F54u, rd16(m, a5 - 0x7F54u) + 1u);
        wr8(m, a5 - 0x7F51u, 1);
        wr8(m, a5 - 0x7F52u, rd8(m, a5 - 0x7F52u) + 1u);
        queue_credit(m, r, 0xE5, 0x000E6A, 0x000B94);
    }
    uint32_t output = a5 - 0x5DCCu;
    if (rd8(m, enable)) {
        r->a[0] = ch0;
        if (rd8(m, ch0 + 1)) {
            r->a[4] = output;
            r->d[1] = 2;
            service_blink(m, r, 0x000E86);
        }
        r->a[0] = ch1;
        if (rd8(m, ch1 + 1)) {
            r->a[4] = output;
            r->d[1] = 3;
            service_blink(m, r, 0x000E9E);
        }
    }
    if (rd8(m, enable)) {
        if (rd16(m, credit) < 9u) {
            wr8(m, output, rd8(m, output) | 1u);
            wr8(m, output, rd8(m, output) | 2u);
        } else {
            wr8(m, output, rd8(m, output) & 0xFEu);
            wr8(m, output, rd8(m, output) & 0xFDu);
        }
    }
    r->a[7] += 4;
    return 0;
}
