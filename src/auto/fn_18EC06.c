/* ROM 0x18EC06: 3D horizon/perspective line-scroll driver (A0 = object).
 * Reads tilt +0x44 (byte 0..255). For <= 0x7F uses table 0x175C9C[tilt*2] and
 * draws perspective line-zoom/Y-offset (0x1767C8) into 0x62A400/0x628400,
 * X-shear (0x1764F2) into 0x624400, and priority bits (0x1766EE/0x1766DA) in
 * 0x62B400; for > 0x7F mirrors tilt (0xFF - tilt) and uses 0x176490 and
 * swapped 0x1766DA/0x1766EE. Then steps phase +0x48 -= 0x100, updates +0x44
 * and scroll A5+0x4218 / A5-0x5D94 from cos(0x027E40 + 0x80), and re-arms
 * 0x18EC06 while +0x48 > 0x100, else sets +0x22 = -1, +0x06 = 4 and advances
 * to 0x18ED80. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x1767C8: perspective line-scale (0x62A400) and line-offset (0x628400) pass. */
static void sub_1767C8(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d0_in, uint32_t d1_in, uint16_t d3_in,
                       uint16_t d6_in, uint16_t d7_in, f3regs_t *r)
{
    wr32(m, F3_A5 + 0x4298u, d0_in);
    uint32_t d5 = d1_in;
    wr16(m, F3_A5 + 0x42A0u, d3_in);
    int16_t d4w = 2;
    int16_t d6w = (int16_t)d6_in;
    int16_t d7w = (int16_t)d7_in;
    if (d7w > d6w) {
        d4w = -2;
        d6w--;
    }
    d6w = (int16_t)(d6w + d6w);
    d7w = (int16_t)(d7w + d7w);

    uint32_t d0 = d0_in;
    uint32_t d1 = d1_in;
    for (;;) {
        int16_t d2w = (int16_t)rd16(m, F3_A5 + 0x4298u);
        d0 = 0xFFFF6000u;                                /* -0xA000 */
        d1 = 0x000000A0u;
        int16_t div1 = (int16_t)((int16_t)d1 + d2w);
        if (div1 != 0) {
            int32_t q = (int32_t)d0 / (int32_t)div1;
            int32_t rem = (int32_t)d0 % (int32_t)div1;
            if (q >= -32768 && q <= 32767)
                d0 = ((uint32_t)(uint16_t)rem << 16) | (uint16_t)q;
        }
        d0 = setw(d0, (uint16_t)((uint16_t)d0 + 0x0100u));
        int16_t h = (int16_t)(0x0024 - (int16_t)rd16(m, F3_A5 + 0x42A0u));
        int32_t d2 = ((int32_t)(int16_t)d0 * (int32_t)h) << 6;
        {
            int32_t q = d2 / 0x32;
            int32_t rem = d2 % 0x32;
            if (q >= -32768 && q <= 32767)
                d2 = (int32_t)(((uint32_t)(uint16_t)rem << 16) | (uint16_t)q);
        }
        uint16_t outw = (uint16_t)((uint16_t)d2 ^ 0x003Fu);
        wr16(m, a0 + (uint32_t)(int32_t)d7w, outw);
        wr8(m, a1 + (uint32_t)(int32_t)d7w, (uint8_t)d0);
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d5);
        d7w = (int16_t)(d7w + d4w);
        uint8_t cond = (d7w <= d6w) ? 0xFFu : 0x00u;
        cond ^= (uint8_t)d4w;
        if ((cond & 0x0Cu) == 0u)
            break;
    }
    r->d[0] = d0;
    r->d[1] = d1;
}

/* ROM 0x176490 (dir = -1) / 0x1764F2 (dir = +1): X-shear line-buffer fill. */
static void sub_shear(const f3mem_t *m, uint32_t a0, uint16_t d2_in,
                      uint16_t d5_end, uint16_t d7_in, int dir, f3regs_t *r)
{
    wr32(m, F3_A5 + 0x4298u, 0u);
    int16_t d4w = 0;
    int16_t d6w = (int16_t)d5_end;
    int16_t d7w = (int16_t)d7_in;
    int16_t idx = (int16_t)(d2_in & 0xFFFEu);
    uint16_t raw_sin = rd16(m, 0x00027E40u + (uint32_t)(int32_t)idx);
    uint32_t d3 = (r->d[3] & 0xFFFF0000u) | raw_sin;
    d3 = (uint32_t)((int32_t)d3 >> 8);                   /* 1764AC/17650E asr.l #8,D3 */
    int16_t d3w = (int16_t)(uint16_t)d3;
    int16_t d2w = (int16_t)rd16(m, 0x00027EC0u + (uint32_t)(int32_t)idx);
    int16_t step = 1;
    uint16_t flag = rd16(m, F3_A5 - 0x5D58u);
    if ((dir < 0 && flag == 0u) || (dir > 0 && flag != 0u)) {
        d2w = (int16_t)(-d2w);
        step = -1;
    }

    uint32_t d0 = r->d[0];
    uint32_t d1;
    for (;;) {
        d1 = 0u;
        d0 = setw(d0, (uint16_t)d4w);
        if (d4w != 0) {
            int32_t num = (int32_t)rd32(m, F3_A5 + 0x4298u) >> 2;
            d0 = (uint32_t)((int32_t)d3w * (int32_t)d4w);
            int16_t div = (int16_t)((int16_t)(uint16_t)d0 + d2w);
            d0 = setw(d0, (uint16_t)div);
            d1 = (uint32_t)num;
            if (div != 0) {
                int32_t q = num / (int32_t)div;
                int32_t rem = num % (int32_t)div;
                if (q >= -32768 && q <= 32767)
                    d1 = ((uint32_t)(uint16_t)rem << 16) | (uint16_t)q;
            }
            d1 = setw(d1, (uint16_t)((int16_t)(uint16_t)d1 - d4w));
        }
        uint16_t cur = rd16(m, a0 + (uint32_t)(int32_t)d7w * 2u);
        d0 = setw(d0, cur);
        d0 = (d0 & ~0x01FFu) | (d1 & 0x01FFu);           /* bfins D1,D0{23:9} */
        wr16(m, a0 + (uint32_t)(int32_t)d7w * 2u, (uint16_t)d0);
        wr16(m, F3_A5 + 0x4298u, (uint16_t)(rd16(m, F3_A5 + 0x4298u) + 1u));
        d4w = (int16_t)(d4w + step);
        d7w = (int16_t)(d7w + dir);
        if ((dir > 0 && d7w > d6w) || (dir < 0 && d7w < d6w))
            break;
    }
    r->d[0] = d0;
    r->d[1] = d1;
}

