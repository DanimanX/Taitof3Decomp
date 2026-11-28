#ifndef FN_EEPROM_BLOCK_H
#define FN_EEPROM_BLOCK_H
/* Shared 68k translations for the serial EEPROM and the settings block. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"
#include "../video/sprite.h"

#define EE_LATCH    0x406C52u
#define EE_SR       0x406C50u
#define EE_SETTINGS 0x406C54u

static inline void ee_drive(const f3mem_t *m, uint8_t latch)
{
    wr8(m, EE_LATCH, latch);
    if (m->eeprom) {
        /* The harness has no instruction clock. Advance between bus accesses
         * on the scale of a 16 MHz 68020 instruction sequence. */
        f3_eeprom_advance(m->eeprom, 1000);
        wr8(m, F3_EEP_OUT, latch);
    }
}

static inline uint8_t ee_sample(const f3mem_t *m)
{
    f3_eeprom_advance(m->eeprom, 1000);
    return m->eeprom ? rd8(m, F3_EEP_IN) & 1u : 0;
}

/* 0x0047C0 saves SR, raises IPL while clocking the EEPROM, and restores SR
 * before jumping back through A6. Its callers set D1 with MOVEQ, which
 * clears NZVC and preserves X. Bits 31 and 30 select receive and poll. */
static inline uint32_t ee_xfer(const f3mem_t *m, uint32_t d0, uint16_t d1,
                               uint16_t sr)
{
    uint8_t latch = rd8(m, EE_LATCH);
    wr16(m, EE_SR, sr & 0xFFF0u);
    do {
        latch &= (uint8_t)~(F3_EEP_CLK | F3_EEP_DI);
        if (d0 & (1u << d1)) latch |= F3_EEP_DI;
        ee_drive(m, latch);                        /* 0x47DE / 0x47FA */
        latch |= F3_EEP_CS;
        ee_drive(m, latch);                        /* 0x480C */
        latch |= F3_EEP_CLK;
        ee_drive(m, latch);                        /* 0x4824 */
    } while (d1-- != 0);

    uint32_t reply;
    if (d0 & 0x80000000u) {
        latch &= (uint8_t)~F3_EEP_CLK;
        ee_drive(m, latch);                        /* 0x483E */
        reply = 0;
        for (unsigned i = 0; i < 16; ++i) {
            ee_drive(m, latch | F3_EEP_CLK);       /* 0x4854 */
            ee_drive(m, latch);                    /* 0x4866 */
            reply = (reply << 1) | ee_sample(m);
        }
    } else if (d0 & 0x40000000u) {
        latch &= (uint8_t)~(F3_EEP_CS | F3_EEP_CLK | F3_EEP_DI);
        ee_drive(m, latch);                        /* 0x48A2 */
        latch |= F3_EEP_CS;
        ee_drive(m, latch);                        /* 0x48B4 */
        if (m->eeprom)
            for (unsigned i = 0; i < 10000 && !ee_sample(m); ++i) {}
        reply = d0 << 2;
    } else {
        latch &= (uint8_t)~F3_EEP_CS;
        ee_drive(m, latch);                        /* 0x48DC */
        latch &= (uint8_t)~F3_EEP_CLK;
        for (unsigned i = 0; i < 3; ++i) {
            ee_drive(m, latch);
            ee_drive(m, latch | F3_EEP_CLK);
        }
        latch |= F3_EEP_CLK;
        reply = d0 << 2;
    }
    ee_drive(m, latch & (uint8_t)~F3_EEP_CS);       /* 0x4996 */
    return reply;
}

/* ROM 0x01014E: soft-reset sequence - clear the cache register and the two
 * control bytes, clear the sprite and layer memories, then spin forever at
 * 0x010170. The endless loop cannot be modelled here, so the caller gets -1. */
static inline int reset_loop_1014E(const f3mem_t *m)
{
    wr8(m, 0x004A0004u, 0u);                      /* 0x010158 */
    wr8(m, 0x004A0014u, 0u);                      /* 0x010160 */
    Sprite_ClearAll(m);                           /* 0x010168 -> 0x00117E */
    Gfx_ClearAllLayers(m);                        /* 0x01016C -> 0x005A12 */
    return -1;                                    /* 0x010170 bra.b 0x010170 */
}

