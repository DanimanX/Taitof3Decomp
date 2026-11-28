/* ROM 0x1321DE: one frame of the lift "floor indicator" script for the object
 * at A5+0x34E8. The word at A5+0x34E8 is a per-frame delay: while it is
 * positive only the shared tail runs, which pushes a 16-byte OSD sprite record
 * (tile from 0x132374, long from 0x13236C, both indexed by the step word at
 * A5+0x34EA) into the list at A5+0x1D00 while the count at A5+0x1CF0 lasts.
 * When the delay expires the step word is reloaded from the 0x132336 record
 * table {A5+0x34EA flag, A5+0x34E8 delay}: a negative flag returns, a zero flag
 * paints the two indicator rows 0x62B030/0x62B630 with 0x1002/0x300C, any
 * other flag paints them with 0x3002/0x100C, plays sound 0x0E, seeds a particle
 * through 0x132392, then both paths advance the step and reload the delay from
 * the 0x132338 half of the same table. Writes are mirrored backwards on the
 * second interlace field (word at A5-0x7EC2). */
#include "ea_auto.h"

/* ROM 0x03706: derive the "volume" byte A5-0x7E18 from the master volume
 * A5-0x7F2A and the ramp state A5-0x7E1C/A5-0x7E1A. Saves D0. */
static void snd_vol(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t d0 = rd16(m, a5 - 0x7F2Au);           /* 0x0370C */
    uint16_t vol = rd16(m, a5 - 0x7E1Au);

    if (rd16(m, a5 - 0x7E1Cu) != 0u) {             /* 0x03710 tst.w ; beq */
        d0 = (uint16_t)(d0 - 0x0020u);             /* 0x03716 */
        if ((int16_t)d0 < 0) {                     /* 0x0371A bge */
            int16_t t = (int16_t)(d0 * vol);       /* 0x0371C muls.w */
            d0 = (uint16_t)(((uint16_t)(t >> 8)) + vol); /* 0x03720 asr.w #8 ; add.w */
        } else {
            d0 = vol;                              /* 0x0373C */
        }
    } else {
        d0 = (uint16_t)(d0 - 0x0020u);             /* 0x03728 */
        if ((int16_t)d0 > 0) {                     /* 0x0372C ble */
            int16_t t = (int16_t)(d0 * vol);
            d0 = (uint16_t)((uint16_t)(-(uint16_t)(t >> 8)) + vol); /* 0x03732 neg.w */
        } else {
            d0 = vol;
        }
    }
    wr8(m, a5 - 0x7E18u, (uint8_t)(d0 & 0x007Fu)); /* 0x03740/0x03744 */
}

/* One 0x2E8C command block: the first byte of the ROM's leading word is the
 * count, the rest is the payload; 0x2E8C copies count bytes into the DPRAM
 * ring at 0xC00000+D1. */
static void snd_blk(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                          uint8_t count, const uint8_t *bytes)
{
    uint32_t a2 = r->a[5] - 0x7E2Cu;
    unsigned i;

    wr8(m, a2, count);                             /* move.w #$ccNN,(a2)+ */
    for (i = 0; i + 1u < (unsigned)count && i < 8u; i++)
        wr8(m, a2 + 1u + i, bytes[i]);
    push_long(m, r, ret);
    Ea_002E8C(m, r);
}

/* ROM 0x002F3A: queue sound id (the word argument at 4(SP)) into the 1 KiB
 * DPRAM ring at 0xC00000. The record table at A5-0x7EB4 gives the sound
 * record; its first byte selects one of five handlers at 0x2F84 (+0x0A,
 * +0xA0, +0xF6, +0x19A, +0x1B0), which push command blocks through 0x2E8C
 * and write the per-voice shadow bytes at A5-0x7EAC/A5-0x6152. The ring index
 * at 0xC00480 becomes the new index doubled. Returns -1 for a handler this
 * file does not have. */
