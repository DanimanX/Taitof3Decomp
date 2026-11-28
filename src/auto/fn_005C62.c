#include "fn_auto.h"

/* ROM 0x005C62: format a string from a big-endian list of 32-bit arguments. */

enum {
    ZERO_PAD = 0x02, LEFT_ALIGN = 0x04, PRECISION = 0x08,
    PLUS_SIGN = 0x10, SPACE_SIGN = 0x20, ALTERNATE = 0x40,
    SHORT_ARG = 0x80, EXTRA_PRECISION = 0x8000
};

typedef struct {
    const f3mem_t *m;
    f3regs_t *r;
    uint32_t frame, sp, fmt, dst, args;
    uint16_t flags, width, precision;
} formatter_t;

static void push_long(formatter_t *f, uint32_t value)
{
    f->sp -= 4u;
    wr32(f->m, f->sp, value);
}

static uint32_t next_arg(formatter_t *f)
{
    uint32_t value = rd32(f->m, f->args);
    f->args += 4u;
    return value;
}

static void emit(formatter_t *f, uint8_t c)
{
    wr8(f->m, f->dst++, c);
}

/* 0x00631E..0x0063FE: the signed SUBQ/BLT followed by the DBRA loop. */
static void pad(formatter_t *f, uint16_t count, uint8_t c)
{
    for (int i = (int16_t)count; i > 0; --i)
        emit(f, c);
}

static uint16_t copy(formatter_t *f, uint32_t *src, uint16_t count)
{
    if ((int16_t)count <= 0)
        return (uint16_t)(count - 1u);
    do {
        uint8_t c = rd8(f->m, *src);
        ++*src;
        emit(f, c);
    } while (--count);
    return UINT16_MAX;
}

/* 0x0062E8: apply the second precision, then dispatch the padding layout. */
static void render(formatter_t *f, uint16_t length)
{
    f3regs_t *r = f->r;
    uint16_t prefix = (uint16_t)r->d[1];
    if (f->flags & EXTRA_PRECISION) {
        uint16_t limit = rd16(f->m, f->frame - 2u);
        if ((int16_t)length > (int16_t)limit) {
            r->a[0] += (uint32_t)(int32_t)(int16_t)(length - limit);
            length = limit;
        }
    }

    uint32_t table = 0x006306u; /* 0x0062FE and 0x006302 share this base. */
    uint32_t handler = table + (uint32_t)(int32_t)(int16_t)
        rd16(f->m, table + (f->flags & 0x0Eu));
    uint16_t zeroes = 0;
    if ((handler == 0x006316u || handler == 0x0063A0u) &&
        (int16_t)f->precision > (int16_t)length) {
        zeroes = (uint16_t)(f->precision - length);
        f->precision = UINT16_MAX; /* 0x00633E / 0x0063BC */
    }
    uint16_t spaces = (uint16_t)(f->width - length - zeroes - prefix);
    if (handler == 0x006316u || handler == 0x006350u)
        pad(f, spaces, ' ');

    prefix = copy(f, &r->a[1], prefix);
    r->d[1] = (r->d[1] & 0xFFFF0000u) | prefix;
    pad(f, handler == 0x006378u ? spaces : zeroes, '0');
    copy(f, &r->a[0], length);

    if (handler == 0x0063A0u || handler == 0x0063DAu)
        pad(f, spaces, ' ');
}

/* 0x005E48..0x006060: power-of-two conversions build digits backwards. */
static uint16_t format_radix(formatter_t *f, uint32_t handler)
{
    f3regs_t *r = f->r;
    uint32_t value = next_arg(f);
    int fixed = handler == 0x006004u || handler == 0x006034u;
    if (!fixed && (f->flags & SHORT_ARG))
        value = (uint16_t)value;
    r->d[1] = 0;
    if (!value && !fixed && (f->flags & PRECISION) && !f->precision)
        return 0;

    unsigned shift = handler == 0x005E48u ? 1u : handler == 0x005E9Eu ? 3u : 4u;
    uint32_t digits = (handler == 0x005FA0u || handler == 0x006034u)
        ? 0x005FF2u : 0x005F8Eu;
    r->a[1] = fixed ? digits : value;
    r->a[0] = f->frame - 2u;
    uint32_t remaining = value;
    uint16_t length = 0;
    do {
        unsigned digit = remaining & ((1u << shift) - 1u);
        uint8_t c = shift == 4u ? rd8(f->m, digits + digit) : (uint8_t)('0' + digit);
        wr8(f->m, --r->a[0], c);
        remaining >>= shift;
        ++length;
    } while (fixed ? length < 8u : remaining != 0u);

    if (handler == 0x006004u)
        return Formatter_Prepare_006020(r, f->flags);

    if ((f->flags & ALTERNATE) && (fixed || value)) {
        if (shift == 4u) {
            r->a[1] = digits + 16u;
            r->d[1] = 2;
        } else {
            wr8(f->m, --r->a[0], '0');
            ++length;
        }
    }
    return length;
}

