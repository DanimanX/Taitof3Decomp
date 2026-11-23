#include "obj_DBC1A.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CEDE4: stats + header init from the record at a1 (+0x14) */
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
    wr32(m, obj + 0x30u, rd32(m, a1));   /* move.l (A1),(0x30,A0): A1 not advanced */
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

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

/* ROM 0x0D0E3A */

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

/* ROM 0x0DBC1A: bullet launch with a random speed 0x1C..0x23 */
int Obj_DBC1A_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, obj + 0x10u) > 50u)
        return -1;
    /* subi.l #0,(0x886,A5): no effect */
    uint16_t speed = (uint16_t)((Obj_Next4Way(m) & 7u) + 0x1Cu);
    wr16(m, obj + 0x60u, speed);
    uint32_t vx = 0u, vy = 0u;
    sub_0D1EBC(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x10u), &vx, &vy);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    sub_0CEDE4(m, obj, 0x000D91C8u);
    sub_0D0678(m, obj, 0x000DBD72u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    sub_0D2D44(m, rd16(m, obj + 0x58u), speed, &vx, &vy);
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);
    uint32_t sx = rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - sx);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - sx);
    uint32_t sy = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - sy);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - sy);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000DBCAEu);
    return 0;
}
