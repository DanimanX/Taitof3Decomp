/* Object tick routine at ROM 0xEAD28 (flow-following bundle: work/disasm/bundle_EAD28.s).
 *
 * The routine is an object "generic behaviour" tick: it advances the object
 * script (ObjScript_CF1E4), runs the movement step (ObjScript_CF4B8), performs
 * the off-screen test, copies the 16.16 working position (+0x3A/+0x3E) into
 * the screen position (+0x14/+0x18), then dispatches on the state byte +0x06
 * (table 0xEAEE0) before stepping the animation, appending the hazard entry,
 * fixing up the colour byte and emitting the sprite.
 */
#include "obj_EAD28.h"
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

static uint16_t sub_0D0C9E(const f3mem_t *m);
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1);
static uint16_t sub_0D0E3A(const f3mem_t *m);

/* bfffo Dn{16:16},Dm for a zero-filled 32-bit destination: returns 16+leading
 * zero count of the low word (32 when the word is 0), MSB-first numbering. */
static uint32_t bfffo_w(uint16_t v)
{
    uint32_t n = 0u;
    if (v == 0u)
        return 32u;
    while ((v & 0x8000u) == 0u) {
        v = (uint16_t)(v << 1);
        n++;
    }
    return 16u + n;
}

/* ROM 0x006422: atan2-style angle from (D0,D1) offsets -> D0.w */
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
    uint32_t d3 = bfffo_w(src);

    d3 -= 0x1Au;
    if ((int32_t)d3 < 0) {
        uint32_t n = (0u - d3) & 63u;
        d0 = (uint16_t)(d0 >> n);
        d1 = (uint16_t)(d1 >> n);
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);

    uint16_t low = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    low = (uint16_t)(low + rd16(m, 0x006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(low & 0x00FFu);
}

/* ROM 0x00649A: distance index -> lookup table value (D0 result) */
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

    uint32_t low = (uint32_t)rd16(m, 0x028340u + (uint32_t)(int16_t)d0 * 2u);
    d0 = (d0 & 0xFFFF0000u) | low;
    d0 = (d0 << (shift & 31u)) & 0xFFFFFFFFu;
    return (uint16_t)(d0 >> 16);
}

/* ROM 0x0CBB2C: explosion / death animation tick (A0 = obj); ends by writing
 * the continuation tick address 0xCBC78 into obj+0 (+0xCBC88 on bail-out). */
