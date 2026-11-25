#include "obj_script10.h"
#include "obj_shared.h"
#include "obj_shared_ext.h"

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

/* ROM 0x178CCE: hold the heading and step the position along it by arg0.
 *
 *   (0x20,A0) := (0x20,A0) + arg0          ; signed byte add
 *   d0w = (0x42,A0) + 0x20 ; >>6          ; magnitude
 *   idx = ((0x20,A0) >> 8) & 0xFF         ; bfextu -- NOT the full word
 *   (0x50,A0) += d0 * 0x27E40[idx*2 + 0x80]   (sine)
 *   (0x54,A0) += d0 * 0x27E40[idx*2]          (cosine)
 * Same lookup 0x178C06 uses; only the heading setter differs. */
static void sub_178CCE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + rd8(m, a1 + 6u)));

    uint16_t mag = (uint16_t)((uint32_t)(int32_t)(int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6);
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint32_t base = 0x0027E40u + (uint32_t)idx * 2u;
    int32_t s = (int32_t)(int16_t)rd16(m, base);
    int32_t c = (int32_t)(int16_t)rd16(m, base + 0x80u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)mag * c));
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)((int32_t)mag * s));
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

static int script10_call(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x00179074u: scr_179074(m, obj);              return 0;
    case 0x00178B20u: Obj_Script10_SetHeading(m, obj); return 0;
    case 0x00178B4Au: sub_178B4A(m, obj);              return 0;
    case 0x00178C06u: sub_178C06(m, obj);              return 0;
    case 0x00178E4Au: Obj_Script10_SteerTo(m, obj);    return 0;
    case 0x00178CCEu: sub_178CCE(m, obj);              return 0;
    case 0x00178ECCu: scr_178ECC(m, obj);              return 0;
    default:          return -1;
    }
}

/* ROM 0x178982 */
int Obj_Script10(const f3mem_t *m, uint32_t obj)
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
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0)
                goto store;
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
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
                a1 += 0x0Au;
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
