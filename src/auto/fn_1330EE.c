/* ROM 0x1330EE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

#define F3_A5 0x408000u

static uint8_t bfset_byte(const f3mem_t *m, uint32_t addr, uint8_t offset, uint8_t width) {
    uint8_t byte = rd8(m, addr);
    uint8_t bit = 7 - (offset & 7);
    uint8_t z = !(byte & (1u << bit));
    byte |= (1u << bit);
    wr8(m, addr, byte);
    (void)width;
    return z;
}

int Fn_1330EE(const f3mem_t *m, f3regs_t *r) {
    uint16_t v;
    uint8_t z;

    /* ROM 0x1330EE: subq.w #1, (0x6,A0) */
    v = rd16(m, r->a[0] + 6);
    v--;
    wr16(m, r->a[0] + 6, v);
    if (v > 0) {
        /* ROM 0x1330E6: move.l #0x1330ee,(A0) */
        wr32(m, r->a[0], 0x1330EE);
        /* ROM 0x1330EC: rts */
        return 0;
    }

    /* ROM 0x1330F4: cmpi.w #0x320,(0x30da,A5) */
    v = rd16(m, F3_A5 + 0x30DA);
    if (v <= 0x320) {
        goto branch_133198;
    }

    /* ROM 0x1330FE: move.b #2,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2, 2);
    /* ROM 0x133104: bfset (0x30ee,A5){0xc:0x1} */
    z = bfset_byte(m, F3_A5 + 0x30EE, 12, 1);
    /* ROM 0x13310A: move.w #0xb4,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0xB4);
    /* ROM 0x133110: beq.b 0x00133120 */
    if (z) {
        goto branch_133120;
    }
    /* ROM 0x133112: move.l #0x13311a,(A0) */
    wr32(m, r->a[0], 0x13311A);
    /* ROM 0x133118: rts */
    return 0;

branch_133120:
    /* ROM 0x133120: movea.l (0x1d28,A5),A1 */
    r->a[1] = rd32(m, F3_A5 + 0x1D28);
    /* ROM 0x133124: move.w (0x14,A1),(0x30d2,A5) */
    wr16(m, F3_A5 + 0x30D2, rd16(m, r->a[1] + 14));
    /* ROM 0x13312A: addi.w #0x14,(0x30d2,A5) */
    v = rd16(m, F3_A5 + 0x30D2);
    v += 0x14;
    wr16(m, F3_A5 + 0x30D2, v);
    /* ROM 0x133130: move.b #3,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2, 3);
    /* ROM 0x133136: bfset (0x30ee,A5){0xc:0x1} */
    z = bfset_byte(m, F3_A5 + 0x30EE, 12, 1);
    /* ROM 0x13313C: move.w #0xb4,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0xB4);
    /* ROM 0x133142: beq.b 0x00133152 */
    if (z) {
        goto branch_133152;
    }
    /* ROM 0x133144: move.l #0x13314c,(A0) */
    wr32(m, r->a[0], 0x13314C);
    /* ROM 0x13314A: rts */
    return 0;

branch_133152:
    /* ROM 0x133152: cmpi.w #0x320,(0x30da,A5) */
    v = rd16(m, F3_A5 + 0x30DA);
    if (v <= 0x320) {
        goto branch_133198;
    }
    /* ROM 0x13315C: move.b #4,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2, 4);
    /* ROM 0x133162: bfset (0x30ee,A5){0xc:0x1} */
    z = bfset_byte(m, F3_A5 + 0x30EE, 12, 1);
    /* ROM 0x133168: move.w #0x168,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x168);
    /* ROM 0x13316E: beq.b 0x0013317e */
    if (z) {
        goto branch_13317e;
    }
    /* ROM 0x133170: move.l #0x133178,(A0) */
    wr32(m, r->a[0], 0x133178);
    /* ROM 0x133176: rts */
    return 0;

branch_13317e:
    /* ROM 0x13317E: move.b #0,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2, 0);
    /* ROM 0x133184: bfset (0x30ee,A5){0xc:0x1} */
    z = bfset_byte(m, F3_A5 + 0x30EE, 12, 1);
    /* ROM 0x13318A: cmpi.w #0x320,(0x30da,A5) */
    v = rd16(m, F3_A5 + 0x30DA);
    if (v <= 0x320) {
        goto branch_133198;
    }
    /* ROM 0x133194: bra.w 0x0013304c */
    goto branch_13304c;

branch_13304c:
    /* ROM 0x13304C: move.w #0x3c,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x3C);
    /* ROM 0x133052: move.l #0x13305a,(A0) */
    wr32(m, r->a[0], 0x13305A);
    /* ROM 0x133058: rts */
    return 0;

branch_133198:
    /* ROM 0x133198: move.w #0x13,-(SP) */
    r->a[7] -= 2;
    wr16(m, r->a[7], 0x13);
    /* ROM 0x13319C: jsr 0x0013f238.l */
    Sound_PlayHooked(m, 0x13);
    /* ROM 0x1331A2: jsr 0x000039d8.l */
    Sound_PlayCmd(m, 0x13);
    /* ROM 0x1331A8: lea (0x2,SP),SP */
    r->a[7] += 2;
    /* ROM 0x1331AC: bfset (0x30ee,A5){0xb:0x1} */
    bfset_byte(m, F3_A5 + 0x30EE, 11, 1);
    /* ROM 0x1331B2: move.b #7,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2, 7);
    /* ROM 0x1331B8: bfset (0x30ee,A5){0xc:0x1} */
    bfset_byte(m, F3_A5 + 0x30EE, 12, 1);
    /* ROM 0x1331BE: move.l #0x1331c6,(A0) */
    wr32(m, r->a[0], 0x1331C6);
    /* ROM 0x1331C4: rts */
    return 0;
}