static void sub_0CBB2C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    int16_t d0;

    wr8(m, obj + 0x32, 0u);
    wr8(m, obj + 0x34, 0xFFu);

    /* LAB_000cbb50: orient along the {dx,dy}.w direction record */
    a1 = rd32(m, obj + 0x10);
    d0 = (int16_t)((int16_t)(uint16_t)rd8(m, obj + 0x1E) - 0x100);
    d0 = (int16_t)((int32_t)d0 * (int32_t)(int16_t)rd16(m, a1));
    a1 += 2u;
    d0 = ASR16(d0, 5);
    wr16(m, obj + 0x18, (uint16_t)(rd16(m, obj + 0x18) - (uint16_t)d0));

    d0 = (int16_t)((int16_t)(uint16_t)rd8(m, obj + 0x1F) - 0x100);
    d0 = (int16_t)((int32_t)d0 * (int32_t)(int16_t)rd16(m, a1));
    d0 = ASR16(d0, 5);
    wr16(m, obj + 0x14, (uint16_t)(rd16(m, obj + 0x14) - (uint16_t)d0));

    d0 = sub_00649A(m,
                    (uint16_t)(rd16(m, obj + 0x14) - 0xA0),
                    (uint16_t)(rd16(m, obj + 0x18) - 0x74));
    d0 = (int16_t)(d0 << 2);
    d0 = (int16_t)(d0 - (int16_t)rd16(m, F3_A5 + 0x1D5Cu));
    if (d0 < 0)
        goto bail;

    d0 = (int16_t)(d0 << 6);
    wr16(m, obj + 0x2C, (uint16_t)d0);

    d0 = sub_006422(m,
                    (uint16_t)(rd16(m, obj + 0x14) - 0xA0),
                    (uint16_t)(rd16(m, obj + 0x18) - 0x74));
    d0 = (int16_t)(d0 << 8);
    wr16(m, obj + 0x2E, (uint16_t)d0);
    wr16(m, obj + 0x30, 0x0100u);

    /* LAB_000cbbc4: vertical flip active? */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        goto bail;

    wr16(m, obj + 0x2E, (uint16_t)(rd16(m, obj + 0x2E) + 0x200u));

    {
        uint16_t b = (uint16_t)rd8(m, obj + 0x32);
        if (b != 0u) {
            uint32_t base = rd32(m, obj + 0x0C);
            uint16_t d1 = (uint16_t)(rd16(m, obj + 0x2E) + 0x4800u);
            d1 = (uint16_t)(d1 >> (b & 15u));
            wr32(m, obj + 0x10, rd32(m, base + (uint32_t)d1 * 4u));
        }
    }

    {
        int32_t d1, d2;
        uint16_t d0w, angle;

        d0w = (uint16_t)(rd16(m, obj + 0x2C) >> 5);
        wr16(m, obj + 0x2C, (uint16_t)(rd16(m, obj + 0x2C) - d0w));

        d1 = (int32_t)(uint16_t)((uint32_t)0x100u * rd16(m, obj + 0x2C));
        d1 = ASR32(d1, 8);
        d1 = ASR32(d1, 6);
        d1 = (int32_t)(uint16_t)(d1 + (int32_t)rd16(m, F3_A5 + 0x1D5Cu));
        d2 = d1;
        d1 = (int32_t)(uint16_t)d1;

        angle = (uint16_t)(rd16(m, obj + 0x2E) >> 8);

        d1 = (int32_t)(uint16_t)((uint16_t)d1 *
            (uint16_t)(int32_t)(int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(angle & 0xFFu) * 2u));
        d1 = (int32_t)(uint32_t)((uint32_t)d1 + 0x00A00000u);
        wr32(m, obj + 0x14, (uint32_t)d1);

        d2 = (int32_t)(uint16_t)((uint16_t)d2 *
            (uint16_t)(int32_t)(int16_t)rd16(m, 0x027E40u + (uint32_t)(angle & 0xFFu) * 2u));
        d2 = (int32_t)(uint32_t)((uint32_t)d2 + 0x00740000u);
        wr32(m, obj + 0x18, (uint32_t)d2);
    }

    {
        int32_t d0l = (int32_t)(int16_t)0xFF00 * (int32_t)(int16_t)rd16(m, obj + 0x30);
        d0l = ASR32(d0l, 8);
        d0l = (int32_t)(uint16_t)(int16_t)((uint16_t)d0l + 0x100u);
        wr8(m, obj + 0x1F, (uint8_t)d0l);
        wr8(m, obj + 0x1E, (uint8_t)d0l);
    }

    {
        uint32_t a1 = rd32(m, obj + 0x10);
        int16_t d0 = (int16_t)((int16_t)(uint16_t)rd8(m, obj + 0x1E) - 0x100);
        d0 = (int16_t)((int32_t)d0 * (int32_t)(int16_t)rd16(m, a1));
        d0 = ASR16(d0, 5);
        wr16(m, obj + 0x14, (uint16_t)(rd16(m, obj + 0x14) + (uint16_t)d0));
        wr16(m, obj + 0x18, (uint16_t)(rd16(m, obj + 0x18) + (uint16_t)d0));
    }

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00, 0x000CBC78u);
    return;

bail:
    wr32(m, obj + 0x00, 0x000CBC88u);
}

/* ROM 0x0CBE34: blinking colour override (obj+0x35, obj+0x1D, obj+0x4F) */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35) != 0u) {
        if (rd8(m, obj + 0x1D) != 0x40u) {
            wr8(m, obj + 0x1D, 0x40u);
            return;
        }
        wr8(m, obj + 0x35, 0u);
        wr8(m, obj + 0x1D, 0x42u);
        return;
    }
    wr8(m, obj + 0x1D, rd8(m, obj + 0x4F));
}