/* ROM 0x004DEE: recompute the two settings checksums through 0x004BD2 and
 * write the 0x80-byte block back through 0x0046C0. A non-zero write result
 * clears all layers, draws the reset message and enters 0x01014E. */
static inline int settings_refresh_4DEE(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[2]);                     /* 0x004DEE */
    r->a[2] = EE_SETTINGS;
    push_long(m, r, 0x20u);                       /* 0x004DF6 pea (0x20).w */
    push_long(m, r, EE_SETTINGS);                 /* 0x004DFA move.l A2,-(SP) */
    push_long(m, r, 0x004E00u);                   /* 0x004DFC jsr 0x004BD2 */
    Fn_004BD2(m, r);
    push_long(m, r, 0x60u);                       /* 0x004E00 pea (0x60).w */
    push_long(m, r, EE_SETTINGS + 0x20u);         /* 0x004E04 pea (0x20,A2) */
    push_long(m, r, 0x004E0Cu);                   /* 0x004E08 jsr 0x004BD2 */
    Fn_004BD2(m, r);
    /* NEG.L D1 at the end of the checksum sets X on a nonzero sum. */
    if (m->eeprom)
        r->sr = (uint16_t)((r->sr & ~0x10u) | (r->d[1] ? 0x10u : 0));
    push_long(m, r, EE_SETTINGS);                 /* 0x004E0C move.l A2,-(SP) */
    push_long(m, r, 0x004E14u);                   /* 0x004E0E jsr 0x0046C0 */
    Fn_0046C0(m, r);
    r->a[7] += 0x14u;                             /* 0x004E14 lea (0x14,SP),SP */

    if ((int32_t)r->d[0] != 0) {                  /* 0x004E18 tst.l D0 / beq */
        Gfx_ClearAllLayers(m);                    /* 0x004E1C jsr 0x005A12 */
        push_word(m, r, 8);                       /* 0x004E22 move.w #8,-(SP) */
        push_long(m, r, 0x0061C81Au);             /* 0x004E26 pea */
        push_long(m, r, 0x001A1C36u);             /* 0x004E2C pea */
        push_long(m, r, 0x004E38u);
        r->d[0] = Text_DrawString(m, 0x001A1C36u, 0x0061C81Au, 8u); /* 0x004E32 */
        r->a[7] += 0x0Au;                         /* 0x004E38 lea (0xa,SP),SP */
        return reset_loop_1014E(m);               /* 0x004E3C jsr 0x01014E */
    }
    r->a[2] = rd32(m, r->a[7]);                   /* 0x004E42 movea.l (SP)+,A2 */
    r->a[7] += 8u;                                /* restore A2 and RTS */
    return 0;
}


/* 68020 bit field access on a memory operand: the offset counts from the MSB of
 * the byte at addr (extension bits 11-6), the width is bits 5-0. */
static inline uint32_t ee_bfget(const f3mem_t *m, uint32_t addr, unsigned off,
                                unsigned w)
{
    unsigned need = off + w;
    unsigned top = need <= 16u ? 16u : 32u;
    uint32_t v = need <= 16u ? rd16(m, addr) : rd32(m, addr);
    return (v >> (top - need)) & ((1u << w) - 1u);
}

static inline void ee_bfput(const f3mem_t *m, uint32_t addr, unsigned off,
                            unsigned w, uint32_t val)
{
    unsigned need = off + w;
    unsigned top = need <= 16u ? 16u : 32u;
    uint32_t mask = ((1u << w) - 1u) << (top - need);
    uint32_t v = need <= 16u ? rd16(m, addr) : rd32(m, addr);
    v = (v & ~mask) | ((val << (top - need)) & mask);
    if (need <= 16u)
        wr16(m, addr, (uint16_t)v);
    else
        wr32(m, addr, v);
}

/* ROM 0x004D06: read the settings block and publish it into the RAM globals
 * (0x402237-0x402240, 0x4022A8, 0x409D3A/3C), then rescale the two derived
 * tables (0x004C78, 0x004C12). Pops its own return address and A2.
 * Legacy captures without a device supply the settings block in Work RAM. */
