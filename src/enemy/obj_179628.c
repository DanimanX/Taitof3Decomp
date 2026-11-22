#include "obj_179628.h"
#include "obj_1795CA.h"
#include "obj_17963C.h"
#include "obj_179650.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* Animation engine of the 0x179A5C table (entry = {init.w, tick.w}):
 * op 12 = ROM 0x179628 (init 0x17960A), op 14 = ROM 0x17963C (init 0x17961E),
 * op 15 = ROM 0x179650 (init 0x179632). The frame timer +0x3E paces the
 * script, +0x3A walks the frame records and +0x40 holds the current op. Both
 * threaded ops (init 0x17960A / 0x17961E / 0x179632 and their inits 0x1795F6
 * / 0x179600 / 0x179614) are not translated yet and return -1.
 *
 * The six plain sprite ops of the same table (0, 3, 5, 8, 11, 13) are kept
 * here, in the shape obj_17A8E0.c gives them, because their ticks live in ROM
 * (0x179664, 0x179698, 0x1796C8, 0x179680, 0x1796AC, 0x1796EC) where they run
 * through the same timed-record machinery. */

/* ROM 0x1794A4: D0w = random in [-(d0-1)..d0-1]-ish via LCG at A5+0x4B3C */
static uint16_t sub_1794A4(const f3mem_t *m, uint32_t d0)
{
    uint16_t r = (uint16_t)d0;
    if (d0 != 0u) {
        int off = 0;
        while (!(d0 & (0x80000000u >> off)))
            off++;
        int16_t d1w = (int16_t)(off - 16);
        uint16_t d2w = rd16(m, 0x179530u + (uint32_t)((int32_t)d1w * 2));
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & d2w);
        if ((int16_t)r < (int16_t)v)
            v = (uint16_t)(v - r);
        r = v;
        if (rd8(m, F3_A5 + 0x4B3Cu) & 1u)
            r = (uint16_t)(0u - r);
    }
    uint32_t s = rd32(m, F3_A5 + 0x4B3Cu) * 5u - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (s << 16) | (s >> 16));
    return r;
}

/* ---- frame script engine, op table 0x179A5C {init, tick} ---- */
static void scr_ofs(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 1u)));
}

/* timed frame record at +0x3A: dur.w spr.l [1c.w] [2c.l 30.l] [dx.b dy.b] */
static void scr_frame(const f3mem_t *m, uint32_t obj, int has1c, int hasbox, int hasofs)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    if (has1c) { wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u; }
    if (hasbox) {
        wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    }
    if (hasofs)
        scr_ofs(m, obj, a1);
}

