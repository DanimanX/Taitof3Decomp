#include "obj_11AB96.h"
#include "../objects/obj_pool.h"

/* ROM 0x11AB96 */
int Obj_11AB96_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11AB96: subq.w #1,(0x6,A0); bgt -> 0x11ab8e */
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0011AB96u);      /* 0x11AB8E */
        return 0;
    }

    /* 0x11AB9C: 0xE8 words of char RAM and line RAM, biased by A5-0x5d56 */
    int16_t bias = (int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a1 = 0x00626000u + (uint32_t)(int32_t)bias;
    uint32_t a2 = 0x00627400u + (uint32_t)(int32_t)bias;
    for (int i = 0; i < 0xE8; i++) {
        uint16_t d1 = rd16(m, a1);
        uint16_t d2 = rd16(m, a2);
        d1 |= 0x0055u;
        d2 &= 0x0FFFu;
        wr16(m, a2, d2);
        a2 += 2;
        wr16(m, a1, d1);
        a1 += 2;
    }

    /* 0x11ABC8: move.l A0,-(SP); jsr 0x105EC; lea (0x4,SP),SP */
    Obj_Free(m, obj);
    return 0;
}