static int snd_2F3A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t id, off;
    int rc = 0;

    /* 0x002F3A movem.l d0-d2/a0-a2,-(a7), highest register first so that
     * D0 lands at SP+0 for the restore below. */
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);
    if (rd8(m, a5 - 0x7EB0u) != 0u)                /* 0x002F3E tst.b */
        goto restore;
    id = rd16(m, r->a[7] + 0x1Cu);                 /* 0x002F44 move.w 0x1c(a7),d0 */
    if ((int16_t)id < 0)                           /* 0x002F48 bmi */
        goto restore;
    if ((int16_t)id > (int16_t)rd16(m, 0x00008334u)) /* 0x002F4A cmp.w $8334.l */
        goto restore;
    {
        uint32_t a0, a1 = 0x00C00000u;             /* 0x002F54 lea.l $c00000.l,a1 */
        uint16_t d0;
        uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x480u) >> 1); /* 0x002F5A */

        a0 = rd32(m, rd32(m, a5 - 0x7EB4u)
                     + (uint32_t)((int32_t)(int16_t)id * 4)); /* 0x002F60/0x002F64 */
        d0 = (uint16_t)rd8(m, a0);                 /* 0x002F68 moveq/move.b (a0)+ */
        a0 += 1u;
        off = rd16(m, 0x00002F84u + (uint32_t)d0 * 2u); /* 0x002F6C */
        r->a[1] = a1;
        r->a[0] = a0;
        r->d[1] = set_word(r->d[1], d1);
        r->d[0] = set_word(r->d[0], off);

        switch (off) {
        case 0x000Au:                              /* 0x002F8E: voice on */
            d0 = (uint16_t)rd8(m, a0);
            if ((d0 & 0x0080u) == 0u) {            /* 0x002F92 bmi 0x2fd0 */
                uint8_t b[4];
                const uint8_t snd = (uint8_t)rd8(m, a0);
                wr8(m, a5 - 0x7EACu + d0, snd);    /* 0x002F98 */
                b[0] = 0x81u; b[1] = snd;
                snd_blk(m, r, 0x0012FA6Cu, 3u, b);   /* 0x002F9C */
                wr16(m, a5 - 0x7E1Cu, 0u);         /* 0x002FAA */
                wr16(m, a5 - 0x7E1Au, 0u);         /* 0x002FAE */
                wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u)); /* 0x002FB2 */
                snd_vol(m, r);               /* 0x002FB8 bsr $3706 */
                b[0] = 0x86u; b[1] = snd;
                b[2] = rd8(m, a5 - 0x7E18u);
                snd_blk(m, r, 0x0012FCA6u, 4u, b);   /* 0x002FBC */
            } else {                               /* 0x002FD0: voice off */
                uint8_t b[4];
                const uint8_t snd = (uint8_t)rd8(m, a0);
                d0 &= 0x007Fu;                     /* 0x002FD0 andi.b #$7f */
                wr8(m, a5 - 0x7EACu + d0, snd);    /* 0x002FD8 */
                b[0] = 0x81u; b[1] = snd;
                snd_blk(m, r, 0x0012FDE8u, 3u, b);   /* 0x002FDE */
                wr16(m, a5 - 0x7E1Cu, 0u);         /* 0x002FEC */
                wr16(m, a5 - 0x7E1Au, 0u);         /* 0x002FF0 */
                wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u)); /* 0x002FF4 */
                snd_vol(m, r);               /* 0x002FFA */
                b[0] = 0x86u; b[1] = snd;
                b[2] = rd8(m, a5 - 0x7E18u);
                snd_blk(m, r, 0x0013000Cu, 4u, b);   /* 0x002FFE */
                b[1] = (uint8_t)d0;                /* 0x003018 move.b d2,(a2)+ */
                snd_blk(m, r, 0x0013001Eu, 4u, b);   /* 0x003010 */
            }
            break;
        case 0x00A0u:                              /* 0x003024: voice on, cmd 0x88 */
        {
            uint8_t b[4];
            const uint8_t snd = (uint8_t)rd8(m, a0);
            d0 = (uint16_t)(snd & 0x7Fu);          /* 0x003028 */
            wr8(m, a5 - 0x7EACu + d0, snd);        /* 0x003030 */
            b[0] = 0x81u; b[1] = snd;
            snd_blk(m, r, 0x0013030Eu, 3u, b);       /* 0x003034 */
            wr16(m, a5 - 0x7E1Cu, 0u);             /* 0x003042 */
            wr16(m, a5 - 0x7E1Au, 0u);             /* 0x003046 */
            wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u)); /* 0x00304A */
            snd_vol(m, r);                   /* 0x003050 */
            b[0] = 0x86u; b[1] = snd;
            b[2] = rd8(m, a5 - 0x7E18u);
            snd_blk(m, r, 0x0013033Eu, 4u, b);       /* 0x003054 */
            b[0] = 0x88u; b[1] = rd8(m, a0 + 2u);
            snd_blk(m, r, 0x00130340u, 4u, b);       /* 0x003066 */
            break;
        }
        case 0x00F6u:                              /* 0x00307A: note on */
        {
            uint8_t b[6];
            uint32_t tbl2, tbl1;
            d0 = (uint16_t)(rd8(m, a0) - 1u);      /* 0x00307C subi.w #1 */
            d0 = (uint16_t)(d0 << 3);              /* 0x003082 asl.w #3 */
            d0 = (uint16_t)(d0 + rd8(m, a0 + 2u)); /* 0x003084 add.b $2(a0),d0 */
            tbl2 = a5 - 0x6152u + (uint32_t)((int32_t)(int16_t)d0 * 2);
            tbl1 = a5 - 0x6171u + (uint32_t)d0;
            if (rd16(m, tbl2) != 0u) {             /* 0x003094 tst.w */
                uint8_t v = rd8(m, tbl1);
                if (v != 0u) {                     /* 0x00309C beq */
                    b[0] = 0x8Fu; b[1] = rd8(m, a0);
                    b[2] = rd8(m, a0 + 2u); b[3] = v;
                    snd_blk(m, r, 0x0013035Eu, 5u, b); /* 0x0030A4 */
                }
            }
            b[0] = 0x8Du; b[1] = rd8(m, a0);
            b[2] = rd8(m, a0 + 2u); b[3] = rd8(m, a0 + 3u);
            b[4] = rd8(m, a0 + 4u);
            snd_blk(m, r, 0x00130374u, 6u, b);   /* 0x0030B8 */
            wr16(m, a5 - 0x7E1Cu, 0xFFFFu);        /* 0x0030D2 */
            wr16(m, a5 - 0x7E1Au, 0u);             /* 0x0030D8 */
            wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u)); /* 0x0030DC */
            snd_vol(m, r);                   /* 0x0030E2 */
            b[0] = 0x8Eu; b[1] = rd8(m, a0);
            b[2] = rd8(m, a0 + 2u); b[3] = rd8(m, a0 + 5u);
            b[4] = rd8(m, a5 - 0x7E18u);
            snd_blk(m, r, 0x001303A6u, 6u, b);   /* 0x0030E6 */
            wr8(m, tbl1, rd8(m, a0 + 5u));         /* 0x003102 move.b $5(a0),(a2) */
            {
                int16_t v = (int16_t)(int8_t)rd8(m, a0 + 6u); /* 0x00310A ext.w */
                if (v == 0)                        /* 0x003112 bne / 0x003114 */
                    v = 1;
                if (v > 0)                         /* 0x003110 bmi */
                    v = (int16_t)(v * 0x003Cu);    /* 0x003116 muls.w #$3c */
                wr16(m, tbl2, (uint16_t)v);        /* 0x00311A */
            }
            break;
        }
        case 0x019Au:                              /* 0x00311E: raw byte stream */
        {
            uint16_t n = (uint16_t)rd8(m, a0);     /* 0x003120 move.b (a0)+,d0 */
            uint16_t d1v = d1;
            a0 += 1u;
            for (;;) {                             /* 0x003124..0x00312E dbra */
                wr8(m, a1 + d1v, rd8(m, a0));
                a0 += 1u;
                d1v = (uint16_t)((d1v + 1u) & 0x03FFu);
                n = (uint16_t)(n - 1u);
                if ((int16_t)n == -1)
                    break;
            }
            r->a[0] = a0;
            r->d[1] = set_word(r->d[1], d1v);
            break;
        }
        case 0x01B0u:                              /* 0x003134: bare rts */
            break;
        default:
            rc = -1;                               /* handler not decompiled */
            break;
        }

        /* 0x002F78 add.w d1,d1 ; move.w d1,$480(a1). The sound-side DPRAM is
         * 8 bits wide and drops A1, so this one word write also lands at
         * +0x482: the captured image holds the index twice. */
        {
            uint16_t v = (uint16_t)((uint16_t)r->d[1] + (uint16_t)r->d[1]);
            wr16(m, a1 + 0x480u, v);
            wr16(m, a1 + 0x482u, v);
        }
    }
