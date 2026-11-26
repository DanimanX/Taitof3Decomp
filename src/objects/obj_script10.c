#include "obj_script10.h"
#include "obj_shared.h"
#include "obj_shared_ext.h"
#include "obj_pool.h"
#include "../audio/sound_cmd.h"

/* The 10-byte movement script at obj+0x44, as walked by ROM 0x178982.
 * Bodies lifted from src/enemy/obj_17C05E.c (same ROM addresses), where this
 * interpreter is already exercised by a passing capture. */

/* ROM 0x178B20 */
void Obj_Script10_SetHeading(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, rd16(m, rd32(m, obj + 0x44u) + 6u));
}

/* ROM 0x178E4A: aim at the object at +0x4C. The entry's arg0 is the heading
 * step and arg1 the speed step; the heading error's sign picks the direction
 * of the speed change, then the position advances along 0x27E40. */
void Obj_Script10_SteerTo(const f3mem_t *m, uint32_t obj)
{
    uint32_t tgt = rd32(m, obj + 0x4Cu);
    if ((int32_t)tgt <= 0)
        return;

    uint16_t dx = (uint16_t)(rd16(m, tgt + 0x14u) + 0x18u - rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(rd16(m, tgt + 0x18u) + 0x10u - rd16(m, obj + 0x54u));
    uint16_t d0 = Obj_Atan2(m, dx, dy);

    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t spd = rd16(m, a1 + 8u);

    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if (diff == 0)
        spd = 0u;
    else if (diff < 0)
        spd = (uint16_t)(0u - spd);

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + spd));

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u);
    mag = (int16_t)(mag >> 6);
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + ang * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + ang * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* Dispatch one command by its ROM address. 0 on success, -1 if untranslated. */
/* ROM 0x184984: scale the heading rate +0x42 by a difficulty table.
 *
 *   bfextu $87e(a5){8:4},d0          ; difficulty selector
 *   bfextu $1849d6(pc,d0.w){0:8},d0  ; table byte (PC = extension word, so the
 *                                    ; base is 0x1849D6, not 0x1849D4)
 *   bfexts $42(a0){0:16},d1
 *   divs.w #$64,d1 ; muls.w d0,d1 ; add.w d1,$42(a0)
 *
 * Selector 0 and 1 read 0, so on the default difficulty the term is a no-op --
 * that is what the 0x17C240 capture shows (+0x42 stays at 0x400). */
static void sub_184984(const f3mem_t *m, uint32_t obj)
{
    /* bfextu $87e(a5){8:4}: bits 8..11 of the word, i.e. (v >> 4) & 0xF */
    uint16_t sel = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) >> 4) & 0x0Fu);
    uint8_t scale = rd8(m, 0x1849D6u + sel);
    int16_t d1 = (int16_t)rd16(m, obj + 0x42u);
    d1 = (int16_t)(d1 / 0x64);
    d1 = (int16_t)(d1 * (int16_t)scale);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + (uint16_t)d1));
}

/* ROM 0x178B4A: assign the record's arg0 to the heading rate, then scale it */
static void sub_178B4A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
    sub_184984(m, obj);
}

/* ROM 0x178C06: advance along the 0x27E40 sine table.
 *
 *   d0 = (0x42,A0) + arg0 ; d2 = (0x20,A0) + arg1
 *   store both, then d0 = (d0 + 0x20) >> 6 and idx = d2 >> 8
 *   (0x50,A0) += d1 * table[idx*2]
 *   (0x54,A0) += d0 * table[idx*2 + 0x80]
 *
 * The +0x80 is a BYTE offset into the table, so the second product reads the
 * cosine while the first reads the sine. */
