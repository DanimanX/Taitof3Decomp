#include "fn_auto.h"
/* ROM 0x01033C: update the credit HUD and latch the frame's credit flags. */
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"
#include "../video/sprite.h"

/* 0x005666 and 0x005690 save their LINK/MOVEM frames and return D0. */
static void draw_string(const f3mem_t *m, f3regs_t *r, uint32_t src,
                        uint16_t attr, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, 0x61CE3Eu);
    push_long(m, r, src);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    r->d[0] = Text_DrawString(m, src, 0x61CE3Eu, attr);
    r->sr &= (uint16_t)~0x10u;                       /* 0x00567A add.w D7,D7 */
    r->a[7] = sp;
}

static void draw_credit_digit(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    push_word(m, r, 7u);
    push_long(m, r, 0x61CE4Cu);
    push_word(m, r, 1u);
    push_long(m, r, r->d[0]);
    push_long(m, r, 0x0103CEu);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[0] = Gfx_DrawHex(m, r->d[0], 1u, 0x61CE4Cu, 7u);
    r->sr |= 0x10u;                                  /* 0x0056B6 subq.w #1 from zero */
    r->a[7] = sp;
}

/* 0x001016: preserve D0's upper bits on the word/byte paths. The
 * SUB.W/BLE compares the signed operands, including the overflow case. */
static void coins_needed(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t price = rd16(m, a5 - 0x7F5Eu);
    r->d[0] = set_word(r->d[0], price);
    if (price == 0u) {
        if (rd16(m, a5 - 0x7F70u) == 0u)
            r->d[0] |= 0xFFu;
    } else {
        uint16_t paid = rd16(m, a5 - 0x7F6Eu);
        r->d[0] = set_word(r->d[0], (uint16_t)(price - paid));
        if ((int16_t)price <= (int16_t)paid || rd16(m, a5 - 0x7F70u) != 0u)
            r->d[0] = 0u;
    }
    r->a[7] += 4u;
}

static void draw_hud_010352(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    if (rd8(m, a5 - 0x5DC1u) != 0u) {
        uint16_t attr = (rd8(m, a5 - 0x5D53u) & 4u) ? 7u : 8u;
        r->d[0] = set_word(r->d[0], attr);
        draw_string(m, r, 0x010402u, attr, 0x0103FCu);
        return;
    }

    if ((rd8(m, a5 - 0x144Bu) & 0x80u) && rd16(m, 0x1FFFFEu) == 2u) {
        push_long(m, r, 0x010376u);
        coins_needed(m, r);
        if ((uint8_t)r->d[0] == 0u) {
            draw_string(m, r, 0x010416u, 7u, 0x010392u);
            return;
        }
    }

    draw_string(m, r, 0x01040Cu, 7u, 0x0103B0u);
    r->d[0] = set_word(r->d[0], rd16(m, a5 - 0x7F70u));
    draw_credit_digit(m, r);
}

int Fn_01033C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    /* 0x010432 enters test mode via scheduler traps and never returns to
     * this caller. The kernel reset helper lacks these trap effects. */
    if ((rd8(m, a5 - 0x5DE3u) & 0x18u) != 0x18u)
        return -1;

    push_long(m, r, 0x010340u);                      /* 0x010420 -> 0x010478 */
    r->a[7] += 4u;
    push_long(m, r, 0x010344u);
    draw_hud_010352(m, r);
    r->a[7] += 4u;
    wr8(m, a5 - 0x144Bu, rd8(m, a5 - 0x144Cu));
    wr8(m, a5 - 0x144Cu, (uint8_t)(rd8(m, a5 - 0x144Cu) & 0x7Fu));
    r->a[7] += 4u;                                  /* 0x010350 RTS */
    return 0;
}
