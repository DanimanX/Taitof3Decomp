#include "fn_auto.h"
/* ROM 0x1A1BA6: format a variadic string into a stack buffer and draw it in text RAM. */
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"
#include <stdbool.h>

enum {
    ZERO_PAD = 0x02,
    LEFT_ALIGN = 0x04,
    PRECISION = 0x08,
    PLUS_SIGN = 0x10,
    SPACE_SIGN = 0x20,
    ALTERNATE = 0x40,
    SHORT_ARG = 0x80,
    LIMIT = 0x8000
};

static uint32_t next_arg(const f3mem_t *m, uint32_t *args)
{
    uint32_t value = rd32(m, *args);
    *args += 4u;
    return value;
}

static uint32_t divu_word(uint32_t dividend, uint32_t divisor)
{
    uint32_t quotient = dividend / divisor;
    if (quotient > 0xFFFFu)
        return dividend;
    return ((dividend % divisor) << 16) | quotient;
}

/* 0x0060CC / 0x006262: both loops divide a 32-bit value using two DIVU.Ws
 * and build the characters in reverse order below A0. The final comparison
 * is signed, including when 0x006262 receives an unshifted 0x80000000. */
static void signed_digits(const f3mem_t *m, f3regs_t *r, uint32_t value)
{
    while ((int32_t)value > 9) {
        uint32_t high = divu_word(value >> 16, 10u);
        uint32_t low = divu_word(set_word(high, (uint16_t)value), 10u);
        wr8(m, --r->a[0], (uint8_t)((low >> 16) + '0'));
        /* 0x0060E6/0x00627C: join the two quotient words after SWAP. */
        value = ((uint32_t)(uint16_t)high << 16) | (uint16_t)low;
    }
    wr8(m, --r->a[0], (uint8_t)(value + '0'));
}

/* 0x006316-0x0063FE: each copy or fill uses SUBQ.W #1 / BLT / DBRA. */
static uint16_t copy_field(const f3mem_t *m, uint32_t *out, uint32_t *src, uint16_t count)
{
    if ((int16_t)count <= 0)
        return (uint16_t)(count - 1u);
    do {
        wr8(m, (*out)++, rd8(m, (*src)++));
    } while (--count);
    return 0xFFFFu;
}

static void pad_field(const f3mem_t *m, uint32_t *out, uint8_t byte, uint16_t count)
{
    if ((int16_t)count > 0) {
        do {
            wr8(m, (*out)++, byte);
        } while (--count);
    }
}

static void emit_field(const f3mem_t *m, f3regs_t *r, uint32_t *out, uint32_t local,
                       uint16_t flags, uint16_t width, uint16_t precision, uint16_t length)
{
    /* 0x0062E8: an explicit limit truncates from the start. CMP via SUB.W
     * is signed, but the address adjustment uses the wrapped word. */
    if (flags & LIMIT) {
        uint16_t limit = rd16(m, local);
        if ((int16_t)length > (int16_t)limit) {
            r->a[0] += (uint32_t)(int32_t)(int16_t)(length - limit);
            length = limit;
        }
    }

    /* Both PC-relative bases resolve to 0x006306. The byte index is even. */
    uint32_t handler = 0x006306u + (int16_t)rd16(m, 0x006306u + (flags & 0xEu));
    uint16_t prefix = (uint16_t)r->d[1];
    uint16_t zeroes = 0;
    if ((handler == 0x006316u || handler == 0x0063A0u)
        && (int16_t)precision > (int16_t)length)
        zeroes = (uint16_t)(precision - length);
    uint16_t spaces = (uint16_t)(width - length - zeroes - prefix);
    bool left = handler == 0x0063DAu || handler == 0x0063A0u;
    bool zero_pad = handler == 0x006378u;
    if (!left && !zero_pad)
        pad_field(m, out, ' ', spaces);
    r->d[1] = set_word(r->d[1], copy_field(m, out, &r->a[1], prefix));
    if (zero_pad)
        pad_field(m, out, '0', spaces);
    else
        pad_field(m, out, '0', zeroes);
    copy_field(m, out, &r->a[0], length);
    if (left)
        pad_field(m, out, ' ', spaces);
}

/* ROM 0x005C62: format into the destination until NUL, returning its length
 * in D0. D1/A0/A1 carry the scratch results into the following text call. */