/* ROM 0x0CF1D8: install a script pointer and request the first step */
static void sub_0CF1D8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x44, a1);
    wr8(m, obj + 0x07, (uint8_t)(rd8(m, obj + 0x07) | 0x02u));
}

/* ROM 0x0CF82C: re-arm the off-screen flag once the object is back on screen */
static void sub_0CF82C(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07) & 0x80u) == 0u)
        return;
    if (rd16(m, obj + 0x3E) >= 0xFFC0u || rd16(m, obj + 0x3E) > 0x00F7u)
        return;
    wr8(m, obj + 0x07, (uint8_t)(rd8(m, obj + 0x07) & (uint8_t)~0x80u));
    wr8(m, obj + 0x07, (uint8_t)(rd8(m, obj + 0x07) | 0x02u));
}

/* ROM 0x0D0678: start an animation sequence (12-byte entries) */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    int16_t d0;

    wr32(m, obj + 0x48, a1);
    wr16(m, obj + 0x4C, rd16(m, a1));
    a1 += 2u;
    d0 = (int16_t)rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1C, (uint8_t)d0);
    wr32(m, obj + 0x10, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68, rd32(m, a1));
}

/* ROM 0x0D0690: advance the animation sequence when the frame timer expires */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    int16_t d0;

    wr16(m, obj + 0x4C, (uint16_t)(rd16(m, obj + 0x4C) - 1u));
    if (rd16(m, obj + 0x4C) != 0u)
        return;

    a1 = (uint32_t)((int32_t)rd32(m, obj + 0x48) + 0x0C);
    wr32(m, obj + 0x48, a1);
    d0 = (int16_t)rd16(m, a1);
    a1 += 2u;
    if (d0 >= 0) {
        wr16(m, obj + 0x4C, (uint16_t)d0);
        d0 = (int16_t)rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1C, (uint8_t)d0);
        wr32(m, obj + 0x10, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68, rd32(m, a1));
        return;
    }

    /* loop marker: word 0 = repeat offset, non-zero = rewind, zero = end */
    if (rd16(m, a1) != 0u) {
        a1 = (uint32_t)((int32_t)a1 - (int32_t)(int16_t)rd16(m, a1));
        wr32(m, obj + 0x48, a1);
        wr16(m, obj + 0x4C, rd16(m, a1));
        a1 += 2u;
        d0 = (int16_t)rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1C, (uint8_t)d0);
        wr32(m, obj + 0x10, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68, rd32(m, a1));
    }
}

/* ROM 0x0D09B8: fire decision. If the muzzle (obj pos + offset table
 * obj+0x68) is on screen, test each live player against the window in the
 * parameter block a1 (+0x08 near band, +0x0A reach, +0x0C/+0x0E accepted
 * angle range, may wrap; +0x10 risk base; +0x12 fire timer). The angle to the
 * chosen (nearer, Chebyshev distance) player goes to obj+0x42 and, if
 * risk + 0xD0C9E() > roll (signed), obj+0x23 is loaded with the fire timer. */
static int aim_test(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t pl,
                    uint16_t *ady, uint16_t *adx, int16_t *angle)
{
    uint32_t a4 = rd32(m, obj + 0x68) + 2u;
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18) - rd16(m, pl + 0x18) + rd16(m, a4) - 0x0F);
    uint16_t a3w = dy;                                /* movea.l D3,A3 (pre-neg) */
    if ((int16_t)dy < 0) dy = (uint16_t)-dy;          /* bpl on subi.w flags */
    *ady = dy;
    if (dy > rd16(m, a1 + 0x0A)) return 0;            /* bhi */
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14) - rd16(m, pl + 0x14) + rd16(m, a4 - 2u) - 0x17);
    uint16_t a4w = dx;                                /* movea.l D2,A4 (pre-neg) */
    uint16_t near = rd16(m, a1 + 0x08);
    if ((int16_t)dx < 0) {
        dx = (uint16_t)-dx;
        *adx = dx;
        if (dx > rd16(m, a1 + 0x0A)) return 0;        /* bhi */
        if (dx < near && dy < near) return 0;         /* bcc / bcs */
    } else {
        *adx = dx;
        if (dx > rd16(m, a1 + 0x0A)) return 0;        /* bhi */
        if ((int16_t)dx < (int16_t)(uint16_t)(near - 0x1E) && dy < near) return 0; /* bge / bcs */
    }
    uint16_t ang = sub_0D0E52(m, a4w, a3w);
    uint16_t lo = rd16(m, a1 + 0x0C), hi = rd16(m, a1 + 0x0E);
    if (lo > hi) {                                     /* wrapped window */
        if (!(ang >= lo || ang <= hi)) return 0;
    } else {
        if (ang < lo || ang > hi) return 0;
    }
    *angle = (int16_t)ang;
    return 1;
}

