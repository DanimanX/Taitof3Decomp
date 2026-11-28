/* Darius Gaiden (Taito F3) — small helpers shared by object tick routines. */
#include "obj_common.h"
#include "obj_pool.h"

/* ROM 0x0CEE74 */
void Obj_StartDying(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x000CEE7Cu);
}

/* ROM 0x0CF85A */
void Obj_FlagOffscreen(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if (!(flags & 0x10u))
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (x < -0x40 || x > 0x17F || y < -0x40 || y > 0x127)
        wr8(m, obj + 0x07u, (uint8_t)(flags | 0x08u));
}

/* ROM 0x0D0DCC */
void Coll_AddHazard(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, F3_A5 + 0x906u);
    wr32(m, p, obj);
    wr32(m, F3_A5 + 0x906u, p + 4u);
}

/* ROM 0x0CBD52 */
void Obj_Animate(const f3mem_t *m, uint32_t obj, uint32_t anim_tbl)
{
    for (;;) {
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        uint32_t entry = anim_tbl + (uint32_t)(int32_t)off;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0);
            continue;
        }
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0);
        wr16(m, obj + 0x3Au, (uint16_t)((uint16_t)off + 6u));
    }
}

/* ROM 0x0CBD24: one step of the {dur.w, sprdef.l} table at A1. The index in
 * +0x3A is a byte offset into the table; +0x3E is the frame timer.
 *
 *   0CBD24  move.w (0x3a,A0),D0w
 *   0CBD28  tst.w (0x0,A1,D0w*0x1)
 *   0CBD2C  ble.w 0x000cbd50
 *   0CBD30  addq.w #0x1,(0x3e,A0)
 *   0CBD34  move.w (0x3e,A0),D1w
 *   0CBD38  cmp.w (0x0,A1,D0w*0x1),D1w
 *   0CBD3C  ble.w 0x000cbd4a
 *   0CBD40  clr.w (0x3e,A0)
 *   0CBD44  addq.w #0x6,(0x3a,A0)
 *   0CBD48  bra.b 0x000cbd24
 *   0CBD4A  move.l (0x2,A1,D0w*0x1),(0x10,A0)
 *   0CBD50  rts
 *
 * The routine ends in `rts` and keeps nothing in D0 that a caller depends on:
 * D0w still holds the offset loaded at 0x0CBD24, i.e. the offset whose table
 * entry made `tst.w (0,A1,D0w)` end the loop. A caller that needs it re-reads
 * obj+0x3A before the call, exactly as the ROM leaves D0. A non-positive
 * duration (including -1, which only `ble` treats as end-of-script) ends the
 * step without touching the object beyond the D0 table probe.
 *
 * D1w is not incidental: 0x0CBD34 `move.w (0x3e,A0),D1w` leaves the frame
 * timer in the register, and the caller keeps it even when 0x0CBD40 has
 * already cleared the field again (fnk_0C06E0 pair 6: the object's +0x3A ran
 * from 0xC to 0x12, +0x3E was clobbered back to 0, and the ROM still returns
 * D1 = 2). Obj_AnimFrameStepD1 reports that word; Obj_AnimFrameStep is the
 * plain-RAM form the other callers use. */
int Obj_AnimFrameStepD1(const f3mem_t *m, uint32_t obj, uint32_t anim_tbl, uint16_t *d1w)
{
    int set = 0;
    for (;;) {
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);          /* 0x0CBD24 */
        uint32_t entry = anim_tbl + (uint32_t)(int32_t)off;
        int16_t dur = (int16_t)rd16(m, entry);                /* 0x0CBD28 */
        if (dur <= 0)                                         /* 0x0CBD2C -> 0x0CBD50 */
            return set;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x0CBD30 */
        wr16(m, obj + 0x3Eu, cnt);
        *d1w = cnt;                                           /* 0x0CBD34 move.w */
        set = 1;
        if ((int16_t)cnt <= dur) {                            /* 0x0CBD3C */
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));        /* 0x0CBD4A */
            return 1;
        }
        wr16(m, obj + 0x3Eu, 0);                              /* 0x0CBD40 */
        wr16(m, obj + 0x3Au, (uint16_t)((uint16_t)off + 6u)); /* 0x0CBD44 */
    }
}

void Obj_AnimFrameStep(const f3mem_t *m, uint32_t obj, uint32_t anim_tbl)
{
    uint16_t d1w;
    (void)Obj_AnimFrameStepD1(m, obj, anim_tbl, &d1w);
}

