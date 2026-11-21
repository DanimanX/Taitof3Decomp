#include "obj_EA848.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}


/* ROM 0x0D4A2C: (D0w, D1w) = high words of tbl[D2] * k0, tbl[D2] * k1
 * (jump table 0x0D4E5A [0..50] -> 0x0D4FF4..0x0D5396: exact D0 * k mod 2^32) */
static int sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                      uint16_t *out_d0, uint16_t *out_d1)
{
    if (d0w > 50u || d1w > 50u)
        return -1; /* unimplemented: ROM 0x0D4E5A entries 51..63 */
    uint32_t d4 = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)d2w * 4));
    *out_d1 = (uint16_t)((uint32_t)(d4 * (uint32_t)d1w) >> 16);
    *out_d0 = (uint16_t)((uint32_t)(d4 * (uint32_t)d0w) >> 16);
    return 0;
}

/* ROM 0x0EA848 */
int Obj_EA848_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x1Eu));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));

    uint16_t d0w, d1w;
    sub_0D4A2C(m, 0x0020u, 0x0019u, (uint16_t)rd8(m, obj + 0x1Eu), &d0w, &d1w);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d1w));

    uint8_t d0b = rd8(m, a1 + 0x22u);
    if (d0b != 0u) {
        wr8(m, a1 + 0x22u, 0u);
        /* movea.l (-0x598,PC,D0w*4),A1 at 0xEA890 -> table 0xEA2FA */
        sub_0D0678(m, obj, rd32(m, 0x000EA2FAu + (uint32_t)d0b * 4u));
    }

    sub_0D0690(m, obj);
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    return 0;
}
