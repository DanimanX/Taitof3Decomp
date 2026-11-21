#include "obj_12EB90.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"
#include "obj_13E4EE.h"

/* sin/cos table: sin at 0x27E40 + i*2, cos at 0x27EC0 + i*2 */
#define SINTBL 0x027E40u

static int16_t sin_at(const f3mem_t *m, uint16_t idx)
{ return (int16_t)rd16(m, SINTBL + (uint32_t)((int32_t)(int16_t)idx * 2)); }
static int16_t cos_at(const f3mem_t *m, uint16_t idx)
{ return (int16_t)rd16(m, SINTBL + 0x80u + (uint32_t)((int32_t)(int16_t)idx * 2)); }

/* divs.w src,Dn: Dn = rem<<16 | quot; unchanged on overflow */
static uint32_t divs_w(uint32_t dn, uint16_t src)
{
    int32_t a = (int32_t)dn, b = (int16_t)src;
    if (b == 0)
        return dn;
    int64_t q = (int64_t)a / b, r = (int64_t)a % b;
    if (q < -32768 || q > 32767)
        return dn;
    return ((uint32_t)(uint16_t)(int16_t)r << 16) | (uint16_t)(int16_t)q;
}

/* asr.w with a register count (mod 64) */
static uint16_t asr_w(uint16_t v, unsigned cnt)
{
    cnt &= 63u;
    if (cnt >= 16u)
        return (int16_t)v < 0 ? 0xFFFFu : 0u;
    return (uint16_t)((int16_t)v >> cnt);
}

/* bit field {n:1} of the flag long at A5+0x30EE (offset from MSB) */
static uint32_t bf_addr(unsigned n) { return F3_A5 + 0x30EEu + n / 8u; }
static uint8_t bf_mask(unsigned n) { return (uint8_t)(0x80u >> (n % 8u)); }
static int bftst(const f3mem_t *m, unsigned n)
{ return (rd8(m, bf_addr(n)) & bf_mask(n)) != 0u; }
static int bfclr(const f3mem_t *m, unsigned n)
{
    uint8_t b = rd8(m, bf_addr(n));
    wr8(m, bf_addr(n), (uint8_t)(b & (uint8_t)~bf_mask(n)));
    return (b & bf_mask(n)) != 0u;
}
static void bfset(const f3mem_t *m, unsigned n)
{ wr8(m, bf_addr(n), (uint8_t)(rd8(m, bf_addr(n)) | bf_mask(n))); }

static void bset8(const f3mem_t *m, uint32_t a, unsigned bit)
{ wr8(m, a, (uint8_t)(rd8(m, a) | (1u << bit))); }

/* ROM 0x13F238 (remap) + Sound_PlayCmd */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

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

/* ROM 0x0CBDA4 */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);

    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0CBDDE */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x11AE62: shared boss hit points (A5+0x30DA) */
static void sub_11AE62(const f3mem_t *m, uint32_t obj, uint32_t d0, unsigned d1)
{
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)
        return;
    int32_t d2 = (int32_t)rd32(m, obj + 0x24u);
    if (d2 == 0)
        return;
    wr32(m, obj + 0x24u, 0u);
    uint32_t a1 = rd32(m, obj + 0x36u);
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u) {
        d2 >>= (d1 & 63u);
        if (k != 1u && k != 5u)
            d2 >>= 1;
    }
    wr32(m, F3_A5 + 0x30DAu, rd32(m, F3_A5 + 0x30DAu) + (uint32_t)d2);
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0)
        return;
    a1 = sub_0CBDDE(m, obj);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

static uint16_t max16(uint16_t a, uint16_t b) { return (int16_t)a < (int16_t)b ? b : a; }
static uint16_t min16(uint16_t a, uint16_t b) { return (int16_t)a > (int16_t)b ? b : a; }

