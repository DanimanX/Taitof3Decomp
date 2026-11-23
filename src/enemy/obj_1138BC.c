#include "obj_1138BC.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
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
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x0CBE34 */

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % d1w);
}

/* ROM 0x113456: during a bomb, scatter away from the screen centre */
static void sub_113456(const f3mem_t *m, uint32_t obj)
{
    if (!(rd8(m, F3_A5 + 0x1D58u) & 0x02u))
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -0x0A || x > 0x011C)
        return;
    uint16_t saved = rd16(m, obj + 0x42u);
    uint16_t r = (uint16_t)Rng_NextMem(m);
    wr16(m, obj + 0x42u, (uint16_t)((r & 0x000Fu) - 5u));
    uint16_t d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u),
                             (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u));
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + 0x58u));
    Obj_MoveByAngle(m, obj, d0);
    wr16(m, obj + 0x42u, saved);
}

static void bset8(const f3mem_t *m, uint32_t a, unsigned bit)
{ wr8(m, a, (uint8_t)(rd8(m, a) | (1u << bit))); }

static void wr_state(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    wr32(m, obj + 0x00u, fn);
}

/* ROM 0x1138BC: steer towards the waypoint (+0x52, +0x54) for +0x56 frames,
 * then leave to the right; new random waypoint every 0x64 frames */
int Obj_1138BC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x113A10 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        bset8(m, obj + 0x50u, 4u);
        wr16(m, obj + 0x06u, 0x78u);
        wr_state(m, obj, 0x00113A2Cu);
        return 0;
    }
    if (rd8(m, obj + 0x50u) & 0x08u) {
        /* 0x113988 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr16(m, obj + 0x06u, 0x04B0u);
        wr_state(m, obj, 0x0011399Cu);
        return 0;
    }
    Obj_BlinkColour(m, obj);
    if (!(rd8(m, obj + 0x50u) & 0x01u)) {
        uint16_t d0;
        int16_t t = (int16_t)(rd16(m, obj + 0x56u) - 1u);
        wr16(m, obj + 0x56u, (uint16_t)t);
        if (t > 0) {
            d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, obj + 0x14u)),
                            (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u)));
        } else {
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x01A4) {
                /* 0x113A32 */
                bset8(m, obj + 0x50u, 5u);
                Obj_Free(m, rd32(m, obj + 0x08u));
                wr_state(m, obj, 0x00113A4Eu);
                return 0;
            }
            d0 = 0x00F0u;
        }
        /* 0x113918 */
        d0 = (uint16_t)(d0 - rd16(m, obj + 0x20u));
        uint8_t a = rd8(m, obj + 0x21u);
        wr8(m, obj + 0x21u, (uint8_t)((int8_t)d0 < 0 ? a - 3u : a + 3u));
        Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
        if ((int16_t)rd16(m, obj + 0x42u) > 6)
            wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 1u));
        if ((int16_t)rd16(m, obj + 0x56u) > 0 && rd8(m, obj + 0x22u) != 0u
            && sub_0CC2BC(m, 0x28u) == 0u)
            bset8(m, obj + 0x51u, 0u);
    }
    /* 0x11396C */
    sub_113456(m, obj);
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n <= 0) {
        wr8(m, obj + 0x22u, 0xFFu);
        wr16(m, obj + 0x06u, 0x64u);
        /* 0x113890 */
        wr16(m, obj + 0x52u, (uint16_t)((Rng_NextMem(m) & 0x0Fu) + 0xE6u));
        wr16(m, obj + 0x54u, (uint16_t)((Rng_NextMem(m) & 0x3Fu) + 0x50u));
    }
    wr_state(m, obj, 0x001138BCu);
    return 0;
}