static void sub_178C06(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = rd16(m, obj + 0x42u);
    uint16_t d2 = rd16(m, obj + 0x20u);
    uint32_t a1 = rd32(m, obj + 0x44u);
    d0 = (uint16_t)(d0 + rd16(m, a1 + 6u));
    d2 = (uint16_t)(d2 + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, d0);
    wr16(m, obj + 0x20u, d2);

    uint16_t mag = (uint16_t)((uint32_t)(int32_t)(int16_t)(uint16_t)(d0 + 0x20u) >> 6);
    uint16_t idx = (uint16_t)(d2 >> 8);
    uint32_t base = 0x0027E40u + (uint32_t)idx * 2u;
    int32_t a = (int32_t)(int16_t)rd16(m, base);           /* muls.w (a1,d2*2),d1 */
    int32_t b = (int32_t)(int16_t)rd16(m, base + 0x80u);   /* muls.w $80(a1,d2*2),d0 */
    /* d0 (the +0x80 term) lands in +0x50, d1 in +0x54 -- 0x178C3E / 0x178C42 */
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)(int16_t)mag * b));
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)((int32_t)(int16_t)mag * a));
}

static int16_t abs16(int16_t v) { return v < 0 ? (int16_t)(0u - (uint16_t)v) : v; }

/* ROM 0x179074: the aim-between-bounds script. D2 = sign-extended byte of
 * +0x42 (the heading rate); below zero the call is a no-op. Each axis whose
 * distance from the target (+0x4C/+0x4E) exceeds |D2| is picked: the heading
 * error drives 0x006422, the record's +0x06 word is added to +0x42, the
 * byte-to-byte jitter comes from 0x1794A4 (record +0x08 word) and then the
 * position moves along the 0x27E40 sine/cosine pair. */
static void scr_179074(const f3mem_t *m, uint32_t obj)
{
    uint8_t d2b = rd8(m, obj + 0x42u);              /* bfextu (0x42,A0){0:8} */
    int16_t d2 = (int16_t)d2b;
    if (d2b & 0x80u) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    int16_t ex = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t ey = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    if (!(abs16(ex) > d2 || abs16(ey) > d2)) {
        wr16(m, obj + 0x48u, 0u);                   /* 0x179104 */
        return;
    }

    uint16_t d0 = Obj_Atan2(m, (uint16_t)ex, (uint16_t)ey); /* 0x1790B2 jsr 0x6422 */
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t step = (int16_t)rd16(m, a1 + 6u);
    uint16_t jitter = rd16(m, a1 + 8u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + (uint16_t)step));

    /* 0x1790C8: and.b D1b,D0b ; asr.w #8,D1w ; add.b D1b,D0b */
    uint8_t ang = (uint8_t)((uint8_t)d0 & (uint8_t)jitter);
    ang = (uint8_t)(ang + (uint8_t)(int8_t)(int16_t)(jitter >> 8));
    wr8(m, obj + 0x20u, ang);

    uint16_t mag = (uint16_t)(rd16(m, obj + 0x42u) + 0x20u);
    int16_t m2 = (int16_t)mag >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int32_t vx = (int32_t)m2 * (int16_t)rd16(m, 0x0027E40u + (uint32_t)idx * 2u + 0x80u);
    int32_t vy = (int32_t)m2 * (int16_t)rd16(m, 0x0027E40u + (uint32_t)idx * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178CCE: step Cartesian (+0x42, +0x5C) velocity components, integrate
 * (+0x50, +0x54), and update +0x20 angle via Obj_Atan2 (0x006422). */
static void sub_178CCE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t vxw = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint16_t vyw = (uint16_t)(rd16(m, obj + 0x5Cu) + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, vxw);
    wr16(m, obj + 0x5Cu, vyw);

    uint32_t dx = ((uint32_t)(int32_t)(int16_t)(uint16_t)(vxw + 0x20u) << 8) & 0xFFFFC000u;
    uint32_t dy = ((uint32_t)(int32_t)(int16_t)(uint16_t)(vyw + 0x20u) << 8) & 0xFFFFC000u;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + dx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + dy);

    uint16_t ang = Obj_Atan2(m, vxw, vyw);
    wr8(m, obj + 0x20u, (uint8_t)ang);
}

/* ROM 0x178ECC: the steer-to command of 0x178E4A with the heading-speed
 * section replaced by a random heading byte. Where 0x178E80..0x178E9A does
 *
 *     sub.b (0x20,A0),D0 ; beq/bgt/neg + add.w D1,(0x20,A0)
 *
 * this one does  and.b D1b,D0 ; asr.w #8,D1 ; add.b D1b,D0 ; move.b D0,(0x20,A0)
 * (D1 = the record's +0x8 word), then moves along 0x27E40 exactly like its
 * sibling. The two are NOT interchangeable: the speed word +0x20 is replaced
 * rather than accumulated, so this needs its own body. */