restore:
    r->d[0] = rd32(m, r->a[7] + 0u);               /* 0x002F7E movem.l (a7)+ */
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->d[2] = rd32(m, r->a[7] + 8u);
    r->a[0] = rd32(m, r->a[7] + 12u);
    r->a[1] = rd32(m, r->a[7] + 16u);
    r->a[2] = rd32(m, r->a[7] + 20u);
    r->a[7] += 28u;                                /* the 24 saved + the word arg */
    return rc;
}
/* ROM 0x132392: take the first free 0x20-byte slot of the five at A5-0x4F70
 * (word 0 = free), arm it and seed it from the 0x001338 RNG: +2 = bit 0,
 * +4 = bits 1-0, +6 = 0x150 + 6 bits, +A = 0x50 + 4 bits, +E = 0xFFF00000 +
 * 8*(sign extended word), +12 = 0xFFFC0000 + 4*(sign extended word).
 * Nothing is saved: D0, D1 and A0 stay clobbered (the caller's movem restores
 * them). */
static void part_132392(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a5 = r->a[5];
    uint32_t a0 = a5 - 0x4F70u;                      /* 0x132392 lea.l -$4f70(a5) */
    const uint32_t stride = 0x20u;                   /* 0x132396 moveq #$20,d0 */
    uint16_t d0 = 0x0020u;
    uint16_t d1 = 0x0004u;                           /* 0x132398 move.w #$4,d1 */

    for (;;) {
        if (rd16(m, a0) == 0u)                       /* 0x13239C tst.w (a0) */
            break;
        a0 += stride;                                /* 0x132410 adda.l d0,a0 */
        d1 = (uint16_t)(d1 - 1u);                    /* 0x132412 dbra d1,0x13239C */
        if (d1 == 0xFFFFu) {
            r->d[0] = set_word(r->d[0], 0x0020u);    /* 0x132416 rts: no slot */
            r->d[1] = set_word(r->d[1], 0xFFFFu);
            r->a[0] = a0;
            r->a[7] += 4u;
            return;
        }
    }

    wr16(m, a0, 0x0001u);                            /* 0x1323A2 move.w #$1,(a0) */
    push_long(m, r, 0x1323ACu);                      /* 0x1323A6 jsr $1338 */
    Ea_001338(m, r);
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] & 0x0001u)); /* 0x1323AC */
    wr16(m, a0 + 0x0002u, (uint16_t)r->d[0]);        /* 0x1323B0 */
    push_long(m, r, 0x1323B4u);                      /* 0x1323B4 jsr $1338 */
    Ea_001338(m, r);
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] & 0x0003u)); /* 0x1323BA */
    wr16(m, a0 + 0x0004u, (uint16_t)r->d[0]);        /* 0x1323BE */
    push_long(m, r, 0x1323C2u);                      /* 0x1323C2 jsr $1338 */
    Ea_001338(m, r);
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] & 0x003Fu)); /* 0x1323C8 */
    d0 = (uint16_t)((uint16_t)r->d[0] + 0x0150u);    /* 0x1323CC addi.w #$150 */
    r->d[0] = set_word(r->d[0], d0);
    wr16(m, a0 + 0x0006u, d0);                       /* 0x1323D0 */
    push_long(m, r, 0x1323D4u);                      /* 0x1323D4 jsr $1338 */
    Ea_001338(m, r);
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] & 0x000Fu)); /* 0x1323DA */
    d0 = (uint16_t)((uint16_t)r->d[0] + 0x0050u);    /* 0x1323DE addi.w #$50 */
    r->d[0] = set_word(r->d[0], d0);
    wr16(m, a0 + 0x000Au, d0);                       /* 0x1323E2 */
    push_long(m, r, 0x1323E6u);                      /* 0x1323E6 jsr $1338 */
    Ea_001338(m, r);
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]; /* 0x1323EC ext.l */
    r->d[0] = r->d[0] << 3;                          /* 0x1323EE lsl.l #3 */
    r->d[0] += 0xFFF00000u;                          /* 0x1323F0 addi.l */
    wr32(m, a0 + 0x000Eu, r->d[0]);                  /* 0x1323F6 */
    push_long(m, r, 0x1323FAu);                      /* 0x1323FA jsr $1338 */
    Ea_001338(m, r);
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0]; /* 0x132400 ext.l */
    r->d[0] = r->d[0] << 2;                          /* 0x132402 lsl.l #2 */
    r->d[0] += 0xFFFC0000u;                          /* 0x132404 addi.l */
    wr32(m, a0 + 0x0012u, r->d[0]);                  /* 0x13240A */
    r->a[0] = a0;
    r->d[1] = set_word(r->d[1], d1);
    r->a[7] += 4u;                                   /* 0x13240E rts */
}

