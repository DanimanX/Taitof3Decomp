#include "obj_DA138.h"
#include "../objects/obj_common.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0000649A: D0 = normalised distance factor of (D0w, D1w), integer
 * table at ROM 0x28340 indexed by (D.31:16>>?); D1w = 4..16 mantissa scale. */
static uint32_t sub_0000649A(const f3mem_t *m, uint16_t d0w_in, uint16_t d1w_in)
{
    uint32_t d0 = ((uint32_t)(uint16_t)((int32_t)(int16_t)d0w_in * (int32_t)(int16_t)d0w_in)) << 16;
    d0 += ((uint32_t)(uint16_t)((int32_t)(int16_t)d1w_in * (int32_t)(int16_t)d1w_in)) << 16;
    d0 = (uint32_t)(d0 + d0);
    uint16_t d1 = 4;
    if ((int32_t)d0 < 0x200) goto lab;
    d0 >>= 8; d1 = (uint16_t)(d1 + 4);
    if ((int32_t)d0 < 0x200) goto lab;
    d0 >>= 8; d1 = (uint16_t)(d1 + 4);
    if ((int32_t)d0 < 0x200) goto lab;
    d0 >>= 8; d1 = (uint16_t)(d1 + 4);
lab:
    {
        uint16_t idx = (uint16_t)(d0 >> 16);
        uint32_t d0r = rd16(m, 0x00028340u + (uint32_t)idx * 2u);
        d0r = (uint32_t)(d0r << d1);
        d0r = (d0r << 16) | (d0r >> 16);
        return d0r;
    }
}

/* ROM 0x00006422: D0 = angle of (D0w, D1w) in 256/4 units via the
 * arctan table at ROM 0x26E40 (low byte, biased by quadrant) */
static uint32_t sub_00006422(const f3mem_t *m, uint16_t d0w_in, uint16_t d1w_in)
{
    uint16_t d0w = d0w_in, d1w = d1w_in;
    uint32_t d2 = 0;
    if ((int16_t)d0w < 0) { d2 |= 1u; d0w = (uint16_t)(-(int16_t)d0w); }
    if ((int16_t)d1w < 0) { d2 |= 2u; d1w = (uint16_t)(-(int16_t)d1w); }

    uint16_t d3;
    if ((int16_t)d0w <= (int16_t)d1w) {
        /* bfffo D0{0x10:0x10},D3: leading zeros of the 16-bit value */
        int i = 15;
        while (i >= 0 && !(d0w & (1u << i))) i--;
        d3 = (uint16_t)(31 - i);
    } else {
        int i = 15;
        while (i >= 0 && !(d1w & (1u << i))) i--;
        d3 = (uint16_t)(31 - i);
    }
    d3 = (uint16_t)(d3 - 0x1A);
    if ((int16_t)d3 < 0) {
        uint16_t s = (uint16_t)(-(int16_t)d3);
        d0w = (uint16_t)(d0w >> s);
        d1w = (uint16_t)(d1w >> s);
    }
    if (d2 != 0u && d2 != 6u) {
        uint16_t t = d0w; d0w = d1w; d1w = t;
    }
    d1w = (uint16_t)(d1w << 6);
    uint32_t a0 = 0x00026E40u + d1w + d0w;
    uint16_t r = rd8(m, a0);
    /* lea (0x12,PC),A0 at 0x647E -> 0x6492; add.w (0,A0,D2w*1) */
    r = (uint16_t)(r + rd16(m, 0x00006492u + d2));
    return r & 0xFFu;
}