static void scr_178ECC(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x4Cu) <= 0)
        return;                                       /* 0x178ECC tst.l/ble */

    uint32_t tgt = rd32(m, obj + 0x4Cu);              /* 0x178ED4 */
    uint16_t dx = (uint16_t)(rd16(m, tgt + 0x14u) + 0x18u - rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(rd16(m, tgt + 0x18u) + 0x10u - rd16(m, obj + 0x54u));
    uint16_t d0 = Obj_Atan2(m, dx, dy);               /* 0x178EF0 jsr 0x006422 */

    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d2 = rd16(m, a1 + 0x06u);
    uint16_t d1 = rd16(m, a1 + 0x08u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + d2));   /* 0x178F02 */

    /* 0x178F06: and.b D1b,D0b ; asr.w #8,D1w ; add.b D1b,D0b */
    uint8_t ang = (uint8_t)((uint8_t)d0 & (uint8_t)d1);
    ang = (uint8_t)(ang + (uint8_t)(int8_t)(int16_t)((int16_t)d1 >> 8));
    wr8(m, obj + 0x20u, ang);                         /* 0x178F0C */

    /* 0x178F10..0x178F38: the 0x27E40 sine/cosine pair */
    {
        int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
        uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
        uint32_t base = 0x0027E40u + (uint32_t)idx * 2u;
        int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
        int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
        wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
        wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
    }
}

/* ROM 0x178AC6 */
static void scr_178AC6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x50u, rd16(m, a1 + 6u));
    wr16(m, obj + 0x54u, rd16(m, a1 + 8u));
    wr16(m, obj + 0x52u, 0u);
    wr16(m, obj + 0x56u, 0u);
}

/* ROM 0x178B0E */
static void scr_178B0E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
    wr16(m, obj + 0x20u, rd16(m, a1 + 8u));
}

/* ROM 0x179222 */
static void scr_179222(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)rd16(m, a1 + 6u);
    if (d0 >= 0)
        wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | (0x80u >> ((uint16_t)d0 & 7u))));
    d0 = (int16_t)rd16(m, a1 + 8u);
    if (d0 >= 0)
        wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | (0x80u >> ((uint16_t)d0 & 7u))));
}

/* ROM 0x17924C */
static void scr_17924C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)rd16(m, a1 + 6u);
    if (d0 >= 0)
        wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & (uint8_t)~(0x80u >> ((uint16_t)d0 & 7u))));
    d0 = (int16_t)rd16(m, a1 + 8u);
    if (d0 >= 0)
        wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & (uint8_t)~(0x80u >> ((uint16_t)d0 & 7u))));
}

/* ROM 0x179276 */
static void scr_179276(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)rd16(m, a1 + 6u);
    if (d0 >= 0)
        Sound_PlayCmd(m, Sound_PreCmdHook(m, d0));
    d0 = (int16_t)rd16(m, a1 + 8u);
    if (d0 >= 0)
        Sound_PlayCmd(m, Sound_PreCmdHook(m, d0));
}

/* ROM 0x1792B0 */
static void scr_1792B0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)rd16(m, a1 + 6u);
    if (d0 >= 0)
        Sound_PlayCmdForced(m, Sound_PreCmdHook(m, d0));
    d0 = (int16_t)rd16(m, a1 + 8u);
    if (d0 >= 0)
        Sound_PlayCmdForced(m, Sound_PreCmdHook(m, d0));
}

/* ROM 0x179312 */
static void scr_179312(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    Obj_Spawn(m, obj, 0x0017933Au, 3u, 2u, rd32(m, a1 + 6u));
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & (uint8_t)~0x04u));
}

/* ROM 0x1793F6: load the heading (+0x42) and the two bytes at +0x5C/+0x5E from
 * the entry's words +0x06/+0x08/+0x0A. Plain moves, no side effect beyond
 * those three fields. */
static void scr_1793F6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
    wr16(m, obj + 0x5Cu, rd16(m, a1 + 8u));
    wr16(m, obj + 0x5Eu, rd16(m, a1 + 0x0Au));
}

