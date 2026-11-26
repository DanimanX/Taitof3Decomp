/* ROM 0x0FADDE: articulated tentacle/arm segment steering tick (A0 = object).
 * Syncs +0x1D and +0x5C from parent (+0x08), transfers +0x35/+0x36 score
 * ownership when parent+0x35 == 0, aims/picks target angle (+0x4C) when
 * +0x3E == 0 (via 0x0D07E4 + 0x0D0E3A) or copies from previous segment (+0x50),
 * and clamps/steers +0x44, +0x48, and +0x0C toward +0x4C (0x0FAF34 + 0x0D0C20)
 * using speed table 0x0FAF92[+0x3E.w]. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

#define REC_0FAFC2 0x000FAFC2u
#define TBL_0FAF92 0x000FAF92u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t idx = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)idx * 2u);
}

/* ROM 0x0D07E4 + 0x0D08A0: test one player record against aim cone at a1. */
static int16_t sub_0D07E4_player(const f3mem_t *m, uint32_t obj, uint32_t a1,
                                 uint32_t player, uint16_t *out_dx, uint16_t *out_dy)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return -1;

    uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
    int16_t d3w = (int16_t)((int16_t)rd16(m, obj + 0x18u) - (int16_t)rd16(m, player + 0x18u)
                            + (int16_t)rd16(m, a4) - 0x0F);
    int16_t a3w = d3w;
    if (d3w < 0)
        d3w = (int16_t)(-d3w);
    if ((uint16_t)d3w > rd16(m, a1 + 0x0Au))
        return -1;

    a4 -= 2u;
    int16_t d2w = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, player + 0x14u)
                            + (int16_t)rd16(m, a4) - 0x17);
    int16_t a4w = d2w;
    if (d2w < 0) {
        d2w = (int16_t)(-d2w);
        if ((uint16_t)d2w > rd16(m, a1 + 0x0Au))
            return -1;
        if ((uint16_t)d2w < rd16(m, a1 + 0x08u) && (uint16_t)d3w < rd16(m, a1 + 0x08u))
            return -1;
    } else {
        if ((uint16_t)d2w > rd16(m, a1 + 0x0Au))
            return -1;
        int16_t d0w = (int16_t)(rd16(m, a1 + 0x08u) - 0x1Eu);
        if (d2w < d0w && (uint16_t)d3w < rd16(m, a1 + 0x08u))
            return -1;
    }

    *out_dx = (uint16_t)d2w;
    *out_dy = (uint16_t)d3w;

    uint16_t ang = Obj_Atan256(m, a3w, a4w);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);
    if (lo > hi) {
        if (ang >= lo || ang <= hi)
            return (int16_t)ang;
    } else {
        if (ang >= lo && ang <= hi)
            return (int16_t)ang;
    }
    return -1;
}

