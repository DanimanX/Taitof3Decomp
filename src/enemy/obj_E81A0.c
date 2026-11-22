#include "obj_E81A0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
/* ROM 0x0CEC4C: base state + the per-group script chunk at A1 */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);                  /* clr.l (0x1c,A0) */
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));  /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    /* 0x0CEC92: group 5 keeps its own chunk, others use group+0x20 */
    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u);
    wr32(m, obj + 0x0Cu, a2);

    /* 0x0CECB2 */
    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
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
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0D0E52 */
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = (d1 > d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)((uint16_t)(d1 << 6) + d0);
    d0 = (uint16_t)rd8(m, 0x0D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x0D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x0D2D44: (D0, D1) = per-mode speed tables scaled by D2w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    /* jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) */
    uint32_t d0 = rd32(m, 0x000D2D64u + (uint32_t)((int32_t)(int16_t)d0w * 4));
    uint32_t d1 = rd32(m, 0x000D2ECCu + (uint32_t)((int32_t)(int16_t)d0w * 4));
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}


/* ROM 0x0D06E0: +0x42 = angle to the nearer live player (0xB4 if none) */
static void sub_0D06E0(const f3mem_t *m, uint32_t obj)
{
    static const uint32_t pl[2] = { 0x41B190u, 0x41B21Au };
    int16_t ang[2] = { -1, -1 };
    uint16_t dist[2] = { 0, 0 };
    wr16(m, obj + 0x42u, 0xB4u);
    for (int i = 0; i < 2; i++) {
        uint32_t a1 = pl[i];
        if ((int16_t)rd16(m, a1 + 0x4Cu) < 0)
            continue;
        uint32_t a2 = rd32(m, obj + 0x68u);
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, a1 + 0x14u) + rd16(m, a2) - 0x17u);
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, a1 + 0x18u) + rd16(m, a2 + 2u) - 0x0Fu);
        uint16_t ax = (int16_t)dx < 0 ? (uint16_t)-dx : dx;
        uint16_t ay = (int16_t)dy < 0 ? (uint16_t)-dy : dy;
        ang[i] = (int16_t)sub_0D0E52(m, dx, dy);
        dist[i] = ax > ay ? ax : ay;
    }
    if (ang[0] < 0) {
        if (ang[1] >= 0)
            wr16(m, obj + 0x42u, (uint16_t)ang[1]);
        return;
    }
    if (ang[1] >= 0 && dist[1] < dist[0])
        wr16(m, obj + 0x42u, (uint16_t)ang[1]);
    else
        wr16(m, obj + 0x42u, (uint16_t)ang[0]);
}

/* ROM 0x0E81A0: init, aimed at the nearer player, fn = 0xE8240 */
int Obj_E81A0_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CEC4C(m, obj, 0xE8362u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr16(m, obj + 0x44u, 0x1A4u);
    uint32_t a1 = (sub_0D0E3A(m) & 0x80u) ? 0xE8406u : 0xE839Au;
    sub_0D0678(m, obj, a1);
    sub_0D06E0(m, obj);
    wr16(m, obj + 0x58u, rd16(m, obj + 0x42u));
    wr16(m, obj + 0x60u, (uint16_t)((sub_0D0E3A(m) & 3u) + 8u));
    uint16_t d0 = (uint16_t)((sub_0D0E3A(m) & 0x3Fu) - 0x20u + rd16(m, obj + 0x58u));
    if ((int16_t)d0 < 0)
        d0 = (uint16_t)(d0 + 0x168u);
    else if ((int16_t)d0 >= 0x168)
        d0 = (uint16_t)(d0 - 0x168u);
    wr16(m, obj + 0x58u, d0);
    uint32_t vx, vy;
    sub_0D2D44(m, d0, rd16(m, obj + 0x60u), &vx, &vy);
    wr32(m, obj + 0x50u, vx);
    wr32(m, obj + 0x54u, vy);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj, 0xE8240u);
    return 0;
}
