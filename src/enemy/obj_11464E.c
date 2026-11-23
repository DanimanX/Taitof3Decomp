#include "obj_11464E.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x0CBEB0: D0w = high word of ((d0 * (A5+0x87E * 8 + 0xC00)) << 4) */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)d0w * (int16_t)d1w);
    d0 <<= 4;
    return (uint16_t)(d0 >> 16);
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

/* ROM 0x114632 */
static int retarget(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, sub_0CBEB0(m, 0x0064u));
    wr32(m, obj + 0x3Au, rd32(m, F3_A5 + 0x1D28u));
    wr32(m, obj + 0x00u, 0x0011464Eu);
    return 0;
}

/* ROM 0x11464E */
int Obj_11464E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x1147E4 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        bset8(m, obj + 0x50u, 4);
        /* move.w #0x78,(0x6,A0); beq: never taken */
        wr16(m, obj + 0x06u, 0x0078u);
        wr32(m, obj + 0x00u, 0x00114800u);
        return 0;
    }
    if (rd8(m, obj + 0x50u) & 0x08u) {
        /* 0x114732 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr16(m, obj + 0x42u, 0x0006u);
        wr16(m, obj + 0x06u, 0x04B0u);
        wr32(m, obj + 0x00u, 0x0011474Cu);
        return 0;
    }
    Obj_BlinkColour(m, obj);

    if (!(rd8(m, obj + 0x50u) & 0x01u)) {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x56u) - 1u);
        wr16(m, obj + 0x56u, t);
        if ((int16_t)t > 0) {
            /* 0x11469C */
            uint32_t a1 = rd32(m, obj + 0x3Au);
            if (a1 == 0u)
                return retarget(m, obj);
            if ((rd16(m, F3_A5 + 0x0878u) & 1u) == 0u) {
                if ((int16_t)rd16(m, obj + 0x18u) <= (int16_t)rd16(m, a1 + 0x18u))
                    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 2u));
                wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
                if ((int16_t)rd16(m, obj + 0x42u) < -0x0A)
                    wr16(m, obj + 0x42u, (uint16_t)-0x0A);
                if ((int16_t)rd16(m, obj + 0x42u) > 0x0A)
                    wr16(m, obj + 0x42u, 0x000Au);
            }
        } else {
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x01A4) {
                /* 0x114806 */
                bset8(m, obj + 0x50u, 5);
                wr32(m, obj + 0x00u, 0x00114814u);
                return 0;
            }
            wr16(m, obj + 0x20u, 0x00F0u);
            wr16(m, obj + 0x42u, 0x0004u);
        }

        /* 0x1146E4 */
        Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
        if ((int16_t)rd16(m, obj + 0x56u) > 0) {
            uint16_t c = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
            wr16(m, obj + 0x06u, c);
            if ((int16_t)c <= 0) {
                if (sub_0CC2BC(m, 0x0014u) == 0u)
                    bset8(m, obj + 0x51u, 0);
                bset8(m, obj + 0x51u, 1);
                wr16(m, obj + 0x42u, 0u);
            }
        }
    }

    /* 0x11471E */
    sub_113456(m, obj);
    if ((int16_t)rd16(m, obj + 0x06u) <= -0x5A)
        return retarget(m, obj);
    wr32(m, obj + 0x00u, 0x0011464Eu);
    return 0;
}