/* ROM 0x11C096: hit spark */
static void sub_11C096(const f3mem_t *m, uint32_t obj)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 0u)
        goto done;
    {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | hit));
    }
    if (rd8(m, obj + 0x34u) & 0x02u)
        goto done;

    {
        uint16_t now = rd16(m, F3_A5 + 0x0878u);
        uint16_t d0w = (uint16_t)(now - rd16(m, F3_A5 + 0x311Cu));
        wr16(m, F3_A5 + 0x311Cu, now);
        if ((int16_t)d0w < 2) {
            uint32_t prev = rd32(m, F3_A5 + 0x3116u);
            wr32(m, F3_A5 + 0x3116u, obj);
            if (prev == obj) {
                uint16_t c = (uint16_t)(rd16(m, F3_A5 + 0x311Au) + 1u);
                wr16(m, F3_A5 + 0x311Au, c);
                if ((int16_t)c <= 3)
                    goto done;
            }
        }
    }

    wr16(m, F3_A5 + 0x311Au, 0u);
    uint32_t a2 = 0x0040B0F6u;
    int found = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t b = rd8(m, a2);
        wr8(m, a2, (uint8_t)(b | 0x80u));
        if (!(b & 0x80u)) {
            found = 1;
            break;
        }
        a2 += 4u;
    }
    if (!found)
        goto done;

    {
        uint32_t a1 = rd32(m, obj + 0x36u);
        uint16_t d0w, d1w;
        if (rd8(m, a1 + 0x22u) == 6u || rd8(m, obj + 0x35u) == 2u) {
            /* 0x11C18A: random point inside the hit box */
            uint32_t d1 = (uint16_t)Rng_NextMem(m);
            uint16_t dv = (uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u));
            if (dv != 0u)
                d1 = ((d1 % dv) << 16) | (d1 / dv);
            d1w = (uint16_t)(d1 >> 16);
            d1w = (uint16_t)(d1w + rd16(m, obj + 0x30u) + rd16(m, obj + 0x18u));
            uint32_t d0 = Rng_NextMem(m) & 0xFFFFu;
            dv = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
            if (dv != 0u)
                d0 = ((d0 % dv) << 16) | (d0 / dv);
            d0w = (uint16_t)(d0 >> 16);
            d0w = (uint16_t)(d0w + rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x14u));
        } else {
            /* centre of the overlap of both hit boxes */
            uint16_t ox = rd16(m, obj + 0x14u), ax = rd16(m, a1 + 0x14u);
            uint16_t oy = rd16(m, obj + 0x18u), ay = rd16(m, a1 + 0x18u);
            d0w = max16((uint16_t)(rd16(m, obj + 0x2Cu) + ox), (uint16_t)(rd16(m, a1 + 0x2Cu) + ax));
            d1w = min16((uint16_t)(rd16(m, obj + 0x2Eu) + ox), (uint16_t)(rd16(m, a1 + 0x2Eu) + ax));
            d0w = (uint16_t)((int16_t)(uint16_t)(d0w + d1w) >> 1);
            d1w = max16((uint16_t)(rd16(m, obj + 0x30u) + oy), (uint16_t)(rd16(m, a1 + 0x30u) + ay));
            uint16_t d2w = min16((uint16_t)(rd16(m, obj + 0x32u) + oy), (uint16_t)(rd16(m, a1 + 0x32u) + ay));
            d1w = (uint16_t)((int16_t)(uint16_t)(d1w + d2w) >> 1);
        }
        /* 0x11C1C8 */
        d0w = (uint16_t)(d0w - rd16(m, F3_A5 + 0x30D2u));
        d1w = (uint16_t)(d1w - rd16(m, F3_A5 + 0x30D6u));
        uint32_t v = rd32(m, a2);
        v = (v & ~(0x3FFu << 21)) | (((uint32_t)d0w & 0x3FFu) << 21); /* bfins D0,(A2){1:10} */
        v = (v & ~(0x1FFu << 12)) | (((uint32_t)d1w & 0x1FFu) << 12); /* bfins D1,(A2){11:9} */
        wr32(m, a2, v);
    }

done:
    wr8(m, obj + 0x35u, 0u);
}

