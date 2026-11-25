#include "obj_14D86E.h"

/* ROM 0x14D86E: 9-frame timer (+6). On expiry, paints the 0xE8-word line-ram
 * block at 0x626200 with one fade word derived from the +0x44 counter, bumps
 * the counter and re-arms the timer; the tick fn stays 0x14D86E. */
int Obj_14D86E_Tick(const f3mem_t *m, uint32_t obj)
{
    int32_t t = (int32_t)(int16_t)rd16(m, obj + 0x06u) - 1;
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0014D86Eu);                       /* ROM 0x14D866 */
        return 0;
    }

    /* ROM 0x14D874 */
    int16_t d0 = (int16_t)(((rd32(m, obj + 0x44u) + 8u) & 0xFu) - 8u);
    if (d0 < 0) d0 = (int16_t)-d0;
    d0 = (int16_t)(d0 - 3);
    if (d0 < 0) d0 = 0;
    int16_t d1 = 4;

    /* ROM 0x14D892 */
    uint32_t a0 = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    d1 &= 0xF; if (d1 > 8) d1 = 8;
    d0 &= 0xF; if (d0 > 8) d0 = 8;
    uint16_t w1 = (uint16_t)((((uint16_t)d1 << 8) + (uint16_t)(8 - d1)) << 4);
    uint16_t w0 = (uint16_t)(((uint16_t)d0 << 8) + (uint16_t)(8 - d0));
    uint16_t v = (uint16_t)~(uint16_t)(w1 | w0);
    for (int i = 0xE7; i >= 0; i--)
        wr16(m, a0 + (uint32_t)i * 2u, v);

    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 1u);             /* ROM 0x14D8F6 */
    wr16(m, obj + 0x06u, 9u);                                    /* ROM 0x14D85E */
    wr32(m, obj + 0x00u, 0x0014D86Eu);
    return 0;
}
