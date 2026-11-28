#include "fn_auto.h"
/* ROM 0x0050DC: validate the EEPROM settings and high-score blocks.
 * PARTIAL: recovery enters the serial EEPROM, input wait, or reset code.
 */
#include "fn_call_frames.h"

static uint16_t block_sum(const f3mem_t *m, uint32_t address, uint32_t length)
{
    uint16_t sum = 0;
    for (uint32_t i = 0; i < length; i += 2u)
        sum = (uint16_t)(sum + rd16(m, address + i));
    return sum;
}

int Fn_0050DC(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t settings = 0x406C54u;
    /* 0x005106..0x0051D8: preflight recovery before writing the stack or
     * recalculating either checksum. 0x004DEE writes through 0x004A0013. */
    for (unsigned i = 0; i < 5; ++i)
        if (rd8(m, settings + i) != rd8(m, 0x1A1BD8u + i))
            return -1;
    if ((int8_t)rd8(m, settings + 5u) != rd8(m, 0x1FFFFFu) ||
        block_sum(m, settings, 0x20u) != 0u ||
        rd32(m, settings + 6u) != 0x0001874Au ||
        rd16(m, settings + 0x12u) != rd16(m, 0x1A1C34u) ||
        block_sum(m, settings + 0x20u, 0x60u) != 0u)
        return -1;

    for (unsigned i = 5; i > 2;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 6; i > 2;)
        push_long(m, r, r->d[--i]);
    r->a[2] = settings;
    r->a[3] = 0x1A1BD8u;
    r->a[4] = 0x004F58u;
    r->d[3] = 0x004DEEu;
    r->d[4] = 0x004E84u;
    r->d[5] = 0x005030u;
    r->d[2] = 0;
    r->a[0] = (uint32_t)(int32_t)(int8_t)rd8(m, settings + 5u);
    r->d[0] = set_word(r->a[0], rd16(m, 0x1FFFFEu));
    r->d[1] = (uint8_t)r->d[0];

    push_long(m, r, 0x20u);
    push_long(m, r, settings);
    push_long(m, r, 0x00516Eu);
    Fn_004BD2(m, r);
    r->a[7] += 8u;

    r->d[0] = 0x00018700u | rd8(m, settings + 0x13u);
    push_long(m, r, 0x60u);
    push_long(m, r, settings + 0x20u);
    push_long(m, r, 0x0051CCu);
    Fn_004BD2(m, r);
    r->a[7] += 8u;

    r->d[0] = r->d[2];
    for (unsigned i = 2; i < 6; ++i, r->a[7] += 4u)
        r->d[i] = rd32(m, r->a[7]);
    for (unsigned i = 2; i < 5; ++i, r->a[7] += 4u)
        r->a[i] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return 0;
}