/* movem.l d0-d7/a0-a4,-(a7) ... movem.l (a7)+,d0-d7/a0-a4: the three paths
 * of 0x1321DE wrap their work in this pair, so every register but SP survives. */
static void push13(const f3mem_t *m, f3regs_t *r)
{
    /* movem.l d0-d7/a0-a4,-(a7): the highest register goes to the highest
     * address, so D0 ends at SP+0 (as pop13 reads it back). */
    push_long(m, r, r->a[4]);
    push_long(m, r, r->a[3]);
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    for (unsigned i = 8u; i-- > 0u;)
        push_long(m, r, r->d[i]);
}

static void pop13(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];

    for (unsigned i = 0; i < 13u; i++) {
        uint32_t v = rd32(m, sp + 4u * i);
        if (i < 8u)
            r->d[i] = v;
        else
            r->a[i - 8u] = v;
    }
    r->a[7] = sp + 52u;
}

/* One indicator row: forward writes repeat at fwd+0,2,.. (dbra with D0 = 0
 * stores exactly one word) and the second interlace field writes the same word
 * at back-2 through -(a0). 0x132208..0x132230 / 0x13223C..0x132264. */
static void flash_row(const f3mem_t *m, uint32_t a5, uint32_t fwd, uint32_t back,
                      uint16_t val)
{
    if (rd16(m, a5 - 0x7EC2u) == 0u) {               /* tst.w -$7ec2(a5) ; bne */
        wr16(m, fwd, val);                           /* move.w d1,(a0)+ */
    } else {
        wr16(m, back - 2u, val);                     /* move.w d1,-(a0) */
    }
}

