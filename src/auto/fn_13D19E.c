#include "fn_auto.h"
#include "../objects/obj_common.h"

/* ROM 0x13D19E: convert each selected ROM palette to the ramp format and queue it. */
static uint8_t f13d19e_component(uint8_t v)
{
    return (uint8_t)(((uint16_t)v * 11u) >> 4);
}

int Fn_13D19E(const f3mem_t *m, f3regs_t *r)
{
    /* 0x13D1A2..0x13D1AE: select the record list through the PC-relative long table. */
    int16_t index = (int16_t)rd16(m, F3_A5 + 0x32DCu);
    uint32_t table_addr = (uint32_t)(0x13D226u + (int32_t)index * 4);
    uint32_t list = rd32(m, table_addr);
    uint32_t last_step = r->d[3];

    /* 0x13D1B4: a negative high word marks the end of a six-byte record list. */
    for (;;) {
        if ((int16_t)rd16(m, list) < 0)
            break;
        uint32_t source = rd32(m, list);          /* 0x13D1BA movea.l (A0)+,A2 */
        uint16_t palette = rd16(m, list + 4u);    /* 0x13D1C0 move.w (A0)+,D0 */

        /* 0x13D1E6..0x13D224: preserve byte zero and scale bytes 1..3 by 11/16. */
        for (uint32_t i = 0; i < 32u; ++i) {
            uint32_t raw = rd32(m, source + 4u * i);
            uint32_t out = rd32(m, 0x0041B58Eu + 4u * i);
            for (uint32_t byte = 1; byte <= 3u; ++byte) {
                /* 68k bitfield offset 8 selects bits 23..16 of the long. */
                uint32_t shift = 24u - 8u * byte;
                uint8_t component = f13d19e_component((uint8_t)(raw >> shift));
                out = (out & ~(0xFFu << shift)) | ((uint32_t)component << shift);
            }
            wr32(m, 0x0041B58Eu + 4u * i, out);
        }

        /* The final DIVSL in 0x13D1E6 is the low-byte ramp step for colour 31. */
        uint8_t raw_last = (uint8_t)rd32(m, source + 31u * 4u);
        uint8_t converted_last = f13d19e_component(raw_last);
        last_step = (uint32_t)(((int32_t)raw_last - (int32_t)converted_last) * 256 / 0x3C);

        /* 0x13D1C2..0x13D1DE calls 0x183C(A1,A2,D0,0x3C,0x20). */
        (void)Pal_QueueRamp(m, 0x0041B58Eu, source, palette, 0x3Cu, 0x20u);
        list += 6u;
    }

    /* 0x13D1E0 MOVEM restores D0,D1,A0,A1,A2; D3 survives the converter. */
    r->d[3] = last_step;
    return 0;
}
