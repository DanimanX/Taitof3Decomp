#include "obj_D901A.h"
#include "../video/sprite.h"

/* ROM 0x0CEDE4 */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0D0678 */
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

/* ROM 0x0D0CB0 */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    /* PC-relative table at 0x0D0CBA + 0x00C8 = 0x0D0D82 */
    return rd16(m, 0x000D0D82u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D1EBC */
static int sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    /* PC-relative tables at 0x0D1EC2 + 0x0182 = 0x0D2044 and 0x0D1ECA + 0x12 = 0x0D1EDC */
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) mod 2^32 */
    if (d2w > 50u)
        return -1;

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0D2D44 */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    /* PC-relative tables at 0x0D2D4A + 0x0182 = 0x0D2ECC and 0x0D2D52 + 0x12 = 0x0D2D64 */
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) mod 2^32 */
    if (d2w > 50u)
        return -1;

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0D901A */
int Obj_D901A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* Pre-check speed index (ROM 0x0D909A..0x0D90E4) before any side effects */
    uint16_t speed = (uint16_t)(sub_0D0CB0(m) + 0x000Du);
    if (speed > 50u)
        return -1;

    /* ROM 0x0D901A..0x0D9052 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    uint32_t d0 = rd32(m, obj + 0x0Cu);
    int16_t dx = (int16_t)(int8_t)(d0 >> 24);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)dx));
    int16_t dy = (int16_t)(int8_t)(d0 >> 16);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)dy));

    uint8_t ang8 = (uint8_t)(0u - (uint8_t)((d0 >> 8) & 0xFFu));
    uint32_t ang = ((uint32_t)ang8 * 0x0168u) >> 8;
    wr16(m, obj + 0x58u, (uint16_t)ang);
    wr16(m, obj + 0x10u, 0u);

    /* ROM 0x0D9092..0x0D9114 */
    /* 0x0D9092: subi.l #0x0, (0x886,A5) (no-op) */
    wr16(m, obj + 0x60u, speed);

    uint32_t vx, vy;
    if (sub_0D1EBC(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x10u), &vx, &vy) < 0)
        return -1;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));

    /* 0x0D90C6: lea (0x100,PC),A1 -> 0x0D90C8 + 0x100 = 0x0D91C8 */
    sub_0CEDE4(m, obj, 0x000D91C8u);

    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));

    if (sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &vx, &vy) < 0)
        return -1;
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);

    /* 0x0D90F2: lea (0xf4,PC),A1 -> 0x0D90F4 + 0xf4 = 0x0D91E8 */
    sub_0D0678(m, obj, 0x000D91E8u);

    uint32_t scroll_y = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - scroll_y);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - scroll_y);

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000D9116u);
    return 0;
}
