#include "obj_C9D7E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

/* ROM 0x0C9E44 */
static void sub_0C9E44(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    a1 = rd32(m, obj + 0x0Cu);
    int16_t dx = (int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)dx));

    int16_t dy = (int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)dy));

    uint16_t idx = (uint16_t)(rd8(m, a1++) & 0x03u);
    wr8(m, obj + 0x4Du, rd8(m, a1++));

    /* PC-relative table at 0x0C9E72 + 2 + 0x12 = 0x0C9E86 */
    wr16(m, obj + 0x20u, rd16(m, 0x000C9E86u + (uint32_t)idx * 4u));
    wr16(m, obj + 0x44u, rd16(m, 0x000C9E86u + 2u + (uint32_t)idx * 4u));
}

/* ROM 0x0C9E96 */
static void sub_0C9E96(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint16_t old_cnt = rd16(m, a1 + 0x5Cu);
    wr16(m, a1 + 0x5Cu, (uint16_t)(old_cnt - 1u));

    if ((int16_t)old_cnt < 1 || (rd8(m, a1 + 0x65u) & 0x01u) != 0u) {
        /* ROM 0x0CA00C */
        a1 = rd32(m, obj + 0x08u);
        wr16(m, a1 + 0x5Cu, (uint16_t)(rd16(m, a1 + 0x5Cu) + 1u));
        Obj_Free(m, obj);
        return;
    }

    wr8(m, obj + 0x22u, 0x04u);
    wr16(m, obj + 0x34u, 0x0000u);
    wr16(m, obj + 0x2Cu, 0x0002u);
    wr16(m, obj + 0x2Eu, 0x000Du);
    wr16(m, obj + 0x30u, 0x0002u);
    wr16(m, obj + 0x32u, 0x000Du);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00018000u);

    uint32_t d0 = ((uint32_t)rd16(m, a1 + 0x56u) << 16) >> 6;
    wr32(m, obj + 0x28u, rd32(m, obj + 0x28u) + d0);

    wr16(m, obj + 0x1Cu, 0x0000u);
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr16(m, obj + 0x3Au, 0x0000u);
    wr16(m, obj + 0x3Eu, 0x0000u);

    /* PC-relative table at 0x0C9EFC + 2 + 0xA12 = 0x0CA910 */
    uint32_t tbl = 0x000CA910u;
    if (rd8(m, obj + 0x4Eu) != 0u) {
        tbl += 0x40u;
    } else if (rd8(m, obj + 0x4Du) != 0u) {
        /* ROM 0x0C9F10 */
        tbl += 0x20u;
    }

    /* ROM 0x0C9F1C */
    wr32(m, obj + 0x48u, tbl);
    wr16(m, obj + 0x46u, 0x0000u);

    /* ROM 0x0C9F26 */
    wr32(m, obj, 0x000C9F2Eu);
}

/* ROM 0x0C9D7E */
int Obj_C9D7E_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0C9E44(m, obj);

    uint32_t a1 = rd32(m, obj + 0x08u);
    a1 = rd32(m, a1 + 0x44u);
    uint16_t d1 = 0u;
    uint16_t d0 = 0u;

    if ((rd8(m, a1) & 0x08u) == 0u) {
        d1 = (uint16_t)(-0x1000);
        d0 = (uint16_t)ASR16(rd16(m, obj + 0x44u), 1u);
    }

    /* ROM 0x0C9DA0 */
    if ((rd8(m, a1) & 0x04u) == 0u) {
        d1 = 0x0C00u;
        d0 = rd16(m, obj + 0x44u);
    }

    /* ROM 0x0C9DB0 */
    d0 = (uint16_t)ASR16(d0, 2u);
    if ((int16_t)rd16(m, obj + 0x20u) < 0)
        d1 = (uint16_t)(0u - d1);

    /* ROM 0x0C9DBC */
    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + d0));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d1));
    wr16(m, obj + 0x42u, 0x0008u);
    wr8(m, obj + 0x4Cu, 0x00u);
    wr8(m, obj + 0x4Eu, 0x00u);

    sub_0C9E96(m, obj);
    return 0;
}