static void format_005C62(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]); /* LINK.W #-0x22 */
    r->a[6] = r->a[7];
    r->a[7] -= 0x22u;
    for (unsigned reg = 5; reg > 2;)
        push_long(m, r, r->a[--reg]);
    for (unsigned reg = 6; reg > 2;)
        push_long(m, r, r->d[--reg]);

    const uint32_t local = r->a[6] - 2u;
    uint32_t src = rd32(m, r->a[6] + 0xCu);
    uint32_t out = rd32(m, r->a[6] + 8u);
    uint32_t args = rd32(m, r->a[6] + 0x10u);
    uint16_t precision = (uint16_t)r->d[5];

    for (;;) {
        uint8_t ch = rd8(m, src++);
        if (ch != '%') {
            wr8(m, out++, ch); /* 0x005C7C */
            if (ch == 0) {
                --out;
                break;
            }
            continue;
        }

        uint16_t flags = 0;
        uint16_t width = 0;
        for (;;) {
            ch = rd8(m, src++);
            if (ch == 0) {
                wr8(m, out, 0); /* 0x005CA4 */
                goto done;
            }

            /* 0x005C9C/0x005CA0: the two extension-word bases each resolve
             * to 0x005CB4. Codes outside the table go to the literal path. */
            uint32_t handler = 0x0062E2u;
            if ((int8_t)ch >= 0x20)
                handler = 0x005CB4u + (int16_t)rd16(m, 0x005CB4u + (ch - 0x20u) * 2u);
            uint16_t length;

            switch (handler) {
            case 0x005D74: flags |= SPACE_SIGN; continue;
            case 0x005D7C: flags |= ALTERNATE; continue;
            case 0x005DBE: flags |= PLUS_SIGN; continue;
            case 0x005DC6: flags |= LEFT_ALIGN; continue;
            case 0x005DEA: flags |= ZERO_PAD; continue;
            case 0x005DF2: flags |= SHORT_ARG; continue;
            case 0x005DFA: case 0x005DFE: continue; /* l and L */
            case 0x005DCE:
                if (!(flags & PRECISION)) {
                    flags |= PRECISION;
                    precision = 0;
                } else if (!(flags & LIMIT)) {
                    flags |= LIMIT;
                    wr16(m, local, 0);
                }
                continue;

            case 0x005D84: { /* *: signed low word of the next long */
                int32_t field = (int16_t)next_arg(m, &args);
                if (!(flags & PRECISION)) {
                    if (field < 0) {
                        field = -field;
                        flags ^= LEFT_ALIGN;
                    }
                    width = (uint16_t)field;
                } else if (!(flags & LIMIT)) {
                    precision = field < 0 ? 0 : (uint16_t)field;
                } else {
                    wr16(m, local, field < 0 ? 0 : (uint16_t)field);
                }
                continue;
            }

            case 0x005E02: { /* decimal width, precision or limit */
                uint16_t field = ch - '0';
                for (;;) {
                    ch = rd8(m, src);
                    if (ch == 0) {
                        wr8(m, out, 0);
                        goto done;
                    }
                    if (ch < '0' || ch > '9')
                        break;
                    /* 0x005E1C: D1.w survives if the next byte is NUL. */
                    r->d[1] = set_word(r->d[1], (uint16_t)(field * 2u));
                    field = (uint16_t)(field * 10u + ch - '0');
                    ++src;
                }
                r->d[1] = flags & PRECISION; /* 0x005E2A */
                if (!(flags & PRECISION))
                    width = field;
                else if (!(flags & LIMIT))
                    precision = field;
                else
                    wr16(m, local, field);
                continue;
            }

            case 0x005EF4: /* %c takes the low byte of a long in place. */
                args += 4u;
                r->a[0] = args - 1u;
                r->d[1] = 0;
                length = 1;
                break;

            case 0x005F02: { /* %s */
                uint32_t address = next_arg(m, &args);
                if (address == 0) {
                    flags &= (uint16_t)~PRECISION;
                    r->d[1] = 0;
                    length = 0;
                    break;
                }
                r->a[0] = address;
                uint32_t cursor = address;
                if (flags & PRECISION) {
                    flags &= (uint16_t)~PRECISION;
                    while (rd8(m, cursor++) != 0 && --precision != 0xFFFFu) {
                    }
                } else {
                    while (rd8(m, cursor++) != 0) {
                    }
                }
                r->a[1] = cursor - address; /* 0x005F1E / 0x005F30 */
                length = (uint16_t)(r->a[1] - 1u);
                r->d[1] = 0;
                break;
            }

            case 0x006064: { /* %n also enters the common field emitter. */
                uint32_t count = out - rd32(m, r->a[6] + 8u);
                r->a[0] = next_arg(m, &args);
                if (flags & SHORT_ARG)
                    wr16(m, r->a[0], (uint16_t)count);
                else
                    wr32(m, r->a[0], count);
                length = (uint16_t)count;
                break;
            }

            case 0x005E48: case 0x005E9E: case 0x005F3C: case 0x005FA0:
            case 0x006004: case 0x006034: {
                /* %b, %o, %x, %X and fixed-size %p/%P. */
                bool fixed = handler == 0x006004u || handler == 0x006034u;
                bool upper = handler == 0x005FA0u || handler == 0x006034u;
                uint32_t digits = upper ? 0x005FF2u : 0x005F8Eu;
                unsigned shift = handler == 0x005E48u ? 1u : handler == 0x005E9Eu ? 3u : 4u;
                uint32_t value = next_arg(m, &args);
                if (!fixed && (flags & SHORT_ARG))
                    value = (uint16_t)value;
                r->d[1] = 0;
                if (!fixed && value == 0 && (flags & PRECISION) && precision == 0) {
                    length = 0;
                    break;
                }
                bool prefix = (flags & ALTERNATE) && (fixed || value != 0);
                r->a[1] = fixed ? digits : value;
                r->a[0] = local;
                unsigned remaining = fixed ? 8u : 0u;
                do {
                    unsigned digit = value & ((1u << shift) - 1u);
                    uint8_t byte = shift == 4u ? rd8(m, digits + digit) : (uint8_t)('0' + digit);
                    wr8(m, --r->a[0], byte);
                    value >>= shift;
                } while (fixed ? --remaining != 0 : value != 0);
                if (prefix) {
                    if (shift == 4u) {
                        r->d[1] = 2u;
                        r->a[1] = digits + 16u;
                    } else {
                        wr8(m, --r->a[0], '0');
                    }
                }
                length = (uint16_t)(local - r->a[0]);
                break;
            }

            case 0x00607C: { /* %d and %i take a signed long. */
                uint32_t value = next_arg(m, &args);
                r->d[1] = 0;
                if (value == 0 && (flags & PRECISION) && precision == 0) {
                    length = 0;
                    break;
                }
                if ((int32_t)value < 0) {
                    r->d[1] = 1;
                    r->a[1] = 0x00610Cu;
                    value = 0u - value;
                    if ((int32_t)value < 0) {
                        r->a[0] = 0x006102u;
                        length = 10;
                        break;
                    }
                } else if (flags & PLUS_SIGN) {
                    r->d[1] = 1;
                    r->a[1] = 0x00610Du;
                } else if (flags & SPACE_SIGN) {
                    r->d[1] = 2; /* 0x0060BC: ROM emits space and a NUL. */
                    r->a[1] = 0x00610Eu;
                }
                r->a[0] = local;
                push_long(m, r, local);
                signed_digits(m, r, value);
                length = (uint16_t)(rd32(m, r->a[7]) - r->a[0]);
                r->a[7] += 4u;
                break;
            }

            case 0x006110: { /* %u */
                uint32_t value = next_arg(m, &args);
                if (flags & SHORT_ARG)
                    value = (uint16_t)value;
                r->d[1] = 0;
                if (value == 0 && (flags & PRECISION) && precision == 0) {
                    length = 0;
                    break;
                }
                r->a[0] = local;
                push_long(m, r, local);
                while (value > 9u) {
                    uint32_t bit = value & 1u;
                    value >>= 1;
                    r->a[1] = divu_word(value >> 16, 5u);
                    uint32_t low = divu_word(set_word(r->a[1], (uint16_t)value), 5u);
                    wr8(m, --r->a[0], (uint8_t)((low >> 16) * 2u + bit + '0'));
                    value = ((uint32_t)(uint16_t)r->a[1] << 16) | (uint16_t)low;
                }
                wr8(m, --r->a[0], (uint8_t)(value + '0'));
                length = (uint16_t)(rd32(m, r->a[7]) - r->a[0]);
                r->a[7] += 4u;
                break;
            }

            case 0x00617A: { /* %B: packed decimal from an address. */
                uint32_t address = next_arg(m, &args);
                uint16_t count = 0;
                if (address != 0) {
                    r->a[1] = address;
                    count = flags & LIMIT ? rd16(m, local) : 8u;
                    flags &= (uint16_t)~LIMIT;
                }
                r->d[1] = 0;
                length = 0;
                if (count == 0)
                    break;
                uint16_t index = (uint16_t)(count - 1u);
                bool low_only = (index & 1u) == 0;
                uint16_t remaining = (uint16_t)((index >> 1) | (index & 0x8000u));
                uint8_t byte = rd8(m, r->a[1]++);
                if (low_only)
                    byte &= 0xFu;
                while (byte == 0 && --remaining != 0xFFFFu)
                    byte = rd8(m, r->a[1]++);
                if (byte == 0) {
                    if (!(flags & PRECISION) || precision != 0) {
                        length = 1;
                        r->a[0] = 0x005FF2u;
                    }
                    break;
                }
                --r->a[1];
                r->a[0] = r->a[6] - 0x22u;
                uint32_t end = r->a[0];
                if ((byte >> 4) != 0)
                    wr8(m, end++, (uint8_t)((rd8(m, r->a[1]) >> 4) + '0'));
                wr8(m, end++, (uint8_t)((rd8(m, r->a[1]++) & 0xFu) + '0'));
                while (--remaining != 0xFFFFu) {
                    wr8(m, end++, (uint8_t)((rd8(m, r->a[1]) >> 4) + '0'));
                    wr8(m, end++, (uint8_t)((rd8(m, r->a[1]++) & 0xFu) + '0'));
                }
                length = (uint16_t)(end - r->a[0]);
                break;
            }

            case 0x0061FE: { /* %D: signed binary fixed-point. */
                r->a[0] = r->a[6] - 0x12u;
                uint16_t bits = 15u;
                if (!(flags & PRECISION))
                    precision = 6;
                else if (flags & LIMIT) {
                    bits = rd16(m, local);
                    if ((int16_t)bits > 28)
                        bits = 28;
                }
                if (flags & PRECISION)
                    flags &= (uint16_t)~LIMIT;
                flags &= (uint16_t)~PRECISION;
                uint32_t value = rd32(m, args);
                r->d[1] = 0;
                if ((int32_t)value < 0) {
                    r->d[1] = 1;
                    r->a[1] = 0x00610Cu;
                    value = 0u - value;
                } else if (flags & PLUS_SIGN) {
                    r->d[1] = 1;
                    r->a[1] = 0x00610Du;
                } else if (flags & SPACE_SIGN) {
                    r->d[1] = 2;
                    r->a[1] = 0x00610Eu;
                }
                if (value == 0x80000000u && bits == 0) {
                    for (unsigned i = 0; i < 10u; ++i)
                        wr8(m, --r->a[0], rd8(m, --r->a[1]));
                    r->d[1] = 0xFFFFu;
                } else {
                    unsigned shift = bits & 63u;
                    signed_digits(m, r, shift < 32u ? value >> shift : 0u);
                }

                /* 0x006290: keep the first digit pointer on the stack while
                 * appending the fractional digits to the fixed-point buffer. */
                push_long(m, r, r->a[0]);
                uint32_t end = r->a[6] - 0x12u;
                wr8(m, end++, '.');
                unsigned shift = (28u - rd16(m, local)) & 63u;
                value = next_arg(m, &args);
                if ((int32_t)value < 0)
                    value = 0u - value;
                value = shift < 32u ? value << shift : 0u;
                while ((value &= 0x0FFFFFFFu) != 0 && --precision != 0xFFFFu) {
                    value *= 10u;
                    wr8(m, end++, (uint8_t)((value >> 28) + '0'));
                }
                --precision;
                if ((int16_t)precision >= 0) {
                    do {
                        wr8(m, end++, '0');
                    } while (--precision != 0xFFFFu);
                }
                r->a[0] = rd32(m, r->a[7]);
                r->a[7] += 4u;
                length = (uint16_t)(end - r->a[0]);
                break;
            }

            default: /* 0x0062E2: consume the escape and emit its byte. */
                wr8(m, out++, ch);
                goto next_literal;
            }
            emit_field(m, r, &out, local, flags, width, precision, length);
            break;
        }
    next_literal:;
    }