/* ROM 0x0CBD86: reset the per-frame object fields. Byte-for-byte from
 * work/maincpu.bin at 0xCBD86:
 *   4268 001c    clr.w (0x1c,A0)
 *   4268 001e    clr.w (0x1e,A0)
 *   4268 0034    clr.w (0x34,A0)
 *   4268 003a    clr.w (0x3a,A0)
 *   4268 003e    clr.w (0x3e,A0)
 *   42a8 0024    clr.l (0x24,A0)
 *   42a8 0028    clr.l (0x28,A0)
 *   4e75         rts
 */
void Obj_ResetFields(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x0D0690: advance the object script (obj+0x48) once its frame timer
 * (obj+0x4C) runs out. */
void Obj_StepScript(const f3mem_t *m, uint32_t obj)
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

/* ROM 0x0CEE7C / 0x0CEE84 / 0x0CEE8C: the dying chain started by
 * Obj_StartDying waits two frames, then frees the object. */
int Obj_Dying1_Tick(const f3mem_t *m, uint32_t obj) { wr32(m, obj, 0x000CEE84u); return 0; }
int Obj_Dying2_Tick(const f3mem_t *m, uint32_t obj) { wr32(m, obj, 0x000CEE8Cu); return 0; }
int Obj_Dying3_Tick(const f3mem_t *m, uint32_t obj) { Obj_Free(m, obj); return 0; }

/* ROM 0x17BDA2: {dx.b, dy.b, radius.b, angle.b} at +0x0C. The signed offset
 * bytes go to +0x16/+0x1A, the angle byte (the 0x27E40*4 index) to +0x20, and
 * the radial step (table entry * radius*4) >> 16 is added to the offsets. The
 * position is then the PARENT's +0x50/+0x54 plus those offsets -- confirmed by
 * the 0x17ED40 capture (parent 213/137 + the d=0 step 0xFFF8/0x0005 -> 205/142,
 * not the object's own stale 1/-2). 0x17EA0A's call reaches the same entry with
 * its own +0x0C record. Verifies the unpacked-table decode through *shift. */
int Obj_PolarPlace(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Cu);
    uint16_t d1 = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Du);
    uint16_t d2 = rd8(m, obj + 0x0Eu);
    uint16_t d3 = rd8(m, obj + 0x0Fu);
    uint16_t px = rd16(m, a4 + 0x50u);
    uint16_t py = rd16(m, a4 + 0x54u);

    wr16(m, obj + 0x16u, d0);
    wr16(m, obj + 0x1Au, d1);
    wr16(m, obj + 0x20u, 0u);
    wr8(m, obj + 0x20u, (uint8_t)d3);

    {
        /* ROM 0x17BDCA: lea (0x27E40).l,A1; move.w (0x80,A1,D3w*2),D0w (cos) ;
         * move.w (0x0,A1,D3w*2),D1w (sin) ; lsl.w #2 the radius; muls.w both;
         * swap: the high halves land in the offset words. */
        int32_t c = (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d3 * 2u);
        int32_t s = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)d3 * 2u);
        int32_t r = (int32_t)(int16_t)(uint16_t)(d2 << 2);
        uint32_t p0 = (uint32_t)(c * r);
        uint32_t p1 = (uint32_t)(s * r);
        wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)(p0 >> 16)));
        wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)(p1 >> 16)));
    }

    {
        uint16_t x = (uint16_t)(px + rd16(m, obj + 0x16u));
        uint16_t y = (uint16_t)(py + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, x);
        wr16(m, obj + 0x54u, y);
        wr16(m, obj + 0x14u, x);
        wr16(m, obj + 0x18u, y);
    }
    return 0;
}

/* ROM 0x11B31C: load +0x50/+0x52 words, clear +0x54/+0x56, sign-extend 4 bytes to +0x2C/+0x30/+0x2E/+0x32 */
uint32_t Obj_InitPosBox(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x50u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x52u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x54u, 0u);
    wr16(m, obj + 0x56u, 0u);
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1 += 1u;
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1 += 1u;
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1 += 1u;
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1 += 1u;
    return a1;
}

/* ROM 0x17BD68: sign-extend obj+0x0C/+0x0D bytes to +0x16/+0x1A and offset parent (+0x08) +0x50/+0x54 */
void Obj_InheritOffsetPos(const f3mem_t *m, uint32_t obj)
{
    uint16_t ox = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Cu);
    uint16_t oy = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Du);
    uint32_t parent = rd32(m, obj + 0x08u);
    uint16_t px = (uint16_t)(rd16(m, parent + 0x50u) + ox);
    uint16_t py = (uint16_t)(rd16(m, parent + 0x54u) + oy);
    wr16(m, obj + 0x16u, ox);
    wr16(m, obj + 0x1Au, oy);
    wr16(m, obj + 0x50u, px);
    wr16(m, obj + 0x54u, py);
    wr16(m, obj + 0x14u, px);
    wr16(m, obj + 0x18u, py);
}

