#include "obj_E04CC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

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

    /* cmp.w D0w,D1w / bgt: take bfffo of D1 when D1 > D0 */
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
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
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
    d0 = (uint16_t)(d0 + rd16(m, 0x006492u + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x00649A: D0.w = squared length (D0^2+D1^2 doubled, scaled),
 * D1.w = shift count (used as .l by the caller). */
static void sub_00649A(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)d0_in * (int32_t)(int16_t)d0_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)d1_in * (int32_t)(int16_t)d1_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    d0 = (d0 << 16) | (d0 >> 16);
    *out_d0 = d0;
    *out_d1 = shift;
}

/* ROM 0x0CBB2C */
static void sub_0CBB2C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x32u, 0x00u);
    wr8(m, obj + 0x34u, 0xFFu);

    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    int16_t off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)off));

    d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u);
    prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)off));

    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    uint32_t d0l, d1l;
    sub_00649A(m, dx, dy, &d0l, &d1l);
    d0 = (uint16_t)(d0l << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0 < (int16_t)thresh;
    d0 = (uint16_t)(d0 - thresh);
    if (is_lt) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    d0 = (uint16_t)(d0 << 6);
    wr16(m, obj + 0x2Cu, d0);

    d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u),
                    (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u));
    d0 = (uint16_t)(d0 << 8);
    wr16(m, obj + 0x2Eu, d0);
    wr16(m, obj + 0x30u, 0x0100u);

    if (!(rd8(m, F3_A5 + 0x1D58u) & 0x02u)) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0200u));

    uint8_t sh = rd8(m, obj + 0x32u);
    if (sh != 0u) {
        uint32_t tbl = rd32(m, obj + 0x0Cu);
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x2Eu) + 0x4800u);
        unsigned cnt = (unsigned)sh & 63u;
        d1 = (uint16_t)(d1 >> cnt);
        wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)((int32_t)(int16_t)d1 * 4)));
    }

    uint16_t rad_fp = rd16(m, obj + 0x2Cu);
    rad_fp = (uint16_t)(rad_fp - (uint16_t)(rad_fp >> 5));
    wr16(m, obj + 0x2Cu, rad_fp);

    uint32_t d1_l = 0x0100u * (uint32_t)rad_fp;
    int32_t d1_s = ASR32(d1_l, 8u);
    d1_s = ASR32(d1_s, 6u);
    uint16_t d1w = (uint16_t)((uint16_t)(uint32_t)d1_s + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t d2w = d1w;

    uint16_t ang = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    uint32_t x32 = (uint32_t)((int32_t)(int16_t)d1w * (int32_t)cos_v) + 0x00A00000u;
    wr32(m, obj + 0x14u, x32);

    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + (uint32_t)(int32_t)(int16_t)ang * 2u);
    uint32_t y32 = (uint32_t)((int32_t)(int16_t)d2w * (int32_t)sin_v) + 0x00740000u;
    wr32(m, obj + 0x18u, y32);

    int32_t z = (int32_t)(int16_t)(-0x0100) * (int32_t)(int16_t)rd16(m, obj + 0x30u);
    z = ASR32(z, 8u);
    uint16_t zw = (uint16_t)((uint16_t)(uint32_t)z + 0x0100u);
    wr8(m, obj + 0x1Fu, (uint8_t)zw);
    wr8(m, obj + 0x1Eu, (uint8_t)zw);

    a1 = rd32(m, obj + 0x10u);
    d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    off = ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)off));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)off));

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CBC78u);
}

/* ROM 0x0CBE34 */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
        return;
    }
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x08FEu, a1);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t t = rd8(m, obj + 0x35u);
    if (t == 0x01u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (t == 0x02u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */
static void sub_0D0E30(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x0CEF0E */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            sub_0CBB2C(m, obj);
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

/* ROM 0x0E04CC */
/* ROM 0x0D0678: start an animation sequence (A1 = table) */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 2u));
    wr32(m, obj + 0x10u, rd32(m, a1 + 4u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 8u));
}

