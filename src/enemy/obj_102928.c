#include "obj_102928.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CFA86 */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D4A12 */
static int sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t *out_d0)
{
    uint32_t d0 = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)d1w * 4));
    if (d0w > 50u) {
        /* unimplemented: ROM 0x0D4E5A */
        return -1;
    }
    /* Jump table 0x0D4E5A [0..50] -> 0x0D4FF4..0x0D5396: exact D0 * k mod 2^32 */
    d0 = (uint32_t)(d0 * (uint32_t)d0w);
    *out_d0 = (uint16_t)(d0 >> 16);
    return 0;
}

/* ROM 0x102984 */
static int sub_102984(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));
    uint8_t b1e = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x0Fu);
    wr8(m, obj + 0x1Eu, b1e);
    if (b1e == 0xFFu)
        wr8(m, obj + 0x06u, 1u);

    uint16_t off = 0u;
    if (sub_0D4A12(m, 0x001Du, (uint16_t)(uint8_t)(~b1e), &off) < 0)
        return -1;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + off));
    return 0;
}

/* ROM 0x1029B4 */
static void sub_1029B4(const f3mem_t *m, uint32_t obj)
{
    sub_0CFA86(m, obj, 0x00102A66u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0017u));
    uint8_t b1e = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x0Fu);
    wr8(m, obj + 0x1Eu, b1e);
    if (b1e == 0u) {
        wr8(m, obj + 0x23u, 6u);
        wr8(m, obj + 0x06u, 2u);
    }
}

/* ROM 0x1029DC */
static void sub_1029DC(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0017u));
    uint8_t b23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, b23);
    if (b23 == 0u)
        wr8(m, obj + 0x06u, 3u);
}

/* ROM 0x1029F2 */
static void sub_1029F2(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0017u));
    uint8_t b1e = (uint8_t)(rd8(m, obj + 0x1Eu) + 0x0Fu);
    wr8(m, obj + 0x1Eu, b1e);
    if (b1e == 0xFFu)
        wr8(m, obj + 0x06u, 4u);
}

/* ROM 0x102A10 */
static int sub_102A10(const f3mem_t *m, uint32_t obj)
{
    sub_0CFA86(m, obj, 0x00102A5Cu);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));
    uint8_t b1e = (uint8_t)(rd8(m, obj + 0x1Eu) - 0x0Fu);
    wr8(m, obj + 0x1Eu, b1e);
    if (b1e == 0u) {
        wr8(m, obj + 0x23u, 6u);
        wr8(m, obj + 0x06u, 5u);
    }

    uint16_t off = 0u;
    if (sub_0D4A12(m, 0x001Du, (uint16_t)(uint8_t)(~b1e), &off) < 0)
        return -1;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + off));
    return 0;
}

/* ROM 0x102A4A */
static void sub_102A4A(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));
    uint8_t b23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, b23);
    if (b23 == 0u)
        wr8(m, obj + 0x06u, 0u);
}

/* ROM 0x102928 */
int Obj_102928_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x3Au));
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x3Eu));
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    uint8_t b22 = rd8(m, a1 + 0x22u);
    wr8(m, obj + 0x22u, b22);
    if (b22 == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0011u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u:
        return sub_102984(m, obj);
    case 1u:
        sub_1029B4(m, obj);
        return 0;
    case 2u:
        sub_1029DC(m, obj);
        return 0;
    case 3u:
        sub_1029F2(m, obj);
        return 0;
    case 4u:
        return sub_102A10(m, obj);
    case 5u:
        sub_102A4A(m, obj);
        return 0;
    default:
        /* unimplemented: ROM 0x10296C */
        return -1;
    }
}
