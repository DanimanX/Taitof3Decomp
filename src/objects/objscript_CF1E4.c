#include "objscript_CF1E4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0D25E4 */
static int sub_0D25E4(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;

    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }

    uint16_t d2w = d1w;
    int32_t idx = (int32_t)(int16_t)d0w;
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2044u + (uint32_t)idx * 4u));
    uint32_t d0 = rd32(m, (uint32_t)(0x000D1EDCu + (uint32_t)idx * 4u));

    /* Jump table 0x0D4F26 ([0..50] at 0x0D4FF2..0x0D538A): exact (D0*k, D1*k) mod 2^32 */
    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    uint32_t d3 = d0;
    d2w = d4w;
    uint32_t d4 = d1;

    d1 = rd32(m, (uint32_t)(0x000D27A4u + (uint32_t)idx * 4u));
    d0 = rd32(m, (uint32_t)(0x000D263Cu + (uint32_t)idx * 4u));

    if (d2w > 50u) {
        /* unimplemented: ROM 0x0D4F26 */
        return -1;
    }
    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    *out_d0 = (uint32_t)(d0 + d3);
    *out_d1 = (uint32_t)(d1 + d4);
    return 0;
}

/* ROM 0x0D346C */
static uint32_t sub_0D346C(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    uint32_t a0 = rd32(m, (uint32_t)(0x000D348Eu + (uint32_t)(int32_t)(int16_t)d1w * 4u));
    if ((int16_t)d0w < 0) {
        d0w = (uint16_t)(0u - d0w);
        uint32_t d0 = rd32(m, (uint32_t)(a0 + (uint32_t)(int32_t)(int16_t)d0w * 4u));
        return (uint32_t)(0u - d0);
    }
    return rd32(m, (uint32_t)(a0 + (uint32_t)(int32_t)(int16_t)d0w * 4u));
}

