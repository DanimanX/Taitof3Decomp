#include "fn_auto.h"
/* ROM 0x1176D0: UNVERIFIED (no capture): record a circular nibble history and detect a five-code sequence. */

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

static uint32_t with_byte(uint32_t reg, uint8_t byte)
{
    return (reg & 0xFFFFFF00u) | byte;
}

/* 0x117702 and 0x11778C use the full signed D1 as a memory bit offset. */
static uint32_t field_address(const f3regs_t *r)
{
    int32_t bit = (int32_t)r->d[1];
    return r->a[1] + (uint32_t)((bit - (int32_t)(r->d[1] & 7u)) / 8);
}

static void insert_history_code(const f3mem_t *m, const f3regs_t *r)
{
    uint32_t addr = field_address(r);
    unsigned bit = r->d[1] & 7u;
    uint8_t code = r->d[0] & 0x0Fu;
    if (bit <= 4) {
        unsigned shift = 4 - bit;
        uint8_t mask = (uint8_t)(0x0Fu << shift);
        wr8(m, addr, (rd8(m, addr) & (uint8_t)~mask) | (code << shift));
    } else {
        unsigned shift = 12 - bit;
        uint16_t mask = (uint16_t)(0x0Fu << shift);
        wr16(m, addr, (rd16(m, addr) & (uint16_t)~mask) | ((uint16_t)code << shift));
    }
}

/* ROM 0x117788: look back up to ten mismatches, within the 64-step history budget. */
static void find_code(const f3mem_t *m, f3regs_t *r)
{
    r->d[4] = with_word(r->d[4], 9);
    for (;;) {
        uint32_t addr = field_address(r);
        unsigned bit = r->d[1] & 7u;
        uint16_t data = bit <= 4 ? (uint16_t)(rd8(m, addr) << 8) : rd16(m, addr);
        r->d[5] = (data >> (12 - bit)) & 0x0Fu;
        if ((uint8_t)r->d[5] == (uint8_t)r->d[3])
            return;

        int16_t cursor = (int16_t)r->d[1];
        r->d[1] = with_word(r->d[1], cursor >= 4 ? (uint16_t)(cursor - 4) : 0xFC);
        r->d[2] = with_word(r->d[2], (uint16_t)(r->d[2] + 1u));
        if ((int16_t)r->d[2] >= 0x40)
            break; /* 0x1177A4 DBGE leaves D4 unchanged if GE is true. */
        r->d[4] = with_word(r->d[4], (uint16_t)(r->d[4] - 1u));
        if ((uint16_t)r->d[4] == 0xFFFF)
            break;
    }
    r->d[3] = with_byte(r->d[3], 0); /* 0x1177A8 */
}

int Fn_1176D0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 0x08);
    if (rd8(m, r->a[1] + 0x50) & 0x10u) {
        FnAuto_FreeObject(m, r, obj, 0x117782);
        return 0;
    }

    r->a[1] = obj + 0x44;
    r->a[2] = rd32(m, obj + 0x0C);
    r->a[2] = rd32(m, r->a[2] + 0x44);
    r->d[0] = with_byte(r->d[0], rd8(m, r->a[2]));
    wr16(m, obj + 0x3A, (uint16_t)(rd16(m, obj + 0x3A) + 4u));
    if ((int16_t)rd16(m, obj + 0x3A) >= 0x100)
        wr16(m, obj + 0x3A, 0);
    r->d[1] = with_word(r->d[1], rd16(m, obj + 0x3A));
    insert_history_code(m, r);

    r->a[3] = rd32(m, obj + 0x08);
    if (rd8(m, r->a[3] + 0x51) == 0 && !(rd8(m, r->a[2] + 1) & 0x10u)) {
        r->d[1] = with_word(r->d[1], rd16(m, obj + 0x3A));
        r->d[2] = with_word(r->d[2], 0);
        static const uint8_t codes[] = { 7, 6, 0xE, 0xA, 0xB };
        unsigned i;
        for (i = 0; i < sizeof(codes); ++i) {
            r->d[3] = with_byte(r->d[3], codes[i]);
            r->a[7] -= 4;
            wr32(m, r->a[7], 0x11772Au + 0xEu * i);
            find_code(m, r);
            r->a[7] += 4;
            if ((uint8_t)r->d[3] == 0)
                break;
        }
        if (i == sizeof(codes)) {
            r->a[1] = rd32(m, obj + 0x08); /* 0x117768 */
            wr8(m, r->a[1] + 0x51, rd8(m, r->a[1] + 0x51) | 2u);
        }
    }

    wr32(m, obj, 0x1176D0); /* 0x117776 -> 0x1176C8 */
    return 0;
}