/* ROM 0x12EC20: keep the depth-sorted list (+0x62 next, +0x66 prev) ordered */
static void sub_12EC20(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = obj, a2, a3, d2;
    int16_t d0 = (int16_t)rd16(m, obj + 0x5Eu);
    int16_t d1 = (int16_t)rd16(m, obj + 0x22u);

    for (;;) {
        d2 = rd32(m, a1 + 0x62u);
        if (d2 == 0u)
            break;
        a1 = d2;
        int16_t z = (int16_t)rd16(m, a1 + 0x5Eu);
        if (d0 > z)
            continue;
        if (d0 < z)
            break;
        if (d1 >= (int16_t)rd16(m, a1 + 0x22u))
            continue;
        break;
    }
    a2 = rd32(m, obj + 0x62u);
    a3 = rd32(m, obj + 0x66u);
    wr32(m, a2 + 0x66u, a3);
    wr32(m, a3 + 0x62u, a2);
    a2 = rd32(m, a1 + 0x66u);
    wr32(m, a1 + 0x66u, obj);
    wr32(m, a2 + 0x62u, obj);
    wr32(m, obj + 0x66u, a2);
    wr32(m, obj + 0x62u, a1);

    a1 = obj;
    d0 = (int16_t)rd16(m, obj + 0x5Eu);
    for (;;) {
        d2 = rd32(m, a1 + 0x66u);
        if (d2 == 0u)
            break;
        a1 = d2;
        int16_t z = (int16_t)rd16(m, a1 + 0x5Eu);
        if (d0 < z)
            continue;
        if (d0 > z)
            break;
        if (d1 <= (int16_t)rd16(m, a1 + 0x22u))
            continue;
        break;
    }
    a2 = rd32(m, obj + 0x62u);
    a3 = rd32(m, obj + 0x66u);
    wr32(m, a2 + 0x66u, a3);
    wr32(m, a3 + 0x62u, a2);
    a2 = rd32(m, a1 + 0x62u);
    wr32(m, a2 + 0x66u, obj);
    wr32(m, a1 + 0x62u, obj);
    wr32(m, obj + 0x66u, a1);
    wr32(m, obj + 0x62u, a2);
}

/* ROM 0x12ECDC: perspective projection of (+0x56, +0x5A, depth +0x5E) */
static void sub_12ECDC(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = (uint32_t)((int32_t)rd32(m, obj + 0x56u) >> 8);
    uint32_t d1 = (uint32_t)((int32_t)rd32(m, obj + 0x5Au) >> 8);
    uint16_t d2w = rd16(m, obj + 0x5Eu);
    if ((int16_t)d2w < 0)
        return;
    d2w = (uint16_t)((d2w >> 1) + 0x0064u);
    d0 = divs_w(d0, d2w);
    d1 = divs_w(d1, d2w);
    uint16_t d3w = (uint16_t)d0;
    wr16(m, obj + 0x14u, (uint16_t)((uint16_t)d0 + 0x00A0u));
    wr16(m, obj + 0x18u, (uint16_t)((uint16_t)d1 + 0x0074u));
    d0 = divs_w(0x0000639Cu, d2w);
    uint16_t d0w = (uint16_t)(0u - (uint16_t)((uint16_t)d0 - 0x00FFu));
    wr8(m, obj + 0x1Fu, (uint8_t)d0w);
    wr8(m, obj + 0x1Eu, (uint8_t)d0w);
    /* cmp2.w (0x76,PC),D2 at 0x12ED2A: bounds at 0x12EDA4 = [0, 0x68] */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && d2w <= rd16(m, 0x0012EDA6u)
        && d2w >= rd16(m, 0x0012EDA4u))
        sub_0CBDA4(m, obj);

    uint16_t d1w = (uint16_t)(0x0100u - d0w);
    uint32_t a1 = rd32(m, obj + 0x3Au);
    d3w = (uint16_t)(d3w << 3);
    d3w = (uint16_t)(d3w + rd16(m, obj + 0x20u));
    d3w = (uint16_t)(d3w >> 10);
    /* lea (0x36ee,PC),A2 at 0x12ED46 -> 0x132436 */
    d0w = (uint16_t)((d0w & 0xFF00u) | rd8(m, 0x00132436u + (uint32_t)(int32_t)(int16_t)d3w));
    d0 = (uint32_t)((int32_t)(int16_t)d0w * 12);
    a1 += (uint32_t)(int32_t)(int16_t)(uint16_t)d0;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr8(m, obj + 0x1Cu, rd8(m, a1));
    a1 += 2u;
    int16_t b = (int8_t)rd8(m, a1++);
    d0w = (uint16_t)(int16_t)((int32_t)b * (int16_t)d1w);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)((int16_t)d0w >> 8)));
    b = (int8_t)rd8(m, a1++);
    d0w = (uint16_t)(int16_t)((int32_t)b * (int16_t)d1w);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)((int16_t)d0w >> 8)));
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
}