/* ROM 0x0D07E4: aim cone over both players. */
static void sub_0D07E4(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t ang1 = sub_0D07E4_player(m, obj, a1, 0x0041B190u, &dx1, &dy1);
    int16_t ang2 = sub_0D07E4_player(m, obj, a1, 0x0041B21Au, &dx2, &dy2);

    int16_t chosen = ang2;
    if (ang1 < 0) {
        if (ang2 < 0) {
            *d0_io = setw(*d0_io, (uint16_t)ang2);
            return;
        }
    } else if (ang2 >= 0) {
        uint16_t m1 = (dx1 <= dy1) ? dx1 : dy1;
        uint16_t m2 = (dx2 <= dy2) ? dx2 : dy2;
        if (m2 >= m1)
            chosen = ang1;
    } else {
        chosen = ang1;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    int16_t chance = (int16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    int16_t rng = (int16_t)Rng_Mix1D60(m);
    *d0_io = setw(*d0_io, (uint16_t)rng);
    if (chance > rng) {
        uint16_t v = rd16(m, a1 + 0x12u);
        *d0_io = setw(*d0_io, v);
        wr8(m, obj + 0x23u, (uint8_t)v);
    }
}

/* ROM 0x0D0C20: step 16.16 angle d0 toward d2w by step d1 (16.16) mod 360.0. */
static uint32_t sub_0D0C20(uint32_t d0, uint32_t *d1_io, uint16_t d2w)
{
    uint32_t d1 = *d1_io;
    uint16_t d0w = (uint16_t)(d0 >> 16);
    uint16_t d1w = (uint16_t)(d1 >> 16);
    int16_t d3w = (int16_t)(d2w - d0w);
    if (d3w < 0)
        d3w = (int16_t)(d3w + 0x0168);
    if ((uint16_t)d3w > 0x00B4u) {
        d3w = (int16_t)(-d3w + 0x0168);
        if ((uint16_t)d3w <= d1w) {
            *d1_io = swap32(d1);
            return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
        }
        int32_t res = (int32_t)(d0 - d1);
        if (res < 0)
            res += 0x01680000;
        return (uint32_t)res;
    } else {
        if ((uint16_t)d3w <= d1w) {
            *d1_io = swap32(d1);
            return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
        }
        uint32_t res = d0 + d1;
        if (res >= 0x01680000u)
            res -= 0x01680000u;
        return res;
    }
}

/* ROM 0x0FAF34: clamp target angle (+0x4C) to [+0x58 - +0x5C, +0x58 + +0x5C]. */
static uint16_t sub_0FAF34(const f3mem_t *m, uint32_t obj)
{
    uint16_t base = rd16(m, obj + 0x58u);
    uint16_t lim = rd16(m, obj + 0x5Cu);
    uint16_t hi = (uint16_t)(base + lim);
    if (hi >= 0x0168u)
        hi = (uint16_t)(hi - 0x0168u);
    int16_t lo_s = (int16_t)(base - lim);
    if (lo_s < 0)
        lo_s = (int16_t)(lo_s + 0x0168);
    uint16_t lo = (uint16_t)lo_s;
    uint16_t tgt = rd16(m, obj + 0x4Cu);

    if (lo <= hi) {
        if (tgt <= hi && tgt >= lo)
            return tgt;
    } else {
        if (tgt <= hi || tgt >= lo)
            return tgt;
    }
    int16_t diff = (int16_t)(tgt - base);
    if (diff < 0)
        diff = (int16_t)(diff + 0x0168);
    return ((uint16_t)diff < 0x00B4u) ? hi : lo;
}

int Fn_0FADDE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);
    r->a[1] = parent;

    uint8_t pst = rd8(m, parent + 0x06u);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | pst;
    if (pst == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr8(m, obj + 0x1Du, 0u);
    if (pst == 4u) {
        Obj_OrbitDebug(m, obj);
        return 0;
    }

    if (rd8(m, parent + 0x35u) == 0u) {
        wr8(m, parent + 0x35u, rd8(m, obj + 0x35u));
        wr32(m, parent + 0x36u, rd32(m, obj + 0x36u));
        wr8(m, obj + 0x35u, 0u);
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            wr32(m, obj + 0x24u, 0x00080000u);
            wr32(m, parent + 0x24u, 0u);
        }
    }

    wr8(m, obj + 0x1Du, rd8(m, parent + 0x1Du));               /* 0x0FAE26 */
    wr16(m, obj + 0x5Cu, rd16(m, parent + 0x08u));            /* 0x0FAE2C */

    if (rd16(m, obj + 0x3Eu) == 0u) {                         /* 0x0FAE32 */
        wr16(m, obj + 0x58u, rd16(m, obj + 0x44u));
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            wr32(m, obj + 0x68u, 0x000FAFE2u);
            r->a[1] = REC_0FAFC2;
            sub_0D07E4(m, obj, REC_0FAFC2, &r->d[0]);
            if (rd8(m, obj + 0x23u) != 0u) {
                uint16_t rng = Rng_Mix1D60(m);
                int16_t ang = (int16_t)((rng & 0x7Fu) - 0x40 + (int16_t)rd16(m, obj + 0x42u));
                if (ang < 0)
                    ang = (int16_t)(ang + 0x0168);
                else if ((uint16_t)ang >= 0x0168u)
                    ang = (int16_t)(ang - 0x0168);
                r->d[0] = setw(r->d[0], (uint16_t)ang);
                wr16(m, obj + 0x4Cu, (uint16_t)ang);
                wr8(m, obj + 0x23u, 0x14u);
            } else {
                uint16_t d2w = Rng_Mix1D60(m);
                d2w = (uint16_t)(d2w + (Rng_Mix1D60(m) & 0x3Fu));
                uint16_t r3 = Rng_Mix1D60(m);
                d2w = (uint16_t)(d2w + (r3 & 0x1Fu));
                r->d[0] = setw(r->d[0], r3 & 0x1Fu);
                wr16(m, obj + 0x4Cu, d2w);
                wr8(m, obj + 0x23u, 0x28u);
            }
        }
    } else {                                                  /* 0x0FAEC8 */
        uint32_t prev = rd32(m, obj + 0x50u);
        wr16(m, obj + 0x58u, rd16(m, prev + 0x58u));
        wr16(m, obj + 0x4Cu, rd16(m, prev + 0x4Cu));
    }

    /* 0x0FAED8: steer +0x44, +0x48, +0x0C */
    {
        uint16_t idx = rd16(m, obj + 0x3Eu);
        uint32_t spd_tbl = rd32(m, TBL_0FAF92 + (uint32_t)(int32_t)(int16_t)idx * 4u);
        static const uint16_t offs[3] = { 0x44u, 0x48u, 0x0Cu };
        uint32_t d0 = r->d[0];
        uint32_t d1 = r->d[1];

        for (int i = 0; i < 3; i++) {
            uint16_t tgt = sub_0FAF34(m, obj);
            d0 = rd32(m, obj + offs[i]);
            d1 = rd32(m, spd_tbl + (uint32_t)i * 4u);
            d0 = sub_0D0C20(d0, &d1, tgt);
            wr32(m, obj + offs[i], d0);
            wr32(m, obj + 0x58u, d0);
        }
        r->d[0] = d0;
        r->d[1] = d1;
    }
    return 0;
}
