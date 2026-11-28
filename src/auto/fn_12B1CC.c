/* ROM 0x12B1CC: aim helper. D0/D1 carry a pixel position; the reference point
 * is the centre of the rectangle of the object A5+0x30E2 owns: the midpoints of
 * (+0x2C,+0x2E) and (+0x30,+0x32) added to (+0x14,+0x18). Returns in D0.b the
 * 0x006422 angle from that centre to (D0, D1), biased by -0x80 so 0 is up.
 * D1/D2/D3/A1 are saved and restored (0x12B1CC/0x12B1FE); D0 keeps its high
 * word because 0x006422 only rewrites D0's low byte (0x00647A/0x006488). */
#include "fn_auto.h"
#include "../objects/obj_shared.h"

int Fn_12B1CC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x30E2u);                   /* 0x12B1D0 */

    /* 0x12B1D4: (int16)((+0x2C + +0x2E) >> 1) + +0x14 */
    uint16_t s2 = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu));
    int16_t d2 = (int16_t)((int16_t)s2 >> 1);
    d2 = (int16_t)(d2 + (int16_t)rd16(m, a1 + 0x14u));

    /* 0x12B1E2: (int16)((+0x30 + +0x32) >> 1) + +0x18 */
    uint16_t s3 = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u));
    int16_t d3 = (int16_t)((int16_t)s3 >> 1);
    d3 = (int16_t)(d3 + (int16_t)rd16(m, a1 + 0x18u));

    uint16_t dx = (uint16_t)((uint16_t)r->d[0] - (uint16_t)d2); /* 0x12B1F0 */
    uint16_t dy = (uint16_t)((uint16_t)r->d[1] - (uint16_t)d3); /* 0x12B1F2 */
    uint16_t ang = Obj_Atan2(m, dx, dy);                       /* 0x12B1F4 */

    /* 0x12B1FA: addi.b #-0x80,D0b -- low byte only, high word kept. */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint8_t)((uint8_t)ang - 0x80u);
    return 0;
}
