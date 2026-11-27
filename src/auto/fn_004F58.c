#include "fn_auto.h"

/* ROM 0x004F58: verified on an injected capture (F3_INJECT, fni_004F58; an empty body fails it): restore EEPROM settings fields and regenerate their checksum. */

static void copy_field(const f3mem_t *m, uint32_t dst, uint32_t src, uint8_t mask)
{
    uint8_t field = rd8(m, src) & mask;
    wr8(m, dst, (uint8_t)((rd8(m, dst) & ~mask) | field));
}

/* ROM 0x004BD2: sum the signed-length block's words and replace its checksum. */
static void checksum(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, r->d[3]);
    wr32(m, sp - 8u, r->d[2]);
    uint32_t base = rd32(m, sp + 4u);
    int32_t last = (int32_t)rd32(m, sp + 8u) / 2 - 1;
    uint32_t addr = base;
    uint16_t sum = 0;
    for (int32_t i = 0; i < last; ++i, addr += 2u)
        sum = (uint16_t)(sum + rd16(m, addr)); /* 0x004BE0: D2.l * 2 */

    r->a[0] = (uint32_t)((int32_t)(int16_t)sum + (int16_t)rd16(m, addr));
    r->a[1] = base;
    r->d[0] = (uint16_t)r->a[0];
    r->d[1] = 0u - (uint32_t)(int32_t)(int16_t)sum;
    wr16(m, addr, (uint16_t)r->d[1]);
    r->d[2] = rd32(m, sp - 8u);
    r->d[3] = rd32(m, sp - 4u);
    r->a[7] = sp + 4u;
}

int Fn_004F58(const f3mem_t *m, f3regs_t *r)
{
    uint32_t dst = 0x406C54u;
    uint32_t defaults = 0x1A1BD8u;
    wr8(m, dst + 0x12u, rd8(m, 0x1A1C34u));
    wr8(m, dst + 0x13u, rd8(m, 0x1A1C35u));
    for (uint32_t off = 0x14u; off <= 0x17u; ++off) {
        copy_field(m, dst + off, defaults + off, 0xF0u);
        copy_field(m, dst + off, defaults + off, 0x0Fu);
    }
    /* 0x004FE0..0x00500A: two flags followed by two three-bit fields. */
    copy_field(m, dst + 0x18u, defaults + 0x18u, 0x80u);
    copy_field(m, dst + 0x18u, defaults + 0x18u, 0x40u);
    copy_field(m, dst + 0x18u, defaults + 0x18u, 0x38u);
    copy_field(m, dst + 0x18u, defaults + 0x18u, 0x07u);
    wr8(m, dst + 0x19u, rd8(m, defaults + 0x19u));
    wr16(m, dst + 0x1Au, rd16(m, defaults + 0x1Au));
    wr16(m, dst + 0x1Cu, rd16(m, defaults + 0x1Cu));

    wr32(m, r->a[7] - 4u, 0x20u);
    wr32(m, r->a[7] - 8u, dst);
    wr32(m, r->a[7] - 12u, 0x00502Cu);
    r->a[7] -= 12u;
    checksum(m, r);
    r->a[7] += 12u; /* Discard the two arguments and return. */
    return 0;
}