done:
    r->d[0] = out - rd32(m, r->a[6] + 8u);
    for (unsigned reg = 2; reg < 6; ++reg, r->a[7] += 4u)
        r->d[reg] = rd32(m, r->a[7]);
    for (unsigned reg = 2; reg < 5; ++reg, r->a[7] += 4u)
        r->a[reg] = rd32(m, r->a[7]);
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8u; /* UNLK / RTS */
}

int Fn_1A1BA6(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    r->a[7] -= 0x40u;
    push_long(m, r, r->a[6] + 0x12u);
    push_long(m, r, rd32(m, r->a[6] + 0xEu));
    push_long(m, r, r->a[6] - 0x40u);
    push_long(m, r, 0x1A1BBCu);
    format_005C62(m, r);

    /* 0x1A1BBC: push the draw arguments on top of the formatter arguments.
     * The existing 0x005666 helper takes the arguments by value. */
    push_word(m, r, rd16(m, r->a[6] + 0xCu));
    push_long(m, r, rd32(m, r->a[6] + 8u));
    push_long(m, r, r->a[6] - 0x40u);
    push_long(m, r, 0x1A1BCEu);
    push_long(m, r, r->a[6]); /* 0x005666 LINK.W #0 */
    r->a[6] = r->a[7];
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    r->d[0] = Text_DrawString(m, rd32(m, r->a[6] + 8u),
                              rd32(m, r->a[6] + 0xCu), rd16(m, r->a[6] + 0x10u));
    r->d[7] = rd32(m, r->a[7]);
    r->a[0] = rd32(m, r->a[7] + 4u);
    r->a[1] = rd32(m, r->a[7] + 8u);
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8u;
    r->a[7] += 0x16u;
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8u;
    return 0;
}
