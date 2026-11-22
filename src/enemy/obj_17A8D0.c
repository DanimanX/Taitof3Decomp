#include "obj_17A8D0.h"

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
    case 13: scr_frame(m, obj, 1, 1, 1); return 0;
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
    case 12: scr_dir(m, obj, 0, 0x0Eu, 1, 1, 0); return 0;
    case 6:  scr_dir(m, obj, 0, 0x0Eu, 0, 1, 1); return 0;
    case 14: scr_dir(m, obj, 0, 0x10u, 1, 1, 1); return 0;
    case 7:  scr_dir(m, obj, 1, 0x0Eu, 0, 1, 1); return 0;
    case 15: scr_dir(m, obj, 1, 0x10u, 1, 1, 1); return 0;
    case 16: scr_dir16(m, obj); return 0;
    default: return -1;
    }
}

/* ROM 0x18495E: scale +0x24 by the difficulty table 0x1849E6 */
static void sub_18495E(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x87Fu) >> 4);
    d0 = rd8(m, 0x1849E6u + d0);
    uint16_t q = (uint16_t)(rd16(m, obj + 0x24u) / 100u);
    wr16(m, obj + 0x24u, (uint16_t)(rd16(m, obj + 0x24u) + (uint16_t)(q * d0)));
}

/* ROM 0x17A870 + vtable init 0x188DE6 / 0x188DFE */
static int part_init(const f3mem_t *m, uint32_t obj)
{
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    uint32_t fn = rd32(m, a3);
    if (fn != 0x188DE6u && fn != 0x188DFEu)
        return -1;
    uint32_t a2 = rd32(m, a3 + 4u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x4Cu, 0xFFFFFFFFu);
    wr16(m, obj + 0x24u, rd16(m, a2));
    wr16(m, obj + 0x28u, rd16(m, a2 + 2u));
    wr32(m, obj + 0x2Cu, rd32(m, a2 + 4u));
    wr32(m, obj + 0x30u, rd32(m, a2 + 8u));
    wr16(m, obj + 0x34u, rd16(m, a2 + 0x0Cu));
    wr16(m, obj + 0x16u, rd16(m, a2 + 0x0Eu));
    wr16(m, obj + 0x1Au, rd16(m, a2 + 0x10u));
    if (scr_jump(m, obj, rd32(m, a2 + 0x12u)) < 0)
        return -1;
    sub_18495E(m, obj);
    wr16(m, obj + 0x1Cu, rd16(m, a2 + 0x16u));
    if (fn == 0x188DFEu && scr_jump(m, obj, 0x189316u) < 0)
        return -1;
    wr32(m, obj + 0x58u, rd32(m, obj + 0x2Cu));
    wr32(m, obj + 0x68u, rd32(m, obj + 0x30u));
    return 0;
}

/* ROM 0x17A8D0: run the part's vtable init, fn = 0x17A8E0 */
int Obj_17A8D0_Tick(const f3mem_t *m, uint32_t obj)
{
    if (part_init(m, obj) < 0)
        return -1;
    wr32(m, obj, 0x17A8E0u);
    return 0;
}