static inline void settings_apply_4D06(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a2 = EE_SETTINGS;
    r->a[7] -= 4u;                              /* 0x004D06 move.l A2,-(SP) */
    wr32(m, r->a[7], r->a[2]);
    r->a[2] = a2;
    if (m->eeprom) {
        push_long(m, r, a2);                    /* 0x004D0E */
        push_long(m, r, 0x004D16u);             /* 0x004D10 jsr 0x00466E */
        Fn_00466E(m, r);
        r->a[7] += 4u;                         /* 0x004D16 */
    }

    r->d[0] = 0u;                               /* 0x004D18 moveq #0,D0 */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | ((rd8(m, a2 + 0xAu) & 0x80u) ? 1u : 0u);
    r->d[0] *= 0xFFFFFFFFu;                     /* 0x004D24 muls.l #-1 */
    wr16(m, 0x004022A8u, (uint16_t)r->d[0]);    /* 0x004D2C */
    r->d[0] = ee_bfget(m, a2 + 0xAu, 11u, 3u);  /* 0x004D32 {11:3} */
    wr8(m, 0x0040223Au, (uint8_t)r->d[0]);      /* 0x004D38 */
    r->d[0] = (rd8(m, a2 + 0x18u) & 0x40u) ? 1u : 0u;   /* 0x004D3E btst #6 */
    wr8(m, 0x0040223Du, (uint8_t)r->d[0]);      /* 0x004D48 */
    r->d[0] = ee_bfget(m, a2 + 0xAu, 14u, 2u);  /* 0x004D4E {14:2} */
    wr8(m, 0x0040223Cu, (uint8_t)r->d[0]);      /* 0x004D54 */
    r->d[0] = (rd8(m, a2 + 0x18u) & 0x80u) ? 1u : 0u;   /* 0x004D5A btst #7 */
    wr8(m, 0x0040223Fu, (uint8_t)r->d[0]);      /* 0x004D64 */
    r->d[0] = (rd8(m, a2 + 0xAu) & 0x20u) ? 1u : 0u;    /* 0x004D6A btst #5 */
    wr8(m, 0x0040223Eu, (uint8_t)r->d[0]);      /* 0x004D74 */
    r->d[0] = ee_bfget(m, a2 + 0xCu, 0u, 4u) << 4;      /* 0x004D7A..0x004D80 */
    r->d[1] = ee_bfget(m, a2 + 0xCu, 4u, 4u);   /* 0x004D82 */
    r->d[0] |= r->d[1];
    wr8(m, 0x00402237u, (uint8_t)r->d[0]);      /* 0x004D8A */
    r->d[0] = ee_bfget(m, a2 + 0xCu, 8u, 4u) << 4;      /* 0x004D90..0x004D96 */
    r->d[1] = ee_bfget(m, a2 + 0xCu, 12u, 4u);  /* 0x004D98 */
    r->d[0] |= r->d[1];
    wr8(m, 0x00402238u, (uint8_t)r->d[0]);      /* 0x004DA0 */
    r->d[0] = rd8(m, a2 + 0xEu);                /* 0x004DA6 */
    wr16(m, 0x00402240u, (uint16_t)r->d[0]);    /* 0x004DAC */
    r->d[0] = ee_bfget(m, a2 + 0x18u, 2u, 3u);  /* 0x004DB2 {2:3} */
    wr16(m, 0x00409D3Au, (uint16_t)r->d[0]);    /* 0x004DB8 */
    r->d[0] = ee_bfget(m, a2 + 0x18u, 5u, 3u);  /* 0x004DBE {5:3} */
    wr16(m, 0x00409D3Cu, (uint16_t)r->d[0]);    /* 0x004DC4 */

    r->a[7] -= 4u;                              /* 0x004DCA jsr 0x004C78 */
    wr32(m, r->a[7], 0x004DCEu);
    Fn_004C78(m, r);
    r->a[7] -= 4u;                              /* 0x004DCE jsr 0x004C12 */
    wr32(m, r->a[7], 0x004DD2u);
    Fn_004C12(m, r);
    if (m->eeprom)
        r->sr &= (uint16_t)~0x10u;              /* 0x004C38 subq.l #5,D0 (5 - 5) */

    if ((rd8(m, a2 + 0xAu) & 0x40u) != 0u)      /* 0x004DD2 btst #6 */
        wr8(m, 0x0040223Bu, 0u);                /* 0x004DE4 clr.b */
    else
        wr8(m, 0x0040223Bu, 1u);                /* 0x004DDA move.b #1 */
    r->a[2] = rd32(m, r->a[7]);                 /* 0x004DEA movea.l (SP)+,A2 */
    r->a[7] += 4u;
    r->a[7] += 4u;                              /* rts */
}

#endif