/* ROM 0x18F5B2 */
static void scr_18F5B2(const f3mem_t *m, uint32_t obj)
{
    Obj_Spawn(m, obj, 0x001915AEu, 9u, 2u, 0u);
}

/* ROM 0x178B7A: aim at the record's point: +0x4C/+0x4E = point (+6/+8 of
 * the record at +0x44); with a positive step count +0x48, back off by one
 * step of (point - position) / count; then +0x48 = 0. */
static void scr_178B7A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)rd16(m, a1 + 6u);
    int16_t d1 = (int16_t)rd16(m, a1 + 8u);
    wr16(m, obj + 0x4Cu, (uint16_t)d0);
    wr16(m, obj + 0x4Eu, (uint16_t)d1);
    int32_t x = (int16_t)(uint16_t)(d0 - (int16_t)rd16(m, obj + 0x50u));   /* sub.w ; ext.l */
    int32_t y = (int16_t)(uint16_t)(d1 - (int16_t)rd16(m, obj + 0x54u));
    int16_t n = (int16_t)rd16(m, obj + 0x48u);
    if (n > 0) {                                        /* ble 0x178BAE */
        wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - (uint16_t)(x / n)));  /* divs.w */
        wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, obj + 0x4Eu) - (uint16_t)(y / n)));
    }
    wr16(m, obj + 0x48u, 0u);
}

static int script10_call(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x00179074u: scr_179074(m, obj);              return 0;
    case 0x00178AC6u: scr_178AC6(m, obj);              return 0;
    case 0x00178B0Eu: scr_178B0E(m, obj);              return 0;
    case 0x00178B20u: Obj_Script10_SetHeading(m, obj); return 0;
    case 0x00178B4Au: sub_178B4A(m, obj);              return 0;
    case 0x00178B7Au: scr_178B7A(m, obj);              return 0;
    case 0x00178C06u: sub_178C06(m, obj);              return 0;
    case 0x00178E4Au: Obj_Script10_SteerTo(m, obj);    return 0;
    case 0x00178CCEu: sub_178CCE(m, obj);              return 0;
    case 0x00178ECCu: scr_178ECC(m, obj);              return 0;
    case 0x0017921Au:                                  return 0; /* ROM 0x17921A: rts */
    case 0x00179222u: scr_179222(m, obj);              return 0;
    case 0x0017924Cu: scr_17924C(m, obj);              return 0;
    case 0x00179276u: scr_179276(m, obj);              return 0;
    case 0x001792B0u: scr_1792B0(m, obj);              return 0;
    case 0x00179312u: scr_179312(m, obj);              return 0;
    case 0x001793F6u: scr_1793F6(m, obj);              return 0;
    case 0x0018F5B2u: scr_18F5B2(m, obj);              return 0;
    default:          return -1;
    }
}

/* The command walk of 0x178982 (10-byte records) and 0x178A2C (12-byte
 * records): {fn.l, count.w, next.l, ...}. */
static int script_walk(const f3mem_t *m, uint32_t obj, uint32_t stride)
{
    uint32_t a1;

    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);

        if (fn == 0u)
            goto loop_marker;
        if (script10_call(m, obj, fn) < 0)
            return -1;

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + stride;
            if ((int32_t)rd32(m, a1) > 0)
                goto store;
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + stride;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }

loop_marker: {
        int16_t c = (int16_t)rd16(m, obj + 0x4Au);
        if (c >= 0) {
            if (c == 0)
                wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
            c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
            wr16(m, obj + 0x4Au, (uint16_t)c);
            if (c == 0) {
                a1 += stride;
                goto store;
            }
        }
        a1 = rd32(m, a1 + 6u);
    }

store:
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    return 0;
}

/* ROM 0x178982 */
int Obj_Script10(const f3mem_t *m, uint32_t obj)
{
    return script_walk(m, obj, 0x0Au);
}

/* ROM 0x178A2C: the same walk over 12-byte records (tail of 0x1789FC) */
int Obj_Script12(const f3mem_t *m, uint32_t obj)
{
    return script_walk(m, obj, 0x0Cu);
}
