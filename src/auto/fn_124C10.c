#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../video/sprite.h"

/* ROM 0x124C10: in-game camera/position diagnostics overlay (test build).
 *
 * Clears A5+0x3318 and then, with A5-0x5DD8 bit 6 SET, edits the flag word at
 * A5+0x30EE: the selection byte A5+0x3271 is adjusted by 0x19FFF0 (range
 * [0..0xF], step 1, buttons A5-0x5DD8) and, unless both A5-0x5DD7 bits 0/1
 * are set, its bit is toggled in the flag word. The word is then drawn as 16
 * L/H bits at 0x61C184 (0x0056E0) and the selected bit again at
 * 0x61C000 + 2*(sel+2) + 3*0x80 with attribute 8. Without bit 6 the four
 * direction bits step A5+0x30D2 / A5+0x30D6. Both paths print A5+0x30D2 and
 * A5+0x30D6 as 4 hex digits at 0x61C204 / 0x61C20E and A5+0x30DA as 4 decimal
 * digits at 0x61C218, then re-arm 0x124C10 (0x124C08 thunk). */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0056E0: the requested bits, high to low, as L or H with attribute
 * `attr`. */
static uint32_t draw_bits(const f3mem_t *m, uint32_t value, uint16_t count,
                          uint32_t dst, uint16_t attr)
{
    while ((int16_t)--count >= 0) {
        wr8(m, dst++, (uint8_t)(attr * 2u));
        wr8(m, dst++, (value & (1u << (count & 31u))) ? 0x48u : 0x4Cu);
    }
    return dst;
}

/* Bit-field offset addressing: a signed bit offset counts from the MSB of the
 * byte at the base address. */
static uint32_t bit_address(uint32_t base, uint32_t offset)
{
    return base + (uint32_t)((int32_t)offset >> 3);
}

int Fn_124C10(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t flags = a5 + 0x30EEu;

    wr16(m, a5 + 0x3318u, 0u);                                   /* 0x124C10 */

    if ((rd8(m, a5 - 0x5DD8u) & 0x40u) == 0u) {                  /* 0x124C14 */
        /* 0x124C1E: nudge the two camera words with the active-low buttons. */
        if ((rd8(m, a5 - 0x5DD8u) & 0x08u) == 0u)
            wr16(m, a5 + 0x30D2u, (uint16_t)(rd16(m, a5 + 0x30D2u) + 1u));
        if ((rd8(m, a5 - 0x5DD8u) & 0x04u) == 0u)
            wr16(m, a5 + 0x30D2u, (uint16_t)(rd16(m, a5 + 0x30D2u) - 1u));
        if ((rd8(m, a5 - 0x5DD8u) & 0x01u) == 0u)
            wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) - 1u));
        if ((rd8(m, a5 - 0x5DD8u) & 0x02u) == 0u)
            wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) + 1u));
    } else {
        /* 0x124C5A: move the flag selection and redraw the bit row. */
        r->d[0] = rd8(m, a5 + 0x3271u);                          /* moveq #0 / move.b */
        push_long(m, r, r->a[0]);                                /* 0x124C60 movem {A0} */
        push_word(m, r, 1u);                                     /* 0x124C64 */
        push_word(m, r, 0x000Fu);                                /* 0x124C68 */
        push_word(m, r, 0u);                                     /* 0x124C6C */
        push_word(m, r, (uint16_t)r->d[0]);                      /* 0x124C70 */
        r->d[0] = setw(r->d[0], (uint16_t)(int16_t)(int8_t)rd8(m, a5 - 0x5DD8u));
        push_word(m, r, (uint16_t)r->d[0]);                      /* 0x124C78 */
        push_long(m, r, 0x00124C80u);
        Fn_19FFF0(m, r);                                         /* 0x124C7A */
        r->a[7] += 10u;                                          /* 0x124C80 */
        r->a[0] = rd32(m, r->a[7]);                              /* 0x124C84 movem (SP)+,{A0} */
        r->a[7] += 4u;
        wr8(m, a5 + 0x3271u, (uint8_t)r->d[0]);                  /* 0x124C88 */

        if ((rd8(m, a5 - 0x5DD7u) & 0x03u) != 0x03u) {           /* 0x124C8C */
            /* 0x124CA0 bfchg (0x30EE,A5){D0:1} */
            uint32_t byte = bit_address(flags, r->d[0]);
            wr8(m, byte, (uint8_t)(rd8(m, byte) ^ (0x80u >> (r->d[0] & 7u))));
        }

        r->d[0] = rd16(m, flags);                                /* 0x124CA6 bfextu */
        r->d[0] = draw_bits(m, r->d[0], 0x0010u, 0x0061C184u, 7u); /* 0x124CBC */

        r->d[0] = rd8(m, a5 + 0x3271u);                          /* 0x124CC6 moveq/move.b */
        r->d[2] = (uint32_t)((rd8(m, bit_address(flags, r->d[0]))
                              >> (7u - (r->d[0] & 7u))) & 1u);   /* 0x124CCC bfextu */
        r->d[0] = setw(r->d[0], (uint16_t)((uint16_t)r->d[0] + 2u)); /* 0x124CD2 */
        {
            uint32_t d0 = r->d[0];
            r->d[1] = setw(r->d[1], 0x0003u);                    /* 0x124CD4 move.w #3,D1w */
            uint16_t idx = (uint16_t)((uint16_t)((uint16_t)d0 + (uint16_t)d0)
                                      + (uint16_t)((uint16_t)r->d[1] << 7)); /* 0x124CE0..0x124CE4 */
            uint32_t dst = 0x0061C000u + (uint32_t)(int32_t)(int16_t)idx; /* 0x124CE6..0x124CE8 */
            (void)draw_bits(m, r->d[2], 1u, dst, 8u);            /* 0x124CF6 */
            r->d[0] = d0;                                        /* 0x124D00 movem (SP)+ */
        }
    }

    /* 0x124D04: the three readouts. */
    r->d[0] = setw(r->d[0], rd16(m, a5 + 0x30D2u));
    r->d[0] = Gfx_DrawHex(m, r->d[0], 4u, 0x0061C204u, 7u);      /* 0x124D18 */
    r->d[0] = setw(r->d[0], rd16(m, a5 + 0x30D6u));
    r->d[0] = Gfx_DrawHex(m, r->d[0], 4u, 0x0061C20Eu, 7u);      /* 0x124D36 */
    r->d[0] = 0u;                                                /* 0x124D40 moveq #0,D0 */
    r->d[0] = setw(r->d[0], rd16(m, a5 + 0x30DAu));
    r->d[0] = Gfx_DrawDecimal(m, r->d[0], 4u, 0x0061C218u, 7u);  /* 0x124D56 */

    wr32(m, r->a[0], 0x00124C10u);                               /* 0x124D60 -> 0x124C08 */
    return 0;
}