static void sub_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t a2 = rd32(m, obj + 0x68);
    int16_t x = (int16_t)(rd16(m, obj + 0x14) + rd16(m, a2));
    if (x < 0x28 || x > 0x127) return;
    int16_t y = (int16_t)(rd16(m, obj + 0x18) + rd16(m, a2 + 2u));
    if (y < 0x10 || y > 0xDF) return;

    uint16_t d2 = 0, d3 = 0, d5 = 0, d6 = 0;
    int16_t d4 = -1, d7 = -1;
    if ((int16_t)rd16(m, P1_STATE_ADDR + 0x4C) >= 0)
        aim_test(m, obj, a1, P1_STATE_ADDR, &d3, &d2, &d4);
    if ((int16_t)rd16(m, P1_STATE_ADDR + 0x8A + 0x4C) >= 0)
        aim_test(m, obj, a1, P1_STATE_ADDR + 0x8A, &d6, &d5, &d7);

    int16_t d0 = d7;                                   /* 0xD0B62 */
    if (d4 >= 0) {
        if (d7 < 0) {
            d0 = d4;
        } else {
            if (d3 > d2) d2 = d3;                      /* max(|dx|,|dy|) P1 */
            if (d6 > d5) d5 = d6;                      /* max(|dx|,|dy|) P2 */
            if (!(d5 < d2)) d0 = d4;                   /* bcs: P2 strictly nearer */
        }
    } else if (d7 < 0) {
        return;
    }
    wr16(m, obj + 0x42, (uint16_t)d0);                 /* 0xD0B96 */
    int16_t risk = (int16_t)(rd16(m, a1 + 0x10) + sub_0D0C9E(m));
    int16_t roll = (int16_t)sub_0D0E3A(m);
    if (risk > roll)                                   /* cmp.w D0w,D1w / ble */
        wr8(m, obj + 0x23, (uint8_t)rd16(m, a1 + 0x12));
}

/* ROM 0x0D0C9E: difficulty-scaled random risk value (0..255) */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);

    /* move.w (0x9a,PC,D0w*2),D0w: full-format extension at 0xD0CA8,
     * so the table is 0xD0CA8 + 0x9A = 0xD0D42 (32 entries, no bound) */
    return rd16(m, 0x0D0D42u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D0DE4: append the object to both per-frame hit lists */
static void sub_0D0DE4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;

    if ((rd8(m, obj + 0x07) & 0x80u) != 0u)
        return;
    a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0D0E04: award D0.l score through the object's score target */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1;

    if (rd8(m, obj + 0x35) == 1u) {
        a1 = rd32(m, obj + 0x36);
        a1 = rd32(m, a1 + 0x08);
        wr32(m, a1 + 0x48, rd32(m, a1 + 0x48) + d0);
        return;
    }
    if (rd8(m, obj + 0x35) == 2u) {
        a1 = rd32(m, obj + 0x36);
        wr32(m, a1 + 0x48, rd32(m, a1 + 0x48) + d0);
    }
}

/* ROM 0x0D0E30: award D0.l score to the global score holder */
static void sub_0D0E30(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48, rd32(m, a1 + 0x48) + d0);
}

