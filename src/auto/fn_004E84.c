#include "fn_auto.h"
/* ROM 0x004E84: UNVERIFIED (weak capture): copy default settings fields and update their checksum. */
#include "fn_call_frames.h"

static void copy_bits(const f3mem_t *m, uint32_t dst, uint32_t src, uint8_t mask)
{
    wr8(m, dst, (uint8_t)((rd8(m, dst) & ~mask) | (rd8(m, src) & mask)));
}

int Fn_004E84(const f3mem_t *m, f3regs_t *r)
{
    uint32_t dst = r->a[0] = 0x406C54u;
    uint32_t src = r->a[1] = 0x1A1BD8u;
    wr16(m, dst + 6u, rd16(m, src + 6u));                /* 0x004E90 */
    wr16(m, dst + 8u, rd16(m, src + 8u));                /* 0x004E96 */

    /* 0x004E9C..0x004EFC: BFINS copies the fields in MSB-first order.
     * The field at bit offset 3 crosses the byte boundary. */
    copy_bits(m, dst + 0x0Au, src + 0x0Au, 0x80u);
    copy_bits(m, dst + 0x0Au, src + 0x0Au, 0x40u);
    copy_bits(m, dst + 0x0Au, src + 0x0Au, 0x20u);
    uint16_t field = rd16(m, src + 0x0Au) & 0x1F80u;
    wr16(m, dst + 0x0Au, (uint16_t)((rd16(m, dst + 0x0Au) & 0xE07Fu) | field));
    copy_bits(m, dst + 0x0Bu, src + 0x0Bu, 0x60u);
    copy_bits(m, dst + 0x0Bu, src + 0x0Bu, 0x1Cu);
    copy_bits(m, dst + 0x0Bu, src + 0x0Bu, 0x03u);

    /* 0x004F02..0x004F44: four more nibbles, then four whole bytes. */
    copy_bits(m, dst + 0x0Cu, src + 0x0Cu, 0xF0u);
    copy_bits(m, dst + 0x0Cu, src + 0x0Cu, 0x0Fu);
    copy_bits(m, dst + 0x0Du, src + 0x0Du, 0xF0u);
    copy_bits(m, dst + 0x0Du, src + 0x0Du, 0x0Fu);
    for (uint32_t i = 0x0Eu; i <= 0x11u; i++)
        wr8(m, dst + i, rd8(m, src + i));

    /* 0x004F4A..0x004F56: push length, buffer and JSR return PC. */
    push_long(m, r, 0x20u);
    push_long(m, r, dst);
    push_long(m, r, 0x004F54u);
    Fn_004BD2(m, r);
    r->a[7] += 12u;
    return 0;
}
