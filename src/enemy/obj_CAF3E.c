#include "obj_CAF3E.h"
#include "../video/sprite.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

/* ROM 0x0CB14A */
static void sub_0CB14A(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0x00A0u);
    wr16(m, obj + 0x18u, 0x0074u);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (uint16_t)ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    d0 = (uint16_t)(0u - (uint16_t)(d0 << 2));
    wr16(m, F3_A5 + 0x1D5Cu, d0);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x0CAF3E */
int Obj_CAF3E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Eu, (uint16_t)(rd16(m, obj + 0x1Eu) - 0x1010u));
    sub_0CB14A(m, obj);
    wr32(m, obj + 0x10u, 0x00061AC4u);
    wr16(m, obj + 0x1Eu, (uint16_t)(int16_t)-0x2728);
    wr32(m, obj + 0x00u, 0x000CAF5Eu);
    return 0;
}