/* ROM 0x0D0E3A: D1-preserving roll for the drop decision (A5+0x1D60 LCG);
 * returns the roll in D0w. */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t n = rd16(m, F3_A5 + 0x1D60u);
    uint16_t t = (uint16_t)(n << 2);
    n = (uint16_t)(n + t);
    n = (uint16_t)(n + 1u);
    wr16(m, F3_A5 + 0x1D60u, n);
    return (uint16_t)(n >> 8);   /* lsr.w #8,D0w */
}

/* ROM 0x0D0E52: octant index -> 0x168-graded table value, 0 on the edge */
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
{
    uint16_t d0 = d0_in, d1 = d1_in, d2 = 0u, d3;
    uint32_t d1l;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)-d0;
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)-d1;
    }
    d3 = bfffo_w(((uint16_t)d1 > (uint16_t)d0) ? d1 : d0);
    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        uint32_t n = (uint32_t)(-((int32_t)(int16_t)d3)) & 63u;
        d0 = (uint16_t)(d0 >> n);
        d1 = (uint16_t)(d1 >> n);
    }
    if (d2 != 2u && d2 != 4u) {
        uint16_t t = d0;
        d0 = d1;
        d1 = t;
    }
    d1l = (uint32_t)(uint16_t)(d1 << 6);
    d1l += d0;
    {
        uint8_t tbl = rd8(m, 0x0D0EBCu + (d1l & 0xFFFFu));
        uint16_t base = rd16(m, 0x0D0EB4u + (uint32_t)d2);   /* (d8,PC,D2.w*1): D2 = 0/2/4/6 */
        uint16_t v = (uint16_t)((uint16_t)tbl + base);
        return (v == 0x0168u) ? 0u : v;
    }
}

/* jump-table entry 0 (state 0): idle / respawn animation select */
static int sub_0EAEEC(const f3mem_t *m, uint32_t obj)
{
    sub_0CF82C(m, obj);

    if ((rd8(m, obj + 0x07) & 0x80u) != 0u)
        return 0;

    /* lea (0x1bc,PC),A1 with PC = 0x0EAF00 -> 0x0EB0BC */
    {
        uint32_t a1 = 0x000EB0BCu;
        if (rd8(m, obj + 0x22) != 0u)
            a1 = 0x000EB17Cu; /* lea (0x27c,PC),A1 with PC = 0x0EAF08 */
        sub_0D0678(m, obj, a1);
    }
    wr8(m, obj + 0x06, 1u);
    return 0;
}

/* jump-table entry 1 (state 1): wait for the player to line up vertically */
static int sub_0EAF1A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    int16_t d0;

    a1 = P1_STATE_ADDR;
    if ((int16_t)rd16(m, a1 + 0x4C) >= 0) {
        d0 = (int16_t)(rd16(m, obj + 0x18) - rd16(m, a1 + 0x18));
        d0 = (int16_t)(d0 - 7);
        if (d0 < 0)
            d0 = (int16_t)-d0;
        if ((uint16_t)d0 <= 0x1Eu)
            goto acquire;
    }

    a1 = P1_STATE_ADDR + 0x8A;
    if ((int16_t)rd16(m, a1 + 0x4C) < 0)
        return 0;
    d0 = (int16_t)(rd16(m, obj + 0x18) - rd16(m, a1 + 0x18));
    d0 = (int16_t)(d0 - 7);
    if (d0 < 0)
        d0 = (int16_t)-d0;
    if ((uint16_t)d0 > 0x1Eu)
        return 0;

acquire:
    {
        /* lea (0x11e,PC),A1 with PC = 0x0EAF6E -> 0x0EB08C */
        uint32_t s = 0x000EB08Cu;
        if (rd8(m, obj + 0x22) != 0u)
            s = 0x000EB0A2u; /* lea (0x128,PC),A1 with PC = 0x0EAF7A */
        sub_0CF1D8(m, obj, s);
    }
    wr8(m, obj + 0x06, 2u);
    return 0;
}

