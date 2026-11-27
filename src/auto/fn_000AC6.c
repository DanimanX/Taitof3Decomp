#include "fn_auto.h"
/* ROM 0x000AC6: queue a 0xD0-family serial command with a ten-character field. */
#include "fn_call_frames.h"

/* 0x000AEC / 0x000B0E: encode one character as a packed decimal byte. */
static void queue_character(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_word(m, r, 1);
    push_long(m, r, r->d[0]);
    push_long(m, r, ret);
    Fn_000C30(m, r);
    r->a[7] += 6;
}

int Fn_000AC6(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    uint8_t cmd = rd8(m, r->a[6] + 8) | 0xD0u;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | cmd;
    r->a[7] -= 2;
    wr8(m, r->a[7], cmd);
    push_long(m, r, 0x000AD8);
    Fn_000C8A(m, r);
    r->a[7] += 2;

    r->d[1] = 9;
    r->a[0] = rd32(m, r->a[6] + 10);
    do {
        uint8_t ch = rd8(m, r->a[0]++);
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | ch;
        if (!ch) {
            /* 0x000B04: pad the rest of the field with spaces. */
            do {
                r->d[0] = (uint32_t)(int32_t)(int16_t)((r->d[0] & 0xFF00u) | 0x20u);
                push_long(m, r, r->d[1]);
                queue_character(m, r, 0x000B18);
                r->d[1] = rd32(m, r->a[7]);
                r->a[7] += 4;
                r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] - 1u));
            } while ((uint16_t)r->d[1] != 0xFFFFu);
            break;
        }
        r->d[0] = (uint32_t)(int32_t)(int16_t)r->d[0];
        push_long(m, r, r->a[0]);
        push_long(m, r, r->d[1]);
        queue_character(m, r, 0x000AF6);
        r->d[1] = rd32(m, r->a[7]);
        r->a[0] = rd32(m, r->a[7] + 4);
        r->a[7] += 8;
        r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] - 1u));
    } while ((uint16_t)r->d[1] != 0xFFFFu);

    r->a[7] -= 2;
    wr8(m, r->a[7], 0xFF);
    push_long(m, r, 0x000B2A);
    Fn_000C8A(m, r);
    r->a[7] += 2;
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8;
    return 0;
}