int Ea_1321DE(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a5 = r->a[5];
    uint32_t d0 = r->d[0];
    uint32_t a0 = r->a[0];
    uint32_t idx = 0u, fidx = 0u;
    uint16_t w;

    w = (uint16_t)(rd16(m, a5 + 0x34E8u) - 1u);      /* 0x1321DE subq.w #1 */
    wr16(m, a5 + 0x34E8u, w);
    if ((int16_t)w > 0)                              /* 0x1321E2 bgt.w 0x1322FE */
        goto tail;

    d0 = set_word(d0, rd16(m, a5 + 0x34ECu));        /* 0x1321E6 move.w $34ec */
    idx = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;
    w = rd16(m, 0x00132336u + idx * 4u);             /* 0x1321EA $14a(pc,d0.w*4) */
    wr16(m, a5 + 0x34EAu, w);
    if ((int16_t)w < 0)                              /* 0x1321F4 bmi.w 0x132334 */
        goto done;
    if ((int16_t)w == 0) {                           /* 0x1321F8 beq.w 0x132280 */
        /* 0x132280: flag 0 -> rows 0x1002 / 0x300C, no sound, no particle */
        push13(m, r);
        flash_row(m, a5, 0x0062B030u, 0x0062B1D0u, 0x1002u); /* 0x13228C..0x1322B4 */
        flash_row(m, a5, 0x0062B630u, 0x0062B7D0u, 0x300Cu); /* 0x1322C0..0x1322E8 */
        pop13(m, r);                                 /* 0x1322EC movem.l (a7)+ */
        goto adv;                                    /* falls into 0x1322F0 */
    }

    /* 0x1321FC: flag non-zero -> rows 0x3002 / 0x100C, sound 0x0E, particle */
    push13(m, r);
    flash_row(m, a5, 0x0062B030u, 0x0062B1D0u, 0x3002u); /* 0x132208..0x132230 */
    flash_row(m, a5, 0x0062B630u, 0x0062B7D0u, 0x100Cu); /* 0x13223C..0x132264 */
    push_word(m, r, 0x000Eu);                        /* 0x132268 move.w #$e,-(a7) */
    push_long(m, r, 0x132272u);                      /* 0x13226C jsr $2f3a */
    snd_2F3A(m, r);
    r->a[7] += 2u;                                   /* 0x132272 addq.l #2,a7 */
    push_long(m, r, 0x132278u);                      /* 0x132274 bsr.w $132392 */
    part_132392(m, r);
    pop13(m, r);                                     /* 0x132278 movem.l (a7)+ */
    /* 0x13227C bra.w 0x1322F0 */

adv:                                                 /* 0x1322F0 */
    wr16(m, a5 + 0x34E8u, rd16(m, 0x00132338u + idx * 4u)); /* $46(pc,d0.w) */
    /* 0x1322F6/0x1322F8 nop */
    wr16(m, a5 + 0x34ECu, (uint16_t)(rd16(m, a5 + 0x34ECu) + 1u)); /* 0x1322FA */
    /* falls into 0x1322FE */

tail:                                                /* 0x1322FE */
    d0 = set_word(d0, rd16(m, a5 + 0x34EAu));        /* 0x1322FE -->
        the record tables below are indexed by this word, not by the step */
    fidx = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;
    if (rd16(m, a5 + 0x1CF0u) == 0u)                 /* 0x132302 tst.w ; beq */
        goto done;
    a0 = rd32(m, a5 + 0x1D00u);                      /* 0x132308 movea.l $1d00(a5) */
    a0 -= 2u;                                        /* 0x13230C clr.w -(a0) */
    wr16(m, a0, 0u);
    a0 -= 2u;                                        /* 0x13230E clr.w -(a0) */
    wr16(m, a0, 0u);
    a0 -= 2u;                                        /* 0x132310 $62(pc,d0.w) */
    wr16(m, a0, rd16(m, 0x00132374u + fidx * 2u));
    /* 0x132314/0x132316 nop */
    a0 -= 2u;                                        /* 0x132318 move.w #$0,-(a0) */
    wr16(m, a0, 0u);
    a0 -= 2u;                                        /* 0x13231C move.w #$0,-(a0) */
    wr16(m, a0, 0u);
    a0 -= 2u;                                        /* 0x132320 clr.w -(a0) */
    wr16(m, a0, 0u);
    a0 -= 2u;                                        /* 0x132322 clr.w -(a0) */
    wr16(m, a0, 0u);
    a0 -= 4u;                                        /* 0x132324 $46(pc,d0.w) */
    wr32(m, a0, rd32(m, 0x0013236Cu + fidx * 4u));
    /* 0x132328/0x13232A nop */
    wr16(m, a5 + 0x1CF0u,
         (uint16_t)(rd16(m, a5 + 0x1CF0u) - 1u));    /* 0x13232C subq.w #1 */
    wr32(m, a5 + 0x1D00u, a0);                       /* 0x132330 move.l a0,$1d00 */

done:                                                /* 0x132334 rts */
    r->d[0] = d0;
    r->a[0] = a0;
    r->a[7] += 4u;
    return 0;
}