/* ROM 0x12FE88 */
static void sub_12FE88(const f3mem_t *m, uint32_t obj)
{
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int16_t r = (int16_t)rd16(m, obj + 0x42u);
    wr32(m, obj + 0x56u, rd32(m, obj + 0x56u) + (uint32_t)((int32_t)r * cos_at(m, d1)));
    wr32(m, obj + 0x5Eu, rd32(m, obj + 0x5Eu) + (uint32_t)((int32_t)r * sin_at(m, d1)));
}

/* ROM 0x12FEB0: follow the parent segment (+0x44) */
static void sub_12FEB0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2 = rd32(m, obj + 0x44u);
    uint16_t d1 = rd16(m, obj + 0x20u);
    uint16_t d0 = (uint16_t)(rd16(m, a2 + 0x20u) + rd16(m, F3_A5 + 0x32A4u) - d1);
    d0 = asr_w(d0, rd16(m, F3_A5 + 0x32A8u));
    d1 = (uint16_t)(d1 + d0);
    wr16(m, obj + 0x20u, d1);
    d1 = (uint16_t)(d1 >> 8);
    wr32(m, obj + 0x56u, rd32(m, a2 + 0x56u));
    wr32(m, obj + 0x5Au, rd32(m, a2 + 0x5Au));
    wr32(m, obj + 0x5Eu, rd32(m, a2 + 0x5Eu));
    int16_t r = (int16_t)rd16(m, obj + 0x50u);
    wr32(m, obj + 0x56u, rd32(m, obj + 0x56u) - (uint32_t)((int32_t)r * cos_at(m, d1)));
    wr32(m, obj + 0x5Eu, rd32(m, obj + 0x5Eu) - (uint32_t)((int32_t)r * sin_at(m, d1)));
}

/* ROM 0x12FF04 */
static void sub_12FF04(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x1000u));
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x22u) << 3);
    d1 = (uint16_t)((d1 & 0xFF00u) | (uint8_t)(d1 + 0x30u));
    wr32(m, obj + 0x56u, rd32(m, obj + 0x56u) + (uint32_t)(0x14 * (int32_t)cos_at(m, d1)));
    wr32(m, obj + 0x5Eu, rd32(m, obj + 0x5Eu) + (uint32_t)(0x14 * (int32_t)sin_at(m, d1)));
}

/* ROM 0x12FF36 */
static void sub_12FF36(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);
    uint8_t d0 = rd8(m, a1 + 0x1Du);
    if (d0 == rd8(m, a1 + 0x4Fu))
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    else
        wr8(m, obj + 0x1Du, d0);
}