/* ROM 0x0CBB2C: move mode 15 -> 22, angle steering + fly-past retarget */
static int sub_0CBB2C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x32u, 0x00u);                 /* 0x0CBB2C: sf (0x32,A0) */
    /* 0x0CBB50 */
    wr8(m, obj + 0x34u, 0xFFu);                 /* st (0x34,A0) */

    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0w = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x100u);
    int16_t m16 = (int16_t)((int32_t)(int16_t)d0w * (int32_t)(int16_t)rd16(m, a1));
    a1 += 2u;
    d0w = (uint16_t)(m16 >> 5);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));

    d0w = (uint16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x100u);
    m16 = (int16_t)((int32_t)(int16_t)d0w * (int32_t)(int16_t)rd16(m, a1));
    d0w = (uint16_t)(m16 >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));

    d0w = (uint16_t)((uint16_t)rd16(m, obj + 0x14u) - 0xA0u);
    uint16_t d1w = (uint16_t)((uint16_t)rd16(m, obj + 0x18u) - 0x74u);
    d0w = (uint16_t)sub_0000649A(m, d0w, d1w);
    d0w = (uint16_t)(d0w << 2);
    d0w = (uint16_t)(d0w - rd16(m, F3_A5 + 0x1D5Cu));
    if ((int16_t)d0w < 0)
        goto lab_cbc80;
    d0w = (uint16_t)(d0w << 6);
    wr16(m, obj + 0x2Cu, d0w);

    d0w = (uint16_t)((uint16_t)rd16(m, obj + 0x14u) - 0xA0u);
    d1w = (uint16_t)((uint16_t)rd16(m, obj + 0x18u) - 0x74u);
    d0w = (uint16_t)sub_00006422(m, d0w, d1w);
    d0w = (uint16_t)(d0w << 8);
    wr16(m, obj + 0x2Eu, d0w);
    wr16(m, obj + 0x30u, 0x0100u);

    /* 0x0CBBC4 */
    if (!(rd8(m, F3_A5 + 0x1D58u) & 0x02u))
        goto lab_cbc80;
    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0200u));

    d0w = (uint16_t)rd8(m, obj + 0x32u);
    if (d0w != 0u) {
        a1 = rd32(m, obj + 0x0Cu);
        d1w = (uint16_t)(rd16(m, obj + 0x2Eu) + 0x4800u);
        d1w = (uint16_t)(d1w >> (d0w & 0x3Fu));
        wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)d1w * 4u));
    }

    /* 0x0CBBF2 */
    d0w = (uint16_t)(rd16(m, obj + 0x2Cu) >> 5);
    wr16(m, obj + 0x2Cu, (uint16_t)(rd16(m, obj + 0x2Cu) - d0w));

    uint32_t d1 = (uint32_t)(0x100u * (uint32_t)rd16(m, obj + 0x2Cu));
    d1 = (uint32_t)((int32_t)d1 >> 8);
    d1 = (uint32_t)((int32_t)d1 >> 6);
    d1w = (uint16_t)((uint16_t)d1 + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t d2w = d1w;

    d0w = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);
    uint32_t a1t = 0x00027E40u;
    d1 = (uint32_t)((int32_t)(int16_t)d1w * (int32_t)(int16_t)rd16(m, a1t + 0x80u + (uint32_t)d0w * 2u));
    d1 = (uint32_t)(d1 + 0x00A00000u);
    wr32(m, obj + 0x14u, d1);
    uint32_t d2 = (uint32_t)((int32_t)(int16_t)d2w * (int32_t)(int16_t)rd16(m, a1t + (uint32_t)d0w * 2u));
    d2 = (uint32_t)(d2 + 0x00740000u);
    wr32(m, obj + 0x18u, d2);

    uint32_t d0 = (uint32_t)((int32_t)(int16_t)0xFF00 * (int32_t)(int16_t)rd16(m, obj + 0x30u));
    d0 = (uint32_t)((int32_t)d0 >> 8);
    d0w = (uint16_t)(d0 + 0x100u);
    wr8(m, obj + 0x1Fu, (uint8_t)d0w);
    wr8(m, obj + 0x1Eu, (uint8_t)d0w);

    a1 = rd32(m, obj + 0x10u);
    d0w = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x100u);
    m16 = (int16_t)((int32_t)(int16_t)d0w * (int32_t)(int16_t)rd16(m, a1));
    d0w = (uint16_t)(m16 >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));

    Obj_DrawSprite(m, obj);
    wr32(m, obj, 0x000CBC78u);
    return 0;

lab_cbc80:
    /* 0x0CBC80 */
    wr32(m, obj, 0x000CBC88u);
    return 0;
}

/* ROM 0x0D0690: script advance (6-byte frames) */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - 1u));
    if (rd16(m, obj + 0x4Cu) != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u);
    a1 += (uint32_t)(int32_t)(int16_t)0x000Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0w = rd16(m, a1); a1 += 2u;
    if ((int16_t)d0w < 0) {
        /* 0x0D06C0 */
        if (rd16(m, a1) == 0u)
            return;
        a1 = (uint32_t)(a1 - (uint32_t)(int32_t)(int16_t)rd16(m, a1));
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
        d0w = rd16(m, a1); a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)d0w);
        wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }
    wr16(m, obj + 0x4Cu, d0w);
    d0w = rd16(m, a1); a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0DA138 */
int Obj_DA138_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);

    /* 0x0DA140 */
    uint8_t b = rd8(m, obj + 0x07u);
    uint8_t nb = (uint8_t)(b & ~0x02u);
    wr8(m, obj + 0x07u, nb);
    if (b & 0x02u) {
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;                      /* 0x0DA14A */
    }

    /* 0x0DA150 */
    if (rd8(m, obj + 0x20u) != 0u) {
        /* lea (-0xca8,PC),A1 at 0xDA158 -> 0x0DA4B2 */
        if (ObjScript_CF4B8(m, obj, 0x000DA4B2u) < 0)
            return -1;                      /* 0x0DA15C */
    }

    /* 0x0DA162 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    /* 0x0DA176: jsr 0x0C6632 (point-vs-screen test; D0b != 0 -> dying) */
    if (Terrain_TestPoint(m, (int16_t)rd16(m, obj + 0x14u), (int16_t)rd16(m, obj + 0x18u)) != 0) {
        Obj_StartDying(m, obj);             /* bne 0x000CEE74 */
        return 0;
    }

    /* 0x0DA184 */
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);             /* 0x0DA18E */
        return 0;
    }

    /* 0x0DA194 */
    if (rd16(m, obj + 0x68u) == 0u && !(rd8(m, F3_A5 - 0x144Bu) & 0x20u))
        Coll_AddHazard(m, obj);             /* 0x0DA1A6 */

    /* 0x0DA1AC */
    uint16_t d0w = 0x10;
    uint32_t d1 = rd8(m, obj + 0x1Eu);
    d1 = (uint16_t)(d1 >> 4);
    d0w = (uint16_t)(d0w - (uint16_t)d1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));

    Obj_DrawSprite(m, obj);                 /* 0x0DA1C2 */

    /* 0x0DA1C8 */
    d0w = (uint16_t)(0x08u - (uint16_t)(d1 >> 1));
    wr16(m, obj + 0x2Eu, d0w);
    wr16(m, obj + 0x32u, d0w);
    d0w = (uint16_t)(-d0w);
    wr16(m, obj + 0x2Cu, d0w);
    wr16(m, obj + 0x30u, d0w);

    /* 0x0DA1E2 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) &&
        (int16_t)rd16(m, obj + 0x14u) >= -0x0A &&
        (int16_t)rd16(m, obj + 0x14u) <= 0x11C)
        return sub_0CBB2C(m, obj);          /* jmp 0x000CBB2C */

    /* 0x0DA200 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    return 0;
}
