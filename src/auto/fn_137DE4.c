/* ROM 0x137DE4: in-game camera/flag diagnostics overlay (test build).
 * Clears A5+0x3318. With A5-0x5DD8 bit 6 SET it edits the 28-bit flag field at
 * A5+0x30EE: the selection byte A5+0x32DB is adjusted by 0x19FFF0 (range
 * [0..0x1B], step 1, buttons = sign-extended A5-0x5DD8) and, unless both
 * A5-0x5DD7 bits 0 and 1 are set, its bit is toggled. The field is drawn as
 * 28 L/H bits at 0x61C184 (0x0056E0) and the selected bit again at
 * 0x61C000 + 2*(sel+2) + 3*0x80 with attribute 8.
 * Without bit 6 the four direction bits (active low, and counted 8 at a time
 * when A5-0x5DD8 bit 5 is also clear) step A5+0x30D2 and A5+0x30D6 by 1/7.
 * Both paths bump A5+0x32D6 unless A5-0x5DD7 bit 4 is set, print A5+0x30D2 and
 * A5+0x30D6 as 4 hex digits at 0x61C204 / 0x61C20E, A5+0x30DA as 4 decimal
 * digits at 0x61C218 and A5+0x32D6 as 4 hex digits at 0x61C284, then re-arm
 * 0x137DE4 (0x137DDC thunk). */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../video/sprite.h"

/* ROM 0x0056E0: the requested bits, high to low, as L or H with attribute
 * `attr` (doubled into the first byte of each pair). */
static uint32_t draw_bits(const f3mem_t *m, uint32_t value, uint16_t count,
                          uint32_t dst, uint16_t attr)
{
    while ((int16_t)--count >= 0) {                             /* 0x0056FA */
        wr8(m, dst++, (uint8_t)(attr * 2u));                    /* 0x0056FE */
        wr8(m, dst++, (value & (1u << (count & 31u))) ? 0x48u : 0x4Cu); /* 0x005700..0x00570A */
    }
    return dst;                                                 /* 0x005710 move.l A0,D0 */
}

/* Bit-field addressing on memory: a signed bit offset counts from the MSB of
 * the byte at the base address. */
static uint32_t bit_address(uint32_t base, uint32_t offset)
{
    return base + (uint32_t)((int32_t)offset >> 3);
}

