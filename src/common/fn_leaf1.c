/* Small leaf routines called through jsr/bsr, register-level: each takes the
 * full 68k register file (f3regs_t) and leaves it as the ROM does at rts.
 * Drafted by the local Strata model from the disassembly, corrected and
 * verified bit-exact against MAME captures in tests/test_fn.c. */
#include "fn_leaf1.h"
#include "f3mem.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

void Leaf_16AB58(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x16AB58: Store low byte of D0 to address in A0 */
    wr8(m, r->a[0], r->d[0] & 0xFFu);
}

void Leaf_16AB5C(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x16AB5C: Store D0 then D1 to A0, post-increment A0 by 8 */
    wr32(m, r->a[0], r->d[0]);
    r->a[0] += 4u;
    wr32(m, r->a[0], r->d[1]);
    r->a[0] += 4u;
}

void Leaf_16AB6A(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x16AB6A: Clear word at A5+0x4294 */
    wr16(m, F3_A5 + 0x4294u, 0u);
}

void Leaf_16AB62(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x16AB62: Set word at A5+0x4294 to 0xFFFF */
    wr16(m, F3_A5 + 0x4294u, 0xFFFFu);
}

/* ROM 0x1766C2: OR D1.w into bits 15..14 of the 0xE8 words at A0 (bits
 * 13..0 kept); D7.w ends at 0xFFFF, D0.w holds the last word written. */
static void loop_1766C2(const f3mem_t *m, f3regs_t *r)
{
    r->d[7] = setw(r->d[7], 0x00E7u);
    for (;;) {
        uint32_t p = r->a[0] + (uint32_t)(int32_t)(int16_t)r->d[7] * 2u;
        uint16_t v = (uint16_t)((rd16(m, p) & 0x3FFFu) | (uint16_t)r->d[1]);
        r->d[0] = setw(r->d[0], v);
        wr16(m, p, v);
        uint16_t c = (uint16_t)(r->d[7] - 1u);              /* dbra */
        r->d[7] = setw(r->d[7], c);
        if (c == 0xFFFFu)
            break;
    }
}

/* ROM 0x1766B6: line-table priority bits = 0x4000 (bra 0x1766C2) */
void Leaf_1766B6(const f3mem_t *m, f3regs_t *r)
{
    r->d[1] = setw(r->d[1], 0x4000u);
    loop_1766C2(m, r);
}

void Leaf_0CEC40(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x0CEC40: Clear word at A5+0x1D5E, copy from A5-0x1456 to A5+0x1D60 */
    wr16(m, F3_A5 + 0x1D5Eu, 0u);
    wr16(m, F3_A5 + 0x1D60u, rd16(m, F3_A5 - 0x1456u));
}

/* ROM 0x1766DA: Clear bit 13 of the A0 words from row D7.w down to row D6.w
 * (do { ... ; D7.w-- } while D6.w <= D7.w, signed) */
void Leaf_1766DA(const f3mem_t *m, f3regs_t *r)
{
    do {
        uint32_t p = r->a[0] + (uint32_t)(int32_t)(int16_t)r->d[7] * 2u;
        uint16_t v = rd16(m, p);
        v &= (uint16_t)~0x2000u;                                    /* bclr #13 on D1 */
        r->d[1] = setw(r->d[1], v);
        wr16(m, p, v);
        r->d[7] = setw(r->d[7], (uint16_t)(r->d[7] - 1u));
    } while ((int16_t)r->d[6] <= (int16_t)r->d[7]);        /* cmp.w d7,d6 ; ble */
}

/* ROM 0x1766EE: Set bit 13 of the A0 words from row D7.w down to row D6.w
 * (do { ... ; D7.w-- } while D6.w <= D7.w, signed) */
void Leaf_1766EE(const f3mem_t *m, f3regs_t *r)
{
    do {
        uint32_t p = r->a[0] + (uint32_t)(int32_t)(int16_t)r->d[7] * 2u;
        uint16_t v = rd16(m, p);
        v |= 0x2000u;                                    /* bset #13 on D1 */
        r->d[1] = setw(r->d[1], v);
        wr16(m, p, v);
        r->d[7] = setw(r->d[7], (uint16_t)(r->d[7] - 1u));
    } while ((int16_t)r->d[6] <= (int16_t)r->d[7]);        /* cmp.w d7,d6 ; ble */
}