/* 0x005F02: DBEQ stops at NUL or after testing precision + 1 bytes. */
static uint16_t format_string(formatter_t *f)
{
    f3regs_t *r = f->r;
    uint32_t value = next_arg(f);
    int limited = (f->flags & PRECISION) != 0;
    f->flags &= (uint16_t)~PRECISION;
    r->d[1] = 0;
    if (!value)
        return 0;

    r->a[0] = r->a[1] = value;
    while (rd8(f->m, r->a[1]++)) {
        if (limited && --f->precision == UINT16_MAX)
            break;
    }
    uint32_t size = r->a[1] - value;
    r->a[1] = size;
    return (uint16_t)(size - 1u);
}

/* 0x006080 / 0x0060A8 and 0x006226 / 0x006242: decimal sign prefix. */
static void decimal_sign(formatter_t *f, int negative)
{
    f3regs_t *r = f->r;
    r->d[1] = 0;
    if (negative) {
        r->d[1] = 1;
        r->a[1] = 0x00610Cu;
    } else if (f->flags & PLUS_SIGN) {
        r->d[1] = 1;
        r->a[1] = 0x00610Du;
    } else if (f->flags & SPACE_SIGN) {
        r->d[1] = 2;
        r->a[1] = 0x00610Eu; /* The second byte of this prefix is NUL. */
    }
}

/* 0x0060C2 / 0x006132 / 0x00625C: the paired DIVU.Ws give quotient and
 * remainder of the full longword. The unsigned loop also exposes the first
 * DIVU result in A1. */
static void decimal_digits(formatter_t *f, uint32_t value, int is_unsigned)
{
    f3regs_t *r = f->r;
    while (is_unsigned ? value > 9u : (int32_t)value > 9) {
        if (is_unsigned) {
            uint32_t high = value >> 17;
            r->a[1] = ((high % 5u) << 16) | (high / 5u);
        }
        wr8(f->m, --r->a[0], (uint8_t)('0' + value % 10u));
        value /= 10u;
    }
    wr8(f->m, --r->a[0], (uint8_t)('0' + value));
}

static uint16_t format_decimal(formatter_t *f, int is_unsigned)
{
    f3regs_t *r = f->r;
    uint32_t value = next_arg(f);
    if (is_unsigned && (f->flags & SHORT_ARG))
        value = (uint16_t)value;
    if (!value && (f->flags & PRECISION) && !f->precision) {
        r->d[1] = 0;
        return 0;
    }

    int negative = !is_unsigned && (int32_t)value < 0;
    if (is_unsigned)
        r->d[1] = 0;
    else
        decimal_sign(f, negative);
    if (negative) {
        value = 0u - value;
        if (value == 0x80000000u) {
            r->a[0] = 0x006102u;
            return 10;
        }
    }

    uint32_t end = f->frame - 2u;
    r->a[0] = end;
    push_long(f, end);
    decimal_digits(f, value, is_unsigned);
    end = rd32(f->m, f->sp);
    f->sp += 4u;
    return (uint16_t)(end - r->a[0]);
}

/* 0x00617A: packed BCD with leading-zero suppression and a digit limit. */
static uint16_t format_bcd(formatter_t *f)
{
    f3regs_t *r = f->r;
    uint32_t value = next_arg(f);
    r->d[1] = 0;
    if (!value)
        return 0;
    r->a[1] = value;
    uint16_t digits = 8;
    if (f->flags & EXTRA_PRECISION) {
        f->flags &= (uint16_t)~EXTRA_PRECISION;
        digits = rd16(f->m, f->frame - 2u);
        if (!digits)
            return 0;
    }

    uint16_t bytes = (uint16_t)(digits - 1u);
    uint8_t mask = bytes & 1u ? 0xFFu : 0x0Fu;
    bytes = (uint16_t)((int16_t)bytes >> 1);
    uint8_t digit;
    do {
        digit = rd8(f->m, r->a[1]++) & mask;
        if (digit)
            break;
        mask = 0xFFu;
    } while (--bytes != UINT16_MAX);

    if (!digit) {
        if ((f->flags & PRECISION) && !f->precision)
            return 0;
        r->a[0] = 0x005FF2u;
        return 1;
    }

    --r->a[1];
    uint32_t start = f->frame - 0x22u;
    uint32_t dst = start;
    do {
        if (digit & 0xF0u)
            wr8(f->m, dst++, (uint8_t)('0' + (rd8(f->m, r->a[1]) >> 4)));
        uint8_t low = rd8(f->m, r->a[1]++) & 0x0Fu;
        wr8(f->m, dst++, (uint8_t)('0' + low));
        digit = 0xF0u; /* 0x0061F0 loops directly to the high-digit write. */
    } while (--bytes != UINT16_MAX);
    r->a[0] = start;
    return (uint16_t)(dst - start);
}