int Fn_18EC06(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t tilt = rd16(m, obj + 0x44u) & 0x00FFu;     /* 0x18EC0A..0x18EC0E */
    int16_t line_off = (int16_t)rd16(m, F3_A5 - 0x5D56u);

    uint32_t yscroll = rd32(m, F3_A5 + 0x4218u);
    uint16_t neg_y = (uint16_t)(-(int16_t)(uint16_t)(yscroll >> 16));
    uint16_t d7w = neg_y & 0x00FFu;

    if ((int16_t)tilt <= 0x007F) {                       /* 0x18EC12..0x18EC16 */
        uint16_t idx = (uint16_t)((tilt * 2u) & 0x00FFu);
        uint32_t step = rd32(m, 0x00175C9Cu + (uint32_t)idx * 4u);
        sub_1767C8(m, 0x0062A400u + (uint32_t)(int32_t)line_off,
                   0x00628400u + (uint32_t)(int32_t)line_off,
                   0u, step, 2u, 0x00E7u, d7w, r);
        r->d[3] = 2u;
        sub_shear(m, 0x00624400u + (uint32_t)(int32_t)line_off,
                  (uint16_t)((int16_t)idx >> 1), 0x00E7u, d7w, +1, r);
        r->a[0] = 0x0062B400u + (uint32_t)(int32_t)line_off;
        r->d[6] = setw(r->d[6], neg_y);
        r->d[7] = setw(r->d[7], 0x00E7u);
        Leaf_1766EE(m, r);
        r->d[7] = setw(r->d[7], neg_y);
        r->d[6] = 0u;
        Leaf_1766DA(m, r);
    } else {
        uint16_t idx = (uint16_t)(((0x00FF - (int16_t)tilt) * 2) & 0x00FF);
        uint32_t step = rd32(m, 0x00175C9Cu + (uint32_t)idx * 4u);
        sub_1767C8(m, 0x0062A400u + (uint32_t)(int32_t)line_off,
                   0x00628400u + (uint32_t)(int32_t)line_off,
                   0u, step, 2u, 0u, d7w, r);
        r->d[3] = 2u;
        sub_shear(m, 0x00624400u + (uint32_t)(int32_t)line_off,
                  (uint16_t)((int16_t)idx >> 1), 0u, d7w, -1, r);
        r->a[0] = 0x0062B400u + (uint32_t)(int32_t)line_off;
        r->d[6] = setw(r->d[6], neg_y);
        r->d[7] = setw(r->d[7], 0x00E7u);
        Leaf_1766DA(m, r);
        r->d[7] = setw(r->d[7], neg_y);
        r->d[6] = 0u;
        Leaf_1766EE(m, r);
    }

    /* 0x18ED28..0x18ED7E */
    r->a[0] = obj;
    uint16_t phase = (uint16_t)(rd16(m, obj + 0x48u) - 0x0100u);
    wr16(m, obj + 0x48u, phase);
    uint16_t ang = phase >> 8;
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
    int16_t s = (int16_t)((int16_t)(c + 0x4000) >> 7);
    wr16(m, obj + 0x44u, (uint16_t)s);
    int16_t scr = (int16_t)(-(s >> 1) + (int16_t)rd16(m, obj + 0x18u));
    wr16(m, F3_A5 + 0x4218u, (uint16_t)scr);
    wr16(m, F3_A5 - 0x5D94u, (uint16_t)scr);

    if ((int16_t)phase > 0x0100) {                       /* 0x18ED60 */
        wr32(m, obj, 0x0018EC06u);
    } else {
        wr16(m, obj + 0x22u, 0xFFFFu);
        wr16(m, obj + 0x06u, 0x0004u);                   /* 0x18ED70 (0x18ED76 beq never taken) */
        wr32(m, obj, 0x0018ED80u);
    }
    r->d[0] = setw(r->d[0], (uint16_t)scr);
    r->a[1] = 0x00027E40u;
    return 0;
}