/* ROM 0x13E9B0: D0 = ((D0.w + 1) * (A0[+6] - D1.w)) << 9, then add A0[+2]
 * to its high word. D1.w is left as A0[+6] - D1.w. */
void Leaf_13E9B0(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = setw(r->d[0], (uint16_t)(r->d[0] + 1u));
    r->d[1] = setw(r->d[1], (uint16_t)(0u - r->d[1]));
    r->d[1] = setw(r->d[1], (uint16_t)(r->d[1] + rd16(m, r->a[0] + 6u)));
    r->d[0] = (uint32_t)((int32_t)(int16_t)r->d[0] * (int32_t)(int16_t)r->d[1]);  /* muls.w */
    r->d[0] <<= 9;                                      /* lsl.l #1 ; lsl.l #8 */
    r->d[0] += (uint32_t)rd16(m, r->a[0] + 2u) << 16;    /* swap ; add.w ; swap */
}

void Leaf_0C660A(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x0C660A: Save D1/A0, compute indexed address, add D0 to long, restore D1/A0 */
    uint32_t saved_d1 = r->d[1];
    uint32_t saved_a0 = r->a[0];
    r->a[0] = r->d[1];
    uint16_t idx = rd16(m, r->a[0] + 0x22u);
    uint32_t base = 0x41A980u;
    uint32_t addr = base + (idx * 4u);
    uint32_t val = rd32(m, addr) + r->d[0];
    wr32(m, addr, val);
    r->d[1] = saved_d1;
    r->a[0] = saved_a0;
}

/* ROM 0x16A2F6: run-length fill. Records {value.w, count.w} at A1 (count
 * = repeats - 1) until 0xFFFFFFFF; each writes value count+1 times at A0. */
void Leaf_16A2F6(const f3mem_t *m, f3regs_t *r)
{
    for (;;) {
        r->d[0] = rd32(m, r->a[1]);
        r->a[1] += 4u;
        if (r->d[0] == 0xFFFFFFFFu)
            return;
        r->d[7] = setw(r->d[7], (uint16_t)r->d[0]);
        r->d[0] = (r->d[0] << 16) | (r->d[0] >> 16);    /* swap */
        for (;;) {
            wr16(m, r->a[0], (uint16_t)r->d[0]);
            r->a[0] += 2u;
            uint16_t c = (uint16_t)(r->d[7] - 1u);
            r->d[7] = setw(r->d[7], c);
            if (c == 0xFFFFu)
                break;
        }
    }
}

void Leaf_13E994(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x13E994: Compute scaled value using D0, D1 and A0+4, A0 */
    r->d[2] = 0x100u;
    r->d[2] -= (uint16_t)r->d[0];
    r->d[1] = (uint16_t)(-(int16_t)r->d[1]);
    r->d[1] += (int16_t)rd16(m, r->a[0] + 4u);
    r->d[2] = (int32_t)(int16_t)r->d[1] * (int32_t)(r->d[2] & 0xFFFFu);
    r->d[2] <<= 8u;
    r->d[0] = r->d[2];
    r->d[0] = (r->d[0] << 16u) | (r->d[0] >> 16u);
    r->d[0] += (int16_t)rd16(m, r->a[0]);
    r->d[0] = (r->d[0] << 16u) | (r->d[0] >> 16u);
}

/* ROM 0x1766BE: line-table priority bits = 0 (falls into 0x1766C2) */
void Leaf_1766BE(const f3mem_t *m, f3regs_t *r)
{
    r->d[1] = setw(r->d[1], 0x0000u);
    loop_1766C2(m, r);
}

void Leaf_13CAAE(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x13CAAE: Save D0/A0, index PC-relative table with A5+0x32DC, store low byte to A5+0x32DE, restore D0/A0 */
    uint32_t saved_d0 = r->d[0];
    uint32_t saved_a0 = r->a[0];
    r->d[0] = rd16(m, F3_A5 + 0x32DCu);
    r->a[0] = 0x13D600u;
    uint8_t val = rd8(m, r->a[0] + (r->d[0] & 0xFFFFu));
    r->d[0] = val & 0xFFu;
    wr16(m, F3_A5 + 0x32DEu, (uint16_t)r->d[0]);
    r->d[0] = saved_d0;
    r->a[0] = saved_a0;
}