/* 0x0061FE: fixed-point conversion with separate integer and fraction shifts. */
static uint16_t format_fixed(formatter_t *f)
{
    f3regs_t *r = f->r;
    uint32_t point = f->frame - 0x12u;
    r->a[0] = point;
    uint16_t shift = 15;
    if (!(f->flags & PRECISION)) {
        f->precision = 6;
    } else if (f->flags & EXTRA_PRECISION) {
        f->flags &= (uint16_t)~EXTRA_PRECISION;
        shift = rd16(f->m, f->frame - 2u);
        if ((int16_t)shift > 28)
            shift = 28;
    }
    f->flags &= (uint16_t)~PRECISION;

    uint32_t value = rd32(f->m, f->args);
    int negative = (int32_t)value < 0;
    decimal_sign(f, negative);
    if (negative)
        value = 0u - value;
    if (value == 0x80000000u && shift == 0) {
        for (unsigned i = 0; i < 10; ++i)
            wr8(f->m, --r->a[0], rd8(f->m, --r->a[1]));
        r->d[1] = UINT16_MAX; /* 0x00623C: DBRA leaves D1.w at 0xFFFF. */
    } else {
        unsigned count = shift & 63u;
        decimal_digits(f, count < 32u ? value >> count : 0, 0);
    }
    push_long(f, r->a[0]);

    wr8(f->m, point++, '.');
    unsigned count = (28u - rd16(f->m, f->frame - 2u)) & 63u;
    value = next_arg(f);
    if ((int32_t)value < 0)
        value = 0u - value;
    value = count < 32u ? value << count : 0;
    /* 0x0062C8 DBEQ decrements only while the masked remainder is nonzero. */
    while ((value &= 0x0FFFFFFFu) != 0u && --f->precision != UINT16_MAX) {
        value *= 10u;
        wr8(f->m, point++, (uint8_t)('0' + (value >> 28)));
    }
    if ((int16_t)--f->precision >= 0) {
        do {
            wr8(f->m, point++, '0');
        } while (--f->precision != UINT16_MAX);
    }
    r->a[0] = rd32(f->m, f->sp);
    f->sp += 4u;
    return (uint16_t)(point - r->a[0]);
}

/* 0x005E02: accumulate decimal width or precision in a word. */
static int format_number(formatter_t *f, uint16_t first)
{
    uint16_t number = first;
    for (;;) {
        uint8_t c = rd8(f->m, f->fmt);
        if (!c)
            return 0;
        if (c < '0' || c > '9')
            break;
        f->r->d[1] = (f->r->d[1] & 0xFFFF0000u) | (uint16_t)(number * 2u);
        number = (uint16_t)(number * 10u + c - '0');
        ++f->fmt;
    }
    f->r->d[1] = f->flags & PRECISION;
    if (!(f->flags & PRECISION))
        f->width = number;
    else if (!(f->flags & EXTRA_PRECISION))
        f->precision = number;
    else
        wr16(f->m, f->frame - 2u, number);
    return 1;
}