/* ROM 0x12FF56 */
static void sub_12FF56(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x329Du) != 0u)
        return;
    if (!bftst(m, 18))
        return;
    if (sub_0CC2BC(m, 4u) != 0u)
        return;
    /* movea.l (0x12ffd4,PC,D0w*4),A1 */
    uint32_t a1 = rd32(m, 0x0012FFD4u + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x22u) * 4));
    uint32_t d1;
    for (;;) {
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        if ((int16_t)d0 < 0)
            return;
        d1 = rd32(m, a1);
        a1 += 4u;
        if ((uint8_t)d0 == rd8(m, F3_A5 + 0x329Cu))
            break;
    }
    uint8_t c = (uint8_t)(rd8(m, F3_A5 + 0x329Cu) + 1u);
    wr8(m, F3_A5 + 0x329Cu, c);
    wr8(m, F3_A5 + 0x329Du, 0xFFu);
    if ((int8_t)c >= 0x0D)
        wr8(m, F3_A5 + 0x329Cu, 0u);
    Obj_Spawn(m, obj, 0x001309CEu, 5u, 2u, d1);
    play_sound(m, 0x003C);
}

/* common head of 0x12F8CA / 0x12FA80 / 0x12FC28 / 0x12FC5A */
static void seg_head(const f3mem_t *m, uint32_t obj)
{
    bset8(m, obj + 0x34u, 1);
    bset8(m, obj + 0x34u, 3);
    bset8(m, obj + 0x34u, 4);
    wr8(m, obj + 0x35u, 0u);
    sub_12FEB0(m, obj);
}

/* ROM 0x12F70A */
static void sub_12F70A(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {
        sub_12FF04(m, obj);
        return;
    }
    wr16(m, obj + 0x20u, rd16(m, F3_A5 + 0x32A2u));
    wr16(m, obj + 0x42u, rd16(m, F3_A5 + 0x32A6u));
    sub_12FE88(m, obj);
    bset8(m, obj + 0x34u, 1);
    bset8(m, obj + 0x34u, 3);
    bset8(m, obj + 0x34u, 4);
    wr8(m, obj + 0x35u, 0u);
    if (bftst(m, 0) && sub_0CC2BC(m, 6u) == 0u) {
        static const struct { uint8_t add; uint32_t hi; } sp[5] = {
            {0x80u, 0x0A2A0000u}, {0x70u, 0x023A0000u}, {0x60u, 0x044A0000u},
            {0x50u, 0x045A0000u}, {0x40u, 0x066A0000u},
        };
        for (int i = 0; i < 5; i++) {
            uint32_t d0 = Rng_NextMem(m) & 0x3Fu;
            d0 = (uint32_t)(uint8_t)(d0 + sp[i].add) << 8;
            Obj_Spawn(m, obj, 0x000CCE6Eu, 5u, 2u, d0 | sp[i].hi);
        }
    }
    if (bfclr(m, 1))
        Obj_Spawn(m, obj, 0x00130360u, 3u, 2u, 0x38560000u);
    if (bftst(m, 12) && sub_0CC2BC(m, 0x10u) == 0u) {
        Obj_Spawn(m, obj, 0x00130838u, 5u, 2u, 0u);
        play_sound(m, 0x0029);
    }
}

/* ROM 0x12F8CA */
static void sub_12F8CA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {
        sub_12FF04(m, obj);
        return;
    }
    seg_head(m, obj);
    if (bfclr(m, 15)) {
        uint32_t d1 = (rd8(m, obj + 0x1Cu) & 0x01u) ? 0x4920E000u : 0x2820A000u;
        Obj_Spawn(m, obj, 0x000CDDB0u, 5u, 2u, d1);
    }
    if (bftst(m, 13) && sub_0CC2BC(m, 6u) == 0u)
        Obj_Spawn(m, obj, 0x00130C76u, 5u, 2u, 0u);
    if (bfclr(m, 17)) {
        static const uint32_t args[5] = {
            0x00131AF8u, 0x00131B8Eu, 0x00131C24u, 0x00131CBAu, 0x00131D50u,
        };
        for (int i = 0; i < 5; i++)
            Obj_Spawn(m, obj, 0x001319D6u, 0u, 1u, args[i]);
    }
    if (bftst(m, 16) && sub_0CC2BC(m, 0x3Cu) == 0u)
        Obj_Spawn(m, obj, 0x000CDDB0u, 5u, 2u, 0x3A15D000u);
    if (bftst(m, 14) && sub_0CC2BC(m, 0x14u) == 0u)
        Obj_Spawn(m, obj, 0x00130C76u, 5u, 2u, 0u);
    sub_12FF56(m, obj);
}