/* directional record selected by the angle byte at +0x20 */
static void scr_dir(const f3mem_t *m, uint32_t obj, int fine, uint32_t size,
                    int has1c, int hasbox, int hasofs)
{
    uint8_t b = rd8(m, obj + 0x20u);
    uint16_t idx = fine ? (uint16_t)((uint8_t)(b + 4u) >> 3) : (uint16_t)((uint8_t)(b + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + idx * size;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    if (has1c) { wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u; }
    if (hasbox) {
        wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    }
    if (hasofs)
        scr_ofs(m, obj, a1);
}

static void scr_dir16(const f3mem_t *m, uint32_t obj)   /* ROM 0x179A1E */
{
    int8_t b = (int8_t)(uint8_t)(rd8(m, obj + 0x20u) - 0x20u);
    if (b < 0) b = 0;
    if (b > 0x40) b = 0x40;
    uint16_t idx = (uint16_t)((uint8_t)(b + 2) >> 2);
    uint32_t a1 = rd32(m, obj + 0x3Au) + idx * 6u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
}

static int scr_tick(const f3mem_t *m, uint32_t obj, uint16_t op);

static int scr_init(const f3mem_t *m, uint32_t obj, uint16_t op)
{
    switch (op) {
    case 0:  scr_frame(m, obj, 0, 0, 0); return 0;
    case 8:  scr_frame(m, obj, 1, 0, 0); return 0;
    case 3:  scr_frame(m, obj, 0, 1, 0); return 0;
    case 11: scr_frame(m, obj, 1, 1, 0); return 0;
    case 5:  scr_frame(m, obj, 0, 1, 1); return 0;
    case 13: return Obj_1795CA_Tick(m, obj);
    case 1: case 2: case 4: case 6: case 7: case 9: case 10:
    case 12: case 14: case 15: case 16:
        wr16(m, obj + 0x3Eu, 1u);
        return scr_tick(m, obj, op);
    default:
        return -1;
    }
}

/* jump: A1 -> {op.w, data...} */
static int scr_jump(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t op = rd16(m, a1);
    wr16(m, obj + 0x40u, op);
    wr32(m, obj + 0x3Au, a1 + 2u);
    return scr_init(m, obj, op);
}

static int scr_timed(const f3mem_t *m, uint32_t obj, uint16_t op, uint32_t step, int ofs)
{
    int16_t t = (int16_t)rd16(m, obj + 0x3Eu);
    if (t == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + step;
        int16_t w = (int16_t)rd16(m, a1);
        if (w > 0) {
            wr32(m, obj + 0x3Au, a1);
            scr_init(m, obj, op);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        }
        if (w == 0) {
            int r = scr_jump(m, obj, rd32(m, a1 + 2u));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return r;
        }
        t = 1; /* w < 0: hold the frame */
    }
    if (t > 0)
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    else if (!ofs)
        return 0;
    if (ofs)
        scr_ofs(m, obj, rd32(m, obj + 0x3Au) + step - 2u);
    return 0;
}

static int scr_tick(const f3mem_t *m, uint32_t obj, uint16_t op)
{
    switch (op) {
    case 0:  return scr_timed(m, obj, 0, 6u, 0);
    case 8:  return scr_timed(m, obj, 8, 8u, 0);
    case 3:  return scr_timed(m, obj, 3, 0x0Eu, 0);
    case 11: return scr_timed(m, obj, 11, 0x10u, 0);
    case 5:  return scr_timed(m, obj, 5, 0x10u, 1);
    case 13: return scr_timed(m, obj, 13, 0x12u, 1);
    case 1:  scr_dir(m, obj, 0, 4u, 0, 0, 0); return 0;
    case 9:  scr_dir(m, obj, 0, 6u, 1, 0, 0); return 0;
    case 2:  scr_dir(m, obj, 1, 4u, 0, 0, 0); return 0;
    case 10: scr_dir(m, obj, 1, 6u, 1, 0, 0); return 0;
    case 4:  scr_dir(m, obj, 0, 0x0Cu, 0, 1, 0); return 0;
    case 12: return Obj_179628_Tick(m, obj);                    /* 0x179628 */
    case 6:  scr_dir(m, obj, 0, 0x0Eu, 0, 1, 1); return 0;
    case 14: return Obj_17963C_Tick(m, obj);                    /* 0x17963C */
    case 7:  scr_dir(m, obj, 1, 0x0Eu, 0, 1, 1); return 0;
    case 15: return Obj_179650_Tick(m, obj);                    /* 0x179650 */
    case 16: scr_dir16(m, obj); return 0;
    default: return -1;
    }
}

/* ROM 0x17BA76: hit point between obj and the attacker at +0x36 (D0w < 0: none) */
static void sub_17BA76(const f3mem_t *m, uint32_t obj, uint16_t *o0, uint16_t *o1)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u) {
        if ((rd16(m, F3_A5 + 0x878u) % 3u) != 0u) { *o0 = 0xFFFFu; return; }
        uint16_t d2 = (uint16_t)((uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)) >> 1);
        uint16_t d3 = (uint16_t)((uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)) >> 1);
        uint16_t r = sub_1794A4(m, d3);
        *o1 = (uint16_t)(r + rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u) + d3);
        r = sub_1794A4(m, d2);
        *o0 = (uint16_t)(r + rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu) + d2);
        return;
    }
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u && (rd16(m, F3_A5 + 0x878u) & 3u) != 0u) { *o0 = 0xFFFFu; return; }
    int16_t d0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
    int16_t d1 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
    int16_t d2 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
    int16_t d3 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
    int16_t d4 = d2 > d0 ? d2 : d0, d5 = d3 > d1 ? d3 : d1;
    d0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
    d1 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
    d2 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
    d3 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
    int16_t d6 = d2 < d0 ? d2 : d0, d7 = d3 < d1 ? d3 : d1;
    *o0 = (uint16_t)(d4 + ((uint16_t)(d6 - d4) >> 1));
    *o1 = (uint16_t)(d5 + ((uint16_t)(d7 - d5) >> 1));
}

static void hit_spark(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    uint16_t d0, d1 = 0;
    sub_17BA76(m, obj, &d0, &d1);
    if ((int16_t)d0 < 0)
        return;
    d0 = (uint16_t)(d0 - rd16(m, obj + 0x50u));
    d1 = (uint16_t)(d1 - rd16(m, obj + 0x54u));
    Obj_Spawn(m, obj, fn, 5u, 2u, ((uint32_t)d0 << 16) | d1);
}

/* ROM 0x17B9BA */
static void sub_17B9BA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (sub_1794A4(m, 5u) != 0u)
        return;
    hit_spark(m, obj, 0x17A122u);
}

/* Both lists carry every live animated enemy (obj_17A8E0.c keeps the same
 * body as part_lists). */
static void anim_lists(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* Object path of ops 12/14/15 (ROM 0x179628, 0x17963C, 0x179650): run the
 * frame script, register for collisions, hit flash, then draw. The sprite
 * centring at 0x17C05E belongs to that caller, not to these ops. */
int Obj_179628_Tick(const f3mem_t *m, uint32_t obj)
{
    if (scr_tick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    anim_lists(m, obj);
    if (rd8(m, obj + 0x1Du) == 6u)
        wr8(m, obj + 0x1Du, 8u);
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, 6u);
    else
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));
    if (rd8(m, obj + 0x34u) & 0x02u) {
        sub_17B9BA(m, obj);
        wr8(m, obj + 0x35u, 0u);
    }
    Obj_DrawSprite(m, obj);
    return 0;
}