/* 0x005C8A: consume format modifiers up to one conversion or literal. */
static int format_specifier(formatter_t *f)
{
    f3regs_t *r = f->r;
    f->flags = f->width = 0;
    for (;;) {
        uint8_t c = rd8(f->m, f->fmt++);
        if (!c)
            return 0;
        /* Both brief PC-index extensions have scale 1, based at 5C9E/5CA2. */
        uint32_t table = 0x005CB4u;
        uint32_t handler = 0x0062E2u;
        if ((int8_t)c >= 0x20)
            handler = table + (uint32_t)(int32_t)(int16_t)
                rd16(f->m, table + 2u * (c - 0x20u));

        uint16_t length;
        switch (handler) {
        case 0x005D74: f->flags |= SPACE_SIGN; continue;
        case 0x005D7C: f->flags |= ALTERNATE; continue;
        case 0x005DBE: f->flags |= PLUS_SIGN; continue;
        case 0x005DC6: f->flags |= LEFT_ALIGN; continue;
        case 0x005DEA: f->flags |= ZERO_PAD; continue;
        case 0x005DF2: f->flags |= SHORT_ARG; continue;
        case 0x005DFA:
        case 0x005DFE: continue;
        case 0x005D84: {
            int16_t value = (int16_t)next_arg(f);
            if (!(f->flags & PRECISION)) {
                f->width = (uint16_t)(value < 0 ? -(int32_t)value : value);
                if (value < 0)
                    f->flags ^= LEFT_ALIGN;
            } else if (!(f->flags & EXTRA_PRECISION)) {
                f->precision = (uint16_t)(value < 0 ? 0 : value);
            } else {
                wr16(f->m, f->frame - 2u, (uint16_t)(value < 0 ? 0 : value));
            }
            continue;
        }
        case 0x005DCE:
            if (!(f->flags & PRECISION))
                f->precision = 0;
            else if (!(f->flags & EXTRA_PRECISION)) {
                f->flags |= EXTRA_PRECISION;
                wr16(f->m, f->frame - 2u, 0);
            }
            f->flags |= PRECISION;
            continue;
        case 0x005E02:
            if (!format_number(f, (uint16_t)(c - '0')))
                return 0;
            continue;
        case 0x005E48:
        case 0x005E9E:
        case 0x005F3C:
        case 0x005FA0:
        case 0x006004:
        case 0x006034: length = format_radix(f, handler); break;
        case 0x005EF4:
            f->args += 4u;
            r->a[0] = f->args - 1u;
            r->d[1] = 0;
            length = 1;
            break;
        case 0x005F02: length = format_string(f); break;
        case 0x006064: {
            uint32_t count = f->dst - rd32(f->m, f->frame + 8u);
            r->a[0] = next_arg(f);
            if (f->flags & SHORT_ARG)
                wr16(f->m, r->a[0], (uint16_t)count);
            else
                wr32(f->m, r->a[0], count);
            length = (uint16_t)count;
            break;
        }
        case 0x00607C:
        case 0x006110: length = format_decimal(f, handler == 0x006110u); break;
        case 0x00617A: length = format_bcd(f); break;
        case 0x0061FE: length = format_fixed(f); break;
        case 0x0062E2:
            emit(f, c);
            return 1;
        default:
            /* The ROM's 96 entries all point to the handlers above. */
            return 0;
        }
        render(f, length);
        return 1;
    }
}

/* 0x005C76: resume the format loop, then return from the enclosing call. */
static int finish_format(formatter_t *f)
{
    const f3mem_t *m = f->m;
    f3regs_t *r = f->r;
    for (;;) {
        uint8_t c = rd8(m, f->fmt++);
        if (c == '%') {
            if (!format_specifier(f)) {
                wr8(m, f->dst, 0);
                break;
            }
        } else {
            emit(f, c);
            if (!c) {
                --f->dst;
                break;
            }
        }
    }

    r->d[0] = f->dst - rd32(m, f->frame + 8u);
    for (unsigned i = 2; i <= 5; ++i, f->sp += 4u)
        r->d[i] = rd32(m, f->sp);
    for (unsigned i = 2; i <= 4; ++i, f->sp += 4u)
        r->a[i] = rd32(m, f->sp);
    r->a[6] = rd32(m, f->frame);
    r->a[7] = f->frame + 8u; /* UNLK and RTS. */
    return 0;
}

int Formatter_RenderAndResume(const f3mem_t *m, f3regs_t *r, uint16_t length)
{
    formatter_t f = { .m = m, .r = r, .frame = r->a[6], .sp = r->a[7],
                      .fmt = r->a[2], .dst = r->a[3], .args = r->a[4],
                      .flags = (uint16_t)r->d[3], .width = (uint16_t)r->d[4],
                      .precision = (uint16_t)r->d[5] };
    render(&f, length);
    return finish_format(&f);
}

int Fn_005C62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t frame = r->a[7] - 4u;
    formatter_t f = { .m = m, .r = r, .frame = frame, .sp = frame - 0x22u,
                      .precision = (uint16_t)r->d[5] };
    wr32(m, frame, r->a[6]); /* LINK and predecrement MOVEM at 0x005C62. */
    for (int i = 4; i >= 2; --i)
        push_long(&f, r->a[i]);
    for (int i = 5; i >= 2; --i)
        push_long(&f, r->d[i]);
    f.fmt = rd32(m, frame + 12u);
    f.dst = rd32(m, frame + 8u);
    f.args = rd32(m, frame + 16u);
    return finish_format(&f);
}