int Fn_137DE4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t flags = a5 + 0x30EEu;
    uint8_t buttons = rd8(m, a5 - 0x5DD8u);

    wr16(m, a5 + 0x3318u, 0u);                                   /* 0x137DE4 clr.w */

    if ((buttons & 0x40u) == 0u) {                               /* 0x137DE8 bne 0x137E66 */
        /* 0x137DF2..0x137E62: camera nudges; each direction adds 1 and, while
         * A5-0x5DD8 bit 5 is also clear, 7 more (8 per frame). */
        if ((buttons & 0x08u) == 0u) {                           /* 0x137DF2 btst #3 */
            wr16(m, a5 + 0x30D2u, (uint16_t)(rd16(m, a5 + 0x30D2u) + 1u)); /* 0x137DFC */
            if ((buttons & 0x20u) == 0u)                         /* 0x137E00 btst #5 */
                wr16(m, a5 + 0x30D2u, (uint16_t)(rd16(m, a5 + 0x30D2u) + 7u)); /* 0x137E0A */
        }
        if ((buttons & 0x04u) == 0u) {                           /* 0x137E0E btst #2 */
            wr16(m, a5 + 0x30D2u, (uint16_t)(rd16(m, a5 + 0x30D2u) - 1u)); /* 0x137E18 */
            if ((buttons & 0x20u) == 0u)                         /* 0x137E1C btst #5 */
                wr16(m, a5 + 0x30D2u, (uint16_t)(rd16(m, a5 + 0x30D2u) - 7u)); /* 0x137E26 */
        }
        if ((buttons & 0x01u) == 0u) {                           /* 0x137E2A btst #0 */
            wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) - 1u)); /* 0x137E34 */
            if ((buttons & 0x20u) == 0u)                         /* 0x137E38 btst #5 */
                wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) - 7u)); /* 0x137E42 */
        }
        if ((buttons & 0x02u) == 0u) {                           /* 0x137E46 btst #1 */
            wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) + 1u)); /* 0x137E50 */
            if ((buttons & 0x20u) == 0u)                         /* 0x137E54 btst #5 */
                wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) + 7u)); /* 0x137E5E */
        }
    } else {
        /* 0x137E66: move the flag selection 0x00..0x1B through 0x19FFF0. */
        r->d[0] = 0u;                                            /* 0x137E66 moveq #0,D0 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, a5 + 0x32DBu); /* 0x137E68 move.b */
        push_long(m, r, r->a[0]);                                /* 0x137E6C movem {A0} */
        push_word(m, r, 1u);                                     /* 0x137E70 */
        push_word(m, r, 0x001Bu);                                /* 0x137E74 */
        push_word(m, r, 0u);                                     /* 0x137E78 */
        push_word(m, r, (uint16_t)r->d[0]);                      /* 0x137E7C */
        r->d[0] = (r->d[0] & 0xFFFF0000u)
                | (uint16_t)(int16_t)(int8_t)buttons;            /* 0x137E7E move.b/ext.w */
        push_word(m, r, (uint16_t)r->d[0]);                      /* 0x137E84 */
        push_long(m, r, 0x00137E8Cu);
        Fn_19FFF0(m, r);                                         /* 0x137E86 */
        r->a[7] += 10u;                                          /* 0x137E8C lea (0xA,SP) */
        r->a[0] = rd32(m, r->a[7]);                              /* 0x137E90 movem (SP)+,{A0} */
        r->a[7] += 4u;
        wr8(m, a5 + 0x32DBu, (uint8_t)r->d[0]);                  /* 0x137E94 */

        if ((rd8(m, a5 - 0x5DD7u) & 0x03u) != 0x03u) {           /* 0x137E98 / 0x137EA2 */
            /* 0x137EAC bfchg (0x30EE,A5){D0:1}: toggle bit D0 of the field. */
            uint32_t byte = bit_address(flags, r->d[0]);
            uint32_t bit = 0x80u >> (r->d[0] & 7u);
            wr8(m, byte, (uint8_t)(rd8(m, byte) ^ bit));
        }

        /* 0x137EB2 bfextu (0x30EE,A5){0:0x1C}: the top 28 bits of the field. */
        r->d[0] = rd32(m, flags) >> 4;
        r->d[0] = draw_bits(m, r->d[0], 0x001Cu, 0x0061C184u, 7u); /* 0x137EC8 */

        r->d[0] = 0u;                                            /* 0x137ED2 moveq #0,D0 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, a5 + 0x32DBu); /* 0x137ED4 move.b */
        r->d[2] = (uint32_t)((rd8(m, bit_address(flags, r->d[0]))
                              >> (7u - (r->d[0] & 7u))) & 1u);   /* 0x137ED8 bfextu {D0:1} */
        r->d[0] = (r->d[0] & 0xFFFF0000u)
                | (uint16_t)((uint16_t)r->d[0] + 2u);            /* 0x137EDE addq.w #2 */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | 0x0003u;             /* 0x137EE0 move.w #3,D1w */
        {
            uint16_t idx = (uint16_t)((uint16_t)((uint16_t)r->d[0] + (uint16_t)r->d[0])
                                      + (uint16_t)((uint16_t)r->d[1] << 7)); /* 0x137EEC..0x137EF0 */
            uint32_t dst = 0x0061C000u + (uint32_t)(int32_t)(int16_t)idx; /* 0x137EF2..0x137EF4 */
            (void)draw_bits(m, r->d[2], 1u, dst, 8u);            /* 0x137F02 */
            /* 0x137F0C movem (SP)+,{D0 D1} restores the saved selection and
             * 3; draw_bits leaves D0/D1 alone, so nothing to write back. */
        }
    }

    /* 0x137F10 */
    if ((rd8(m, a5 - 0x5DD7u) & 0x10u) == 0u)                    /* 0x137F10 btst #4 */
        wr16(m, a5 + 0x32D6u, (uint16_t)(rd16(m, a5 + 0x32D6u) + 1u)); /* 0x137F1A */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a5 + 0x30D2u);   /* 0x137F1E */
    r->d[0] = Gfx_DrawHex(m, r->d[0], 4u, 0x0061C204u, 7u);      /* 0x137F32 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a5 + 0x30D6u);   /* 0x137F3C */
    r->d[0] = Gfx_DrawHex(m, r->d[0], 4u, 0x0061C20Eu, 7u);      /* 0x137F50 */
    r->d[0] = set_word(0u, rd16(m, a5 + 0x30DAu));               /* 0x137F5A moveq #0 */
    r->d[0] = Gfx_DrawDecimal(m, r->d[0], 4u, 0x0061C218u, 7u);  /* 0x137F70 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a5 + 0x32D6u);   /* 0x137F7A */
    r->d[0] = Gfx_DrawHex(m, r->d[0], 4u, 0x0061C284u, 7u);      /* 0x137F8E */

    wr32(m, r->a[0], 0x00137DE4u);                               /* 0x137F98 -> 0x137DDC */
    return 0;                                                    /* 0x137DE2 rts */
}
