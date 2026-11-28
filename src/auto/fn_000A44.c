/* ROM 0x000A44: queue one serial command built from a byte selector, three
 * words and a word count (stack args, A6 frame is not copied).
 * The byte at entry SP+4 OR-ed with 0xC8 goes through the serial queue
 * (0x000C8A); the three words at ((SP+6)) +0/+2/+4 are each formatted as one
 * decimal byte via 0x000C30; then two 0 bytes are queued, the signed word at
 * entry SP+0xA is formatted as two decimal bytes via 0x000C30, and 0xFF
 * terminates the command through 0x000C8A. */
#include "fn_auto.h"
#include "fn_call_frames.h"

static void queue_byte(const f3mem_t *m, f3regs_t *r, uint8_t byte, uint32_t ret)
{
    r->a[7] -= 2u;                                    /* move.b D0b,-(SP) */
    wr8(m, r->a[7], byte);
    push_long(m, r, ret);
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                    /* addq.l #2,SP */
}

static void queue_number(const f3mem_t *m, f3regs_t *r, uint32_t value,
                         uint16_t count, uint32_t ret)
{
    push_word(m, r, count);                           /* move.w #n,-(SP) */
    push_long(m, r, value);                           /* move.l D0,-(SP) */
    push_long(m, r, ret);
    Fn_000C30(m, r);
    r->a[7] += 6u;                                    /* addq.l #6,SP */
}

int Fn_000A44(const f3mem_t *m, f3regs_t *r)
{
    /* 0x000A44 link.w A6,#0 -> A6 = entry SP-4, so (0x8,A6) = SP+4,
     * (0xa,A6) = SP+6 and (0xe,A6) = SP+0xA in the caller's frame. */
    uint32_t sp = r->a[7];
    uint32_t words = rd32(m, sp + 6u);
    uint8_t cmd = (uint8_t)(rd8(m, sp + 4u) | 0xC8u);          /* 0x000A48..0x000A4C */
    uint32_t cnt = (uint32_t)(int32_t)(int16_t)rd16(m, sp + 0x0Au); /* 0x000AA6..0x000AAA */

    r->d[0] = (r->d[0] & 0xFFFFFF00u) | cmd;
    queue_byte(m, r, cmd, 0x000A56u);                          /* 0x000A50..0x000A56 */

    static const uint32_t rets[3] = { 0x000A68u, 0x000A7Cu, 0x000A90u };
    for (unsigned i = 0; i < 3u; i++) {
        r->d[0] = set_word(r->d[0], rd16(m, words + 2u * i));  /* 0x000A5C/0x000A6E/0x000A82 */
        queue_number(m, r, r->d[0], 1u, rets[i]);              /* 0x000A5E..0x000A68 etc. */
    }

    r->d[0] &= 0xFFFFFF00u;                                    /* 0x000A92 clr.b D0b */
    queue_byte(m, r, 0u, 0x000A9Au);                           /* 0x000A94..0x000A9A */
    r->d[0] &= 0xFFFFFF00u;                                    /* 0x000A9C clr.b D0b */
    queue_byte(m, r, 0u, 0x000AA4u);                           /* 0x000A9E..0x000AA4 */

    r->d[0] = cnt;
    queue_number(m, r, cnt, 2u, 0x000AB6u);                    /* 0x000AAC..0x000AB6 */

    queue_byte(m, r, 0xFFu, 0x000AC0u);                        /* 0x000AB8..0x000AC0 */
    return 0;                                                  /* 0x000AC2 unlk ; rts */
}
