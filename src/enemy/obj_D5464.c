#include "obj_D5464.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0D545C */
static void sub_0D545C(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x000D5464u);
}

/* ROM 0x0CBEC2 */
static void sub_0CBEC2(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint32_t d1w = rd16(m, obj + 0x42u);
    uint32_t d2w = d1w;
    int32_t prod1 = (int32_t)(int16_t)d1w
                  * (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d0 * 2u);
    wr32(m, obj + 0x54u, (uint32_t)prod1);
    int32_t prod2 = (int32_t)(int16_t)d2w
                  * (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)d0 * 2u);
    wr32(m, obj + 0x58u, (uint32_t)prod2);
}

/* ROM 0x0CBF7A */
static void sub_0CBF7A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x00028740u);
    wr32(m, obj, 0x000CBF8Au);
}

/* ROM 0x0D5464 */
int Obj_D5464_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {
        sub_0D545C(m, obj);
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x08u);
    a1 = rd32(m, a1 + 0x50u);
    if ((int16_t)rd16(m, a1 + 0x18u) < 0x6C) {
        wr32(m, obj + 0x18u, 0x00D40000u);
        wr8(m, obj + 0x64u, 3u);
    } else {
        wr32(m, obj + 0x18u, 0x00040000u);
        wr8(m, obj + 0x64u, 1u);
    }

    uint16_t d0 = (uint16_t)Rng_NextMem(m) & 0x3Fu;
    d0 = (uint16_t)(d0 - 0x20u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));

    wr32(m, obj + 0x14u, 0xFFE00000u);
    wr16(m, obj + 0x20u, 0x0800u);

    sub_0CBEC2(m, obj);

    wr32(m, obj + 0x50u, 0x000D54D0u);
    sub_0CBF7A(m, obj);
    return 0;
}