/* ROM 0x12FA80 */
static void sub_12FA80(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {
        sub_12FF04(m, obj);
        return;
    }
    seg_head(m, obj);
    if (bfclr(m, 2) && (int16_t)rd16(m, F3_A5 + 0x087Eu) >= 0x10)
        Obj_Spawn(m, obj, 0x000CD504u, 5u, 2u, 0x24620000u);
    if (bfclr(m, 3))
        Obj_Spawn(m, obj, 0x001314C2u, 3u, 2u, 0x001316D2u);
    if (bfclr(m, 4))
        Obj_Spawn(m, obj, 0x001314C2u, 3u, 2u, 0x00131758u);
    if (bfclr(m, 5)) {
        bfset(m, 6);
        bfclr(m, 7);
        for (int i = 0; i < 4; i++)
            Obj_Spawn(m, obj, 0x001314C2u, 3u, 2u, 0x001317C4u);
    }
    if (bfclr(m, 8)) {
        for (int i = 0; i < 3; i++)
            Obj_Spawn(m, obj, 0x001314C2u, 3u, 2u, 0x0013180Au);
    }
    sub_12FF56(m, obj);
}

/* ROM 0x12FC28 */
static void sub_12FC28(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {
        sub_12FF04(m, obj);
        return;
    }
    seg_head(m, obj);
    sub_12FF56(m, obj);
}

/* ROM 0x12FC5A */
static void sub_12FC5A(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u) {
        sub_12FF04(m, obj);
        return;
    }
    seg_head(m, obj);
    if (bfclr(m, 11)) {
        static const uint32_t a[4] = { 0x1A18D000u, 0x1A443000u, 0x3D0DE800u, 0x34471800u };
        static const uint32_t b[4] = { 0x2518B000u, 0x25445000u, 0x020D9800u, 0x0B476800u };
        play_sound(m, 0x0029);
        const uint32_t *d = (rd8(m, obj + 0x1Cu) & 0x01u) ? b : a;
        for (int i = 0; i < 4; i++)
            Obj_Spawn(m, obj, 0x0013103Au, 9u, 2u, d[i]);
    }
}

