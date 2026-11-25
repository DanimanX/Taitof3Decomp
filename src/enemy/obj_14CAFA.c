#include "obj_14CAFA.h"
#include "obj_14CB12.h"
#include "../common/f3mem.h"

/* ROM 0x14CAEE: step the line-RAM wave one column (0x14CA76) for x = 0xE7..0,
 * then, once the object's +0x44 counter passes -0xC0, advance the two scroll
 * longs at A5+0x4234 / A5+0x423C, restore A0 and re-arm 0x14CAFA. */
static void sub_14CA52(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x14CA52 */
    uint32_t a1 = F3_A5 + 0x3BE0u;
    uint32_t a2 = F3_A5 + 0x3DE0u;
    uint32_t a3 = 0x0062A600u;
    uint32_t a4 = 0x00628600u;
    a3 += (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    a4 += (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    /* ROM 0x14CA76 */
    uint16_t d7 = 0x00E7u;
    do {
        /* ROM 0x14CA76 */
        wr16(m, a1 + (uint32_t)d7 * 2u,
             (uint16_t)(rd16(m, a1 + (uint32_t)d7 * 2u) + 0x0040u));
        uint32_t d0 = (uint32_t)rd16(m, a1 + (uint32_t)d7 * 2u);   /* moveq #0 + move.w */
        d0 = ((d0 << 3) | (d0 & 0xFFFF0000u)) & 0xFFFFFFFFu;       /* lsl.l #3 */
        d0 = (d0 + 0x0000F000u) & 0xFFFFFFFFu;                     /* addi.l #0xF000 */
        wr32(m, a2 + (uint32_t)d7 * 4u, rd32(m, a2 + (uint32_t)d7 * 4u) + d0);
        uint16_t d4 = rd16(m, a2 + (uint32_t)d7 * 4u);

        /* ROM 0x14CA92: D0 = -0xA000 / (0xA0 + D4w) */
        uint16_t d1 = (uint16_t)(0x00A0u + d4);
        int16_t q = (d1 == 0u) ? 0 : (int16_t)(int32_t)(-0x0000A000 / (int32_t)(int16_t)d1);
        uint16_t d0w = (uint16_t)((uint16_t)q + 0x0100u);
        uint16_t d2 = (rd16(m, 0x004022A8u) != 0u) ? 0x0009u : 0x003Fu;

        /* ROM 0x14CABA */
        d2 = (uint16_t)(d2 - 3u);
        int32_t r = (int32_t)(int16_t)d2 * (int32_t)(int16_t)d0w;  /* muls.w */
        r = (r << 6) & 0xFFFFFFFFu;                                /* asl.l #6 */
        int16_t d2w = (int16_t)(r / 0x32);                         /* divs.w #0x32 */
        d2w = (int16_t)(d2w ^ 0x3F);                               /* eori.w #0x3F */
        wr16(m, a3 + (uint32_t)d7 * 2u, (uint16_t)d2w);
        wr8(m, a4 + (uint32_t)d7 * 2u, (uint8_t)d0w);
    } while (d7-- != 0u);

    /* ROM 0x14CAD4 */
    if ((int16_t)rd16(m, obj + 0x44u) > -0xC0) {
        wr32(m, F3_A5 + 0x4234u, rd32(m, F3_A5 + 0x4234u) + 0x00000780u);
        wr32(m, F3_A5 + 0x423Cu, rd32(m, F3_A5 + 0x423Cu) + 0x0000003Eu);
    }

    /* ROM 0x14CAEE */
    wr32(m, obj + 0x00u, 0x0014CAFAu);
}

/* ROM 0x14CAFA: wave scroll tamer. Counts +0x44 up and keeps the 0x14CA52
 * sweep going while it stays below 0; after that it arms a 0x68-frame delay
 * and hands over to 0x14CB12. */
int Obj_14CAFA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x14CAFA */
    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + 1u));
    if ((int16_t)rd16(m, obj + 0x44u) <= 0) {
        /* ROM 0x14CA52 */
        sub_14CA52(m, obj);
        return 0;
    }

    /* ROM 0x14CB02 */
    wr16(m, obj + 0x06u, 0x0068u);
    wr32(m, obj + 0x00u, 0x0014CB12u);            /* ROM 0x14CB0A */
    return 0;
}