/* jump-table entry 2 (state 2): attacking; hit test + child spawn on drop */
static int sub_0EAF8C(const f3mem_t *m, uint32_t obj)
{
    uint16_t b = (uint16_t)rd8(m, obj + 0x23);
    uint32_t child;

    if (b != 0u) {
        wr8(m, obj + 0x23, (uint8_t)(b - 1u));
        if (rd8(m, obj + 0x23) != 0u)
            return 0;
    }

    /* lea (0x54,PC),A1 with PC = 0x0EAF9E -> 0x0EAFF2 */
    sub_0D09B8(m, obj, 0x000EAFF2u);

    if (rd8(m, obj + 0x23) == 0u)
        return 0;

    child = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
    if ((int32_t)child < 0)
        return 0;
    wr32(m, child + 0x14, rd32(m, obj + 0x14));
    wr16(m, child + 0x14, (uint16_t)(rd16(m, child + 0x14) + 8u));
    wr32(m, child + 0x18, rd32(m, obj + 0x18));
    wr16(m, child + 0x18, (uint16_t)(rd16(m, child + 0x18) + 8u));
    wr16(m, child + 0x58, rd16(m, obj + 0x42));
    wr16(m, child + 0x10, 0u);
    return 0;
}

/* Shared object death tails (ROM 0xCEE48..0xCEEEE), all acting on the
 * object itself (A0). d0 is the live D0 register: `move.b (0x4e,A0),D0b`
 * only replaces its low byte before it is pushed as the spawn argument. */
static void tail_spawn_bonus(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4E);
    if (b)
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, (d0 & 0xFFFFFF00u) | b);
}

static int tail_cee52(const f3mem_t *m, uint32_t obj, uint32_t d0)  /* ROM 0x0CEE52 */
{
    tail_spawn_bonus(m, obj, d0);
    Obj_StartDying(m, obj);                                         /* 0xCEE74 */
    return 0;
}

static int tail_cee48(const f3mem_t *m, uint32_t obj, uint32_t d0)  /* ROM 0x0CEE48 */
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) { Obj_StartDying(m, obj); return 0; }
    return tail_cee52(m, obj, d0);
}

static int tail_ceea2(const f3mem_t *m, uint32_t obj, uint32_t d0)  /* ROM 0x0CEEA2 */
{
    tail_spawn_bonus(m, obj, d0);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) &&                        /* 0xCEEC4 */
        (int16_t)rd16(m, obj + 0x14) >= -0x0A && (int16_t)rd16(m, obj + 0x14) <= 0x011C) {
        wr8(m, obj + 0x1D, rd8(m, obj + 0x4F));
        sub_0CBB2C(m, obj);
        return 0;
    }
    wr32(m, obj, 0x000CEEF0u);                                      /* 0xCEEE8 */
    return 0;
}

static int tail_cee9a(const f3mem_t *m, uint32_t obj, uint32_t d0)  /* ROM 0x0CEE9A */
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) { Obj_StartDying(m, obj); return 0; }
    return tail_ceea2(m, obj, d0);
}

static int tail_eae1c(const f3mem_t *m, uint32_t obj, uint32_t d0)  /* ROM 0x0EAE1C */
{
    uint32_t parent = rd32(m, obj + 0x08);
    uint8_t n = (uint8_t)(rd8(m, parent + 0x22) - 1u);
    wr8(m, parent + 0x22, n);
    if (n || (rd8(m, parent + 0x07) & 0x01u))
        return tail_cee48(m, obj, d0);
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) { Obj_StartDying(m, obj); return 0; }
    sub_0D0E04(m, obj, 0x000007D0u);                 /* last of the formation */
    d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x000007D0u);
    return tail_cee52(m, obj, d0);
}

static int tail_eae6e(const f3mem_t *m, uint32_t obj)               /* ROM 0x0EAE6E */
{
    uint32_t parent = rd32(m, obj + 0x08);
    wr8(m, parent + 0x22, (uint8_t)(rd8(m, parent + 0x22) - 1u));
    wr8(m, parent + 0x07, (uint8_t)(rd8(m, parent + 0x07) | 0x01u));
    Obj_StartDying(m, obj);
    return 0;
}