/* ROM 0x12FD4C */
static void sub_12FD4C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30F2u) & 0x01u)
        return;
    wr8(m, obj + 0x4Fu, 0x40u);
    sub_12FF36(m, obj);
    bset8(m, obj + 0x34u, 1);
    bset8(m, obj + 0x34u, 0);
    bset8(m, obj + 0x34u, 3);
    bset8(m, obj + 0x34u, 4);
    bset8(m, obj + 0x34u, 5);
    wr8(m, obj + 0x35u, 0u);
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));
    wr32(m, obj + 0x56u, rd32(m, a1 + 0x56u));
    wr32(m, obj + 0x5Au, rd32(m, a1 + 0x5Au));
    wr32(m, obj + 0x5Au, rd32(m, obj + 0x5Au) + 0x0000C000u);
    wr32(m, obj + 0x5Eu, rd32(m, a1 + 0x5Eu));
    sub_12ECDC(m, obj);

    uint32_t d0 = rd32(m, obj + 0x14u);
    uint32_t d1 = rd32(m, obj + 0x18u);
    uint32_t d2 = rd8(m, obj + 0x1Eu);
    {
        uint32_t a2 = rd32(m, obj + 0x48u);
        uint16_t i = (uint16_t)(rd16(m, a2 + 0x20u) >> 8);
        int16_t r = (int16_t)(uint16_t)(rd16(m, F3_A5 + 0x32A0u) + 0x14u);
        wr32(m, obj + 0x56u, rd32(m, obj + 0x56u) - (uint32_t)((int32_t)r * cos_at(m, i)));
        wr32(m, obj + 0x5Eu, rd32(m, obj + 0x5Eu) - (uint32_t)((int32_t)r * sin_at(m, i)));
    }
    sub_12ECDC(m, obj);

    /* divu.w #0x16,D4; ext.l D4 */
    uint32_t d4 = (uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, F3_A5 + 0x32A0u) / 0x16u);
    if ((int32_t)d4 > 0) {
        wr32(m, obj + 0x10u, 0x000316CCu);
        d0 -= rd32(m, obj + 0x14u);
        d1 -= rd32(m, obj + 0x18u);
        uint16_t d3w = (uint16_t)((uint16_t)d2 << 8);
        uint16_t d5w = rd8(m, obj + 0x1Eu);
        d2 = (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)d2 - d5w);
        d2 <<= 8;
        d0 = (uint32_t)((int32_t)d0 / (int32_t)d4);
        d1 = (uint32_t)((int32_t)d1 / (int32_t)d4);
        d2 = divs_w(d2, (uint16_t)d4);
        uint16_t cnt = (uint16_t)d4;
        do {
            Obj_DrawSprite(m, obj);
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
            d3w = (uint16_t)(d3w + (uint16_t)d2);
            uint8_t d5 = (uint8_t)(d3w >> 8);   /* bfextu D3{16:8} */
            wr8(m, obj + 0x1Fu, d5);
            wr8(m, obj + 0x1Eu, d5);
            cnt = (uint16_t)(cnt - 1u);
        } while (cnt != 0xFFFFu);
    }

    /* 0x12FE54 */
    {
        uint32_t a2 = rd32(m, obj + 0x48u);
        uint16_t i = (uint16_t)(rd16(m, a2 + 0x20u) >> 8);
        int32_t r = -0x14;
        wr32(m, obj + 0x56u, rd32(m, obj + 0x56u) - (uint32_t)(r * cos_at(m, i)));
        wr32(m, obj + 0x5Eu, rd32(m, obj + 0x5Eu) - (uint32_t)(r * sin_at(m, i)));
    }
    wr32(m, obj + 0x5Au, rd32(m, obj + 0x5Au) - 0x0000C000u);
}

/* ROM 0x12EB90 */
int Obj_12EB90_Tick(const f3mem_t *m, uint32_t obj)
{
    /* movea.l (0x52,A0),A1; jsr (A1) */
    switch (rd32(m, obj + 0x52u)) {
    case 0x0012F70Au: sub_12F70A(m, obj); break;
    case 0x0012F8CAu: sub_12F8CA(m, obj); break;
    case 0x0012FA80u: sub_12FA80(m, obj); break;
    case 0x0012FC28u: sub_12FC28(m, obj); break;
    case 0x0012FC5Au: sub_12FC5A(m, obj); break;
    case 0x0012FD4Cu: sub_12FD4C(m, obj); break;
    default:
        return -1; /* unknown segment handler */
    }

    sub_11AE62(m, obj, 0x000186A0u, 2u);
    sub_11C096(m, obj);
    wr8(m, obj + 0x1Du, rd8(m, rd32(m, obj + 0x08u) + 0x1Du));
    if ((int16_t)rd16(m, obj + 0x5Eu) < 0)
        wr16(m, obj + 0x5Eu, 0u);
    if ((int16_t)rd16(m, obj + 0x5Eu) > 0x0FA0)
        wr16(m, obj + 0x5Eu, 0x0FA0u);
    sub_12EC20(m, obj);
    sub_12ECDC(m, obj);
    if (rd8(m, obj + 0x3Eu) != 0u)
        wr32(m, obj + 0x10u, 0x00030C74u);

    if (rd8(m, F3_A5 + 0x30F2u) & 0x02u) {
        wr32(m, obj + 0x08u, obj);
        wr32(m, obj + 0x0Cu, 4u);
        return Obj_13E4EE_Tick(m, obj);
    }
    if (rd8(m, F3_A5 + 0x30F2u) & 0x04u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x0012EB90u);
    return 0;
}
