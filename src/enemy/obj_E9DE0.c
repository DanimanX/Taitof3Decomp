#include "obj_E9DE0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
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

/* ROM 0x00649A */
static uint16_t sub_00649A(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
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
    return (uint16_t)d0;
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
    d0 = sub_00649A(m, dx, dy);
    d0 = (uint16_t)(d0 << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0 < (int16_t)thresh;
    d0 = (uint16_t)(d0 - thresh);
    if (is_lt) {
        wr32(m, obj + 0x00u, 0x000CBC88u);
        return;
    }

    d0 = (uint16_t)(d0 << 6);
    wr16(m, obj + 0x2Cu, d0);

    dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    d0 = sub_006422(m, dx, dy);
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
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
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
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

/* ROM 0x0CF7FE */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if (!(flags & 0x80u))
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;

    flags = (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u);
    wr8(m, obj + 0x07u, flags);
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
/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit_by = rd8(m, obj + 0x35u);
    if (hit_by == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit_by == 2u) {
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


/* ROM 0x0CEF0E: leave the screen (or detonate during a bomb) */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        Obj_StartDying(m, obj);
        return;
    }
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            sub_0CBB2C(m, obj);
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

/* ROM 0x0CF1D8: start object script */
static void sub_0CF1D8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x44u, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
}

/* ROM 0x0E9F9A: player proximity check (|dx - 15| <= range) */
static int sub_0E9F9A_near(const f3mem_t *m, uint32_t obj, uint32_t pl)
{
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) + 0xFFF1u);
    if ((int16_t)d0 < 0)
        d0 = (uint16_t)(0u - d0);
    uint16_t d1 = (uint16_t)(((uint16_t)rd8(m, F3_A5 + 0x087Fu) + 1u) >> 2);
    d1 = (uint16_t)(d1 + 0x00C0u);
    d1 = (uint16_t)(((uint32_t)d1 * 0x96u) & 0xFFFFu);
    d1 = (uint16_t)(d1 >> 8);
    return d0 <= d1;
}

/* ROM 0x0E9F9A */
static void sub_0E9F9A(const f3mem_t *m, uint32_t obj)
{
    int hit = 0;
    if ((int16_t)rd16(m, 0x41B190u + 0x4Cu) >= 0 && sub_0E9F9A_near(m, obj, 0x41B190u))
        hit = 1;
    else if ((int16_t)rd16(m, 0x41B21Au + 0x4Cu) >= 0 && sub_0E9F9A_near(m, obj, 0x41B21Au))
        hit = 1;
    if (hit) {
        sub_0CF1D8(m, obj, 0x000EA0B0u);
        wr8(m, obj + 0x06u, 0x03u);
    }
}

/* ROM 0x0E9DE0 */
int Obj_E9DE0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x0000012Cu);
        goto death_spawn;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto death_spawn;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (f7 & 0x02u) {
            if (ObjScript_CF1E4(m, obj) != 0)
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        /* lea (0x228,PC),A1 at 0xE9E12 -> 0xEA03C */
        if (ObjScript_CF4B8(m, obj, 0x000EA03Cu) != 0)
            return -1;
    }

    sub_0CF7FE(m, obj);
    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u)
        goto die;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if (rd16(m, obj + 0x60u) < 0x000Au) {
        uint16_t d0 = (uint16_t)(0x000Au - rd16(m, obj + 0x60u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
    }

    {
        uint8_t d0b = rd8(m, obj + 0x22u);
        if (d0b != 0u)   /* table (0x2d4,PC,D0w*4) at 0xE9E5E -> 0xEA134 */
            sub_0D0678(m, obj, rd32(m, 0x000EA134u + (uint32_t)d0b * 4u));
    }

    sub_0D0690(m, obj);
    if ((int16_t)rd16(m, obj + 0x68u) >= 0) {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0017u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x000Au));
            wr16(m, a1 + 0x58u, 0x005Au);
            wr16(m, a1 + 0x10u, 0u);
        }
    }

    /* jsr ([0xac,PC],D0w*4) at 0xE9EC6: table 0xE9F74 */
    switch (rd8(m, obj + 0x06u)) {
    case 1u: /* ROM 0x0E9F88 */
        if (!(rd8(m, obj + 0x07u) & 0x80u))
            wr8(m, obj + 0x06u, 0x02u);
        break;
    case 2u:
        sub_0E9F9A(m, obj);
        break;
    case 3u: /* ROM 0x0EA020 */
        if (rd16(m, obj + 0x4Cu) == 0u) {
            sub_0CF1D8(m, obj, 0x000EA122u);
            wr8(m, obj + 0x06u, 0x04u);
        }
        break;
    case 4u: /* ROM 0x0EA03A */
        break;
    default:
        return -1; /* never reached here: side effects already applied */
    }

    sub_0D0DC2(m, obj);
    sub_0CBE34(m, obj);
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x0E9F5E */
            sub_0D0E30(m, 0x0000012Cu);
            wr8(m, obj + 0x06u, 0u);
            sub_0CEF0E(m, obj);
        }
    }
    return 0;

death_spawn:
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC328u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0014u));
            wr16(m, a1 + 0x58u, 0x00B4u);
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
    }
die:
    wr8(m, obj + 0x06u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