/* ROM 0x0D0690: step the animation sequence (12-byte entries, negative
 * duration = loop back by the following word, or stop when it is 0) */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, t);
    if (t) return;
    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    if (!(d0 & 0x8000u)) {
        wr16(m, obj + 0x4Cu, d0);
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 2u));
        wr32(m, obj + 0x10u, rd32(m, a1 + 4u));
        wr32(m, obj + 0x68u, rd32(m, a1 + 8u));
        return;
    }
    a1 += 2u;
    uint16_t back = rd16(m, a1);
    if (!back) return;
    a1 -= back;
    sub_0D0678(m, obj, a1);
}

int Obj_E04CC_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* jmp 0x000E063E */
            sub_0D0E30(m, 0x0000012Cu);
            wr8(m, obj + 0x06u, 0x00u);
            sub_0CEF0E(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x0000012Cu);
        /* falls through to the explosion path */
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        uint32_t d0 = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);

        if (rd8(m, obj + 0x22u) == 0u) {
            uint32_t s = rd32(m, F3_A5 + 0x4224u);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - s);
            if ((int16_t)rd16(m, obj + 0x3Au) > 0x140) {
                goto draw;
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x20u));
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
            wr8(m, obj + 0x22u, 0x01u);
        }

        {
            uint8_t v = rd8(m, obj + 0x07u);
            wr8(m, obj + 0x07u, (uint8_t)(v & (uint8_t)~0x02u));
            if (v & 0x02u) {
                if (ObjScript_CF1E4(m, obj) != 0)
                    return -1;
            }
        }

        if (rd8(m, obj + 0x20u) != 0u) {
            /* lea (-0x1a62,PC),A1 at 0xE0554: PC = extension word 0xE0556 -> 0xDEAF4 */
            if (ObjScript_CF4B8(m, obj, 0x000DEAF4u) != 0)
                return -1;
        }

draw:
        Obj_FlagOffscreen(m, obj);
        if (rd8(m, obj + 0x07u) & 0x08u) {
            goto explode;
        }

        if (rd16(m, obj + 0x58u) != 0u) {
            if (Terrain_TestPoint(m, (int16_t)rd16(m, obj + 0x3Au),
                                  (int16_t)rd16(m, obj + 0x3Eu))) {
                wr16(m, obj + 0x58u, 0u);
            }
        } else {
            if ((int16_t)rd16(m, obj + 0x3Au) > 0x127) {
                wr16(m, obj + 0x58u, 0x00B4u);
            } else if (Terrain_TestPoint(m, (int16_t)(uint16_t)(rd16(m, obj + 0x3Au) + 0x1Fu),
                                         (int16_t)rd16(m, obj + 0x3Eu))) {
                wr16(m, obj + 0x58u, 0x00B4u);
            }
        }

        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

        /* jsr ([0x86,PC],D0w*4) at 0xE05CC: table 0xE0654 indexed by obj+6.b;
         * entry 0 is null, 1..5 below. Then 0xE05D4: D0DC2, CBE34. */
        switch (rd8(m, obj + 0x06u)) {
        case 1: /* 0xE066C */
            sub_0D0690(m, obj);
            break;
        case 2: /* 0xE0674 */
            sub_0D0678(m, obj, 0x000DEC7Au);
            wr8(m, obj + 0x06u, 3u);
            break;
        case 3: /* 0xE0686 */
            sub_0D0690(m, obj);
            if (rd16(m, obj + 0x4Cu) == 0u) {
                sub_0D0678(m, obj, 0x000DEC56u);
                wr8(m, obj + 0x06u, 1u);
            }
            break;
        case 4: /* 0xE06A6 */
            wr32(m, obj + 0x44u, 0x000E06F0u);            /* 0xCF1D8 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
            sub_0D0678(m, obj, 0x000DEC56u);
            wr8(m, obj + 0x06u, 5u);
            break;
        case 5: /* 0xE06C2 */
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
            break;
        default:
            return -1;                                     /* null table entry */
        }
        sub_0D0DC2(m, obj);
        sub_0CBE34(m, obj);
        return 0;
    }

    /* 0x000E05EE */
    {
        uint32_t r = Obj_Spawn(m, obj, 0x000DC328u, 3u, 2u, 0u);
        if ((int32_t)r < 0) {
            goto finish;
        }
        uint32_t a1 = r;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0008u));
        wr16(m, a1 + 0x58u, 0x00B4u);
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
    }

explode:
finish:
    wr8(m, obj + 0x06u, 0x00u);
    Obj_StartDying(m, obj);
    return 0;
}
