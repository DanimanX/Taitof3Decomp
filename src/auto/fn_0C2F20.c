#include "fn_auto.h"
/* ROM 0x0C2F20: write a NUL-terminated string as text-layer tile words. */
#include "fn_call_frames.h"

/* The injected calls target the ROM. The F3 bus discards those writes. */
static void write_tile_byte(const f3mem_t *m, uint32_t addr, uint8_t value)
{
    if ((addr & 0xFFFFFFu) >= F3_ROM_SIZE)
        wr8(m, addr, value);
}

int Fn_0C2F20(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);                                    /* 0x0C2F20 LINK */
    r->a[6] = r->a[7];
    push_long(m, r, r->a[1]);                                    /* 0x0C2F24 MOVEM */
    push_long(m, r, r->d[7]);

    uint32_t src = rd32(m, r->a[6] + 8u);
    uint32_t dst = rd32(m, r->a[6] + 12u);
    uint8_t attr = (uint8_t)(rd16(m, r->a[6] + 16u) * 2u);
    uint8_t ch;
    while ((ch = rd8(m, src++)) != 0) {                          /* 0x0C2F36 */
        write_tile_byte(m, dst++, attr);                         /* 0x0C2F3A */
        write_tile_byte(m, dst++, ch);
    }
    r->d[0] = dst;                                              /* 0x0C2F40 */
    r->a[0] = src;
    r->d[7] = rd32(m, r->a[7]);
    r->a[1] = rd32(m, r->a[7] + 4u);
    r->a[7] = r->a[6] + 8u;                                      /* 0x0C2F46 + RTS */
    r->a[6] = rd32(m, r->a[6]);
    return 0;
}