/* ROM 0x0CF1E4 */
int ObjScript_CF1E4(const f3mem_t *m, uint32_t obj)
{
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint16_t op = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x20u, (uint8_t)op);

        switch (op) {
        case 0u: /* ROM 0x0CF278 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x80u));
            break;

        case 1u: /* ROM 0x0CF280 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u | 0x04u));
            break;

        case 2u: /* ROM 0x0CF28E */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x40u | 0x04u));
            break;

        case 3u: /* ROM 0x0CF2A2 */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
            wr8(m, obj + 0x07u, (uint8_t)((rd8(m, obj + 0x07u) & ~0x40u) | 0x04u));
            break;

        case 4u: /* ROM 0x0CF2B6 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x20u | 0x04u));
            break;

        case 5u: /* ROM 0x0CF2C4 */
            wr8(m, obj + 0x07u, (uint8_t)((rd8(m, obj + 0x07u) & ~0x20u) | 0x04u));
            break;

        case 6u: /* ROM 0x0CF2D2 */
            wr32(m, obj + 0x58u, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 7u: /* ROM 0x0CF2DE */
            wr32(m, obj + 0x5Cu, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 8u: /* ROM 0x0CF2EA */
            wr32(m, obj + 0x60u, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 9u: { /* ROM 0x0CF2F6 */
            uint16_t d0w = (uint16_t)rd8(m, F3_A5 + 0x087Fu);
            d0w = (uint16_t)(d0w + 1u);
            d0w = (uint16_t)(d0w << 2);
            d0w = (uint16_t)(d0w + 0x0C00u);
            uint32_t d0 = ((uint32_t)d0w * (uint32_t)rd16(m, a1)) << 4;
            wr32(m, obj + 0x60u, d0);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 10u: /* ROM 0x0CF314 */
            wr32(m, obj + 0x64u, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 11u: { /* ROM 0x0CF320 */
            uint32_t d0 = (uint32_t)rd8(m, F3_A5 + 0x087Fu);
            d0 = ((d0 + 1u) >> 2) + 0xC0u;
            uint32_t d1 = rd32(m, a1);
            if ((int32_t)d1 < 0) {
                d1 = (uint32_t)(0u - d1);
                d0 = (uint32_t)(d0 * d1);
                d0 >>= 8;
                d0 = (uint32_t)(0u - d0);
            } else {
                d0 = (uint32_t)(d0 * d1);
                d0 >>= 8;
            }
            wr32(m, obj + 0x64u, d0);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 12u: { /* ROM 0x0CF35E */
            uint32_t d0_out, d1_out;
            if (sub_0D25E4(m, rd16(m, obj + 0x58u), rd16(m, a1), &d0_out, &d1_out) < 0)
                return -1;
            wr32(m, obj + 0x50u, d0_out);
            wr32(m, obj + 0x54u, d1_out);
            break;
        }

        case 13u: { /* ROM 0x0CF374 */
            uint16_t d0w = rd16(m, a1);
            a1 += 2u;
            uint16_t d1w = rd16(m, obj + 0x60u);
            uint32_t res = sub_0D346C(m, d0w, d1w);
            wr32(m, obj + 0x5Cu, res);
            wr32(m, obj + 0x50u, 0u);
            wr16(m, obj + 0x50u, rd16(m, a1));
            break;
        }

        case 14u: /* ROM 0x0CF38E */
        case 15u: /* ROM 0x0CF38E */
        case 20u: /* ROM 0x0CF38E */
            wr16(m, obj + 0x50u, rd16(m, a1));
            break;

        case 16u: /* ROM 0x0CF394 */
            wr8(m, obj + 0x21u, (uint8_t)rd16(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 17u: { /* ROM 0x0CF3A2 */
            uint8_t cnt = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
            wr8(m, obj + 0x21u, cnt);
            if (cnt != 0u)
                wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 18u: /* ROM 0x0CF3B8 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u | 0x04u));
            break;

        case 19u: /* ROM 0x0CF3C6 */
            wr8(m, obj + 0x07u, (uint8_t)((rd8(m, obj + 0x07u) | 0x08u) & ~0x80u));
            break;

        case 21u: /* ROM 0x0CF3D4 */
            wr16(m, obj + 0x1Eu, rd16(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 22u: { /* ROM 0x0CF3E0 */
            uint8_t cur = rd8(m, obj + 0x1Eu);
            uint8_t delta = rd8(m, a1);
            if (cur < delta) {
                wr16(m, obj + 0x1Eu, 0u);
            } else {
                uint8_t nv = (uint8_t)(cur - delta);
                wr8(m, obj + 0x1Eu, nv);
                wr8(m, obj + 0x1Fu, nv);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 23u: { /* ROM 0x0CF406 */
            unsigned sum = (unsigned)rd8(m, obj + 0x1Eu) + (unsigned)rd8(m, a1);
            if (sum > 0xFFu) {
                wr16(m, obj + 0x1Eu, 0xFFFFu);
            } else {
                wr8(m, obj + 0x1Eu, (uint8_t)sum);
                wr8(m, obj + 0x1Fu, (uint8_t)sum);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 24u: { /* ROM 0x0CF42E */
            uint8_t cur = rd8(m, obj + 0x1Eu);
            uint8_t delta = rd8(m, a1);
            if (cur < delta) {
                wr16(m, obj + 0x1Eu, 0u);
            } else {
                uint8_t nv = (uint8_t)(cur - delta);
                wr8(m, obj + 0x1Eu, nv);
                wr8(m, obj + 0x1Fu, nv);
                wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - (rd32(m, a1) & 0xFFFFu));
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 25u: { /* ROM 0x0CF460 */
            unsigned sum = (unsigned)rd8(m, obj + 0x1Eu) + (unsigned)rd8(m, a1);
            if (sum > 0xFFu) {
                wr16(m, obj + 0x1Eu, 0xFFFFu);
            } else {
                wr8(m, obj + 0x1Eu, (uint8_t)sum);
                wr8(m, obj + 0x1Fu, (uint8_t)sum);
                wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - (rd32(m, a1) & 0xFFFFu));
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 26u: /* ROM 0x0CF494 */
            wr8(m, obj + 0x06u, rd8(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 27u: /* ROM 0x0CF4A0 */
            wr8(m, obj + 0x22u, rd8(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 28u: /* ROM 0x0CF4AC */
            wr8(m, obj + 0x23u, rd8(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        default:
            /* unimplemented: ROM 0x0CF204 */
            return -1;
        }

        wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 6u);
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x04u));
        if ((f7 & 0x04u) == 0u)
            break;
    }

    return 0;
}