/* ROM 0x16AB2C: store (D0, D1) long pair to both A0 and A1 */
void Mem_WritePair64(const f3mem_t *m, uint32_t a0, uint32_t a1, uint32_t d0, uint32_t d1)
{
    wr32(m, a0, d0);
    wr32(m, a0 + 4u, d1);
    wr32(m, a1, d0);
    wr32(m, a1 + 4u, d1);
}

/* ROM 0x16A19C: copy D0w+1 longs from A0 to A1 */
void Mem_CopyLongs(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d0w)
{
    uint32_t count = (uint32_t)d0w + 1u;
    for (uint32_t i = 0; i < count; i++) {
        wr32(m, a1, rd32(m, a0));
        a0 += 4u;
        a1 += 4u;
    }
}

/* ROM 0x0054AC: clear (rows=A0[0], cols=A0[1]) longs at A1 with 0x80-byte row stride */
void Video_ClearRect(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t rows = rd8(m, a0);
    uint16_t cols = rd8(m, a0 + 1u);
    int16_t stride = (int16_t)(0x80u - (cols << 2));
    for (uint16_t r = 0; r < rows; r++) {
        for (uint16_t c = 0; c < cols; c++) {
            wr32(m, a1, 0u);
            a1 += 4u;
        }
        a1 = (uint32_t)((int32_t)a1 + (int32_t)stride);
    }
}

/* ROM 0x0017D6: populate RGB interpolation ramp entries at A5-0x7F50 + d6
 *
 * The step is `divs.l D6,D3`. On the 68k that is the LONG form
 * (mode 6, size 2) with a mandatory padding 0 word after the extension, so
 * the dividend is the pair (D6h:D6l) = D6:D7 rather than D3:D7, and the
 * quotient is left in D3. That matters for the callers that pre-load the
 * step count into D7 and rely on the high half of the dividend being 0. */
static void pal_ramp_fill(const f3mem_t *m, uint32_t a0, uint32_t a1,
                          uint32_t d1, uint32_t d2, uint32_t d6)
{
    uint32_t a2 = (F3_A5 - 0x7F50u) + d6;
    int32_t div = (int32_t)d1;
    uint16_t d7 = (uint16_t)((uint16_t)d2 - 1u);
    for (;;) {
        uint32_t c0 = rd32(m, a0);
        a0 += 4u;
        uint32_t c1 = rd32(m, a1);
        a1 += 4u;
        for (int k = 0; k < 3; k++) {
            unsigned sh = 16u - 8u * (unsigned)k;
            uint32_t s = ((c0 >> sh) & 0xFFu) << 8;
            uint32_t e = ((c1 >> sh) & 0xFFu) << 8;
            wr16(m, a2 + 2u * (uint32_t)k, (uint16_t)s);
            int32_t step = ((int32_t)e - (int32_t)s) / div; /* divsl.l D6,D3:D3 */
            wr16(m, a2 + 6u + 2u * (uint32_t)k, (uint16_t)(uint32_t)step);
        }
        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x001792 / 0x00183C: claim a palette-ramp slot at A5-0x6650 and build
 * ramp. Returns the claimed slot entry (what 0x00183C hands back in A0/D0) or
 * 0 when no slot could be claimed -- which is exactly the `sub.l A0,A0` the
 * abort path at 0x17C8 performs. */
uint32_t Pal_QueueRamp(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    uint16_t d7 = 0x000Fu;
    for (;;) {
        if (rd16(m, a3) == 0u) {
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return 0u;
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return 0u;
            wr16(m, a3 + 4u, (uint16_t)d0);
            pal_ramp_fill(m, a0, a1, d1, d2, d6);
            return a3;
        }
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            return 0u;
    }
}

/* ROM 0x19900: init the player-relative main object. Seeds the enemy stats and
 * the behaviour script from the records it builds on its stack (D0 = XY-like
 * pair, D1 = the two long args, both passed through from the spawner), writes
 * the position words, then installs 0x199C6. Not yet decompiled: the caller
 * (0x1057C) runs this before it returns the new object. */
void Obj_Init19900(const f3mem_t *m, uint32_t obj, uint32_t d0, uint32_t d1)
{
    (void)m; (void)obj; (void)d0; (void)d1;
}
