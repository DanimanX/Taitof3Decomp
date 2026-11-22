#include "obj_11B1BE.h"

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x11B1BE: aim at the linked object (offsets +0xC/+0xE), reset the
 * position, then re-arm as 0x11B1FA with a 0x10-frame timer. */
int Obj_11B1BE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x8u);
    uint16_t d0 = (uint16_t)(rd16(m, 0x40B0D2u) - rd16(m, a1 + 0x14u));
    d0 = (uint16_t)(d0 - rd16(m, obj + 0x0Cu));
    uint16_t d1 = (uint16_t)(rd16(m, 0x40B0D6u) - rd16(m, a1 + 0x18u));
    d1 = (uint16_t)(d1 - rd16(m, obj + 0x0Eu));

    wr16(m, obj + 0x20u, sub_006422(m, d0, d1));
    wr32(m, obj + 0x14u, 0u);   /* clr.l (0x14,A0): x and y together */
    wr32(m, obj + 0x18u, 0u);   /* clr.l (0x18,A0): y and zoom */
    wr16(m, obj + 0x06u, 0x0010u);
    wr32(m, obj, 0x0011B1FAu);  /* LAB_0011B1F2 */
    return 0;
}