static int tail_eae82(const f3mem_t *m, uint32_t obj)               /* ROM 0x0EAE82 */
{
    sub_0D0E30(m, 0x000000C8u);
    uint32_t parent = rd32(m, obj + 0x08);
    uint8_t n = (uint8_t)(rd8(m, parent + 0x22) - 1u);
    wr8(m, parent + 0x22, n);
    if (n || (rd8(m, parent + 0x07) & 0x01u))
        return tail_cee9a(m, obj, 0x000000C8u);
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) { Obj_StartDying(m, obj); return 0; }
    sub_0D0E30(m, 0x000007D0u);
    uint32_t d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x000007D0u);
    return tail_ceea2(m, obj, d0);
}

/* ROM 0x0EAD28 */
int Obj_EAD28_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    uint8_t state;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
        (int16_t)rd16(m, obj + 0x14) >= -0x0A &&
        (int16_t)rd16(m, obj + 0x14) <= 0x011C) {
        wr8(m, obj + 0x1D, rd8(m, obj + 0x4F));
        goto LAB_000eae82;
    }

    if ((int32_t)rd32(m, obj + 0x24) <= 0)
        goto LAB_000eadcc;

    /* LAB_000ead54: normal alive frame */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto LAB_000eadd8;

    /* advance 16.16 working position by the per-frame scroll velocity */
    wr32(m, obj + 0x3E, rd32(m, obj + 0x3E) - rd32(m, F3_A5 + 0x4228u));

    if ((rd8(m, obj + 0x07) & 0x02u) != 0u) {
        wr8(m, obj + 0x07, (uint8_t)(rd8(m, obj + 0x07) & (uint8_t)~0x02u));
        if (ObjScript_CF1E4(m, obj) != 0)
            return -1;
    }

    if (rd8(m, obj + 0x20) != 0u) {
        /* lea (0x272,PC),A1 at 0xEAD7E: PC = extension word 0xEAD80 -> 0xEAFF2 */
        if (ObjScript_CF4B8(m, obj, 0x000EAFF2u) != 0)
            return -1;
    }

    Obj_FlagOffscreen(m, obj);

    if ((rd8(m, obj + 0x07) & 0x08u) != 0u)
        goto LAB_000eae6e;

    wr32(m, obj + 0x14, rd32(m, obj + 0x3A));
    wr32(m, obj + 0x18, rd32(m, obj + 0x3E));

    state = rd8(m, obj + 0x06);
    switch (state) {
    case 0u:
        if (sub_0EAEEC(m, obj) != 0)
            return -1;
        break;
    case 1u:
        if (sub_0EAF1A(m, obj) != 0)
            return -1;
        break;
    case 2u:
        if (sub_0EAF8C(m, obj) != 0)
            return -1;
        break;
    default:
        /* dispatch through table 0xEAEE0 (3 entries) */
        return -1; /* unimplemented: ROM 0x0EAEAA */
    }

    sub_0D0690(m, obj);
    sub_0D0DE4(m, obj);
    sub_0CBE34(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;

LAB_000eadcc:
    /* LAB_000eadcc: defeated object awards 200 points */
    sub_0D0E04(m, obj, 0x000000C8u);

LAB_000eadd8:
    /* LAB_000eadd8: spawn the death object (group 3, 2 groups) */
    a1 = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
    if ((int32_t)a1 >= 0) {
        wr32(m, a1 + 0x14, rd32(m, obj + 0x14));
        wr16(m, a1 + 0x14, (uint16_t)(rd16(m, a1 + 0x14) + 8u));
        wr32(m, a1 + 0x18, rd32(m, obj + 0x18));
        wr16(m, a1 + 0x18, (uint16_t)(rd16(m, a1 + 0x18) + 8u));
        wr16(m, a1 + 0x58, rd16(m, obj + 0x58));
        wr16(m, a1 + 0x60, rd16(m, obj + 0x60));
    }

    /* 0xEAE1C: D0 = Obj_Spawn result (pointer or -1) */
    return tail_eae1c(m, obj, a1);

LAB_000eae6e:
    return tail_eae6e(m, obj);

LAB_000eae82:
    return tail_eae82(m, obj);
}
