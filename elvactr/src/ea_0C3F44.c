/* ROM 0x0C3F44: one step of the "bumped into a floor edge" script of the
 * object at A2. The byte at A2+0x1C is the phase: negative drops the object
 * from the handler list (0x0C412E + the block before this entry), positive
 * re-applies the last floor found, zero probes the grid at (A2+8, A2+0xC)
 * through 0x0BD546 and moves A2+8 by the signed byte at A2+0x10.
 *
 * When the probe found a solid cell the record is kept at A2+0x18 and
 * A2+0x1F is set; the floor found below the object is stored back as the
 * "previous" floor at 0x18(A2). A step that brings the object within
 * 0x14F (0xFFF0 in the other direction) of the scroll origin is snapped:
 * a 0x10-byte OSD sprite record (tile 0x44, the script at 0x3FBBA or
 * 0x3FBDA) is queued, sound 0x34A0+0x2F3A is played for the tile under the
 * object, and the step script at A2 stays 0x0C3F44. */
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

    push_movem(m, r, 0x0707u);                     /* 0x002F3A d0-d2/a0-a2 */
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
/* 0x0BD516 (not translated yet): probe the floor grid with D0.w = x, D1.w = y.
 * x and y are biased by A5+0x998/A5+0x99A and scaled (x/4, y/2); D1 then picks
 * a row of the table at A5+0x994 (word offsets) and D0, sign extended at
 * 0x0BD52E, is the BIT offset tested by `bftst (a0){d0:1}` (extension word
 * 0x0801 = a register offset, so the column is a bit inside the row bytes).
 * D0 = 0xFFFF when the bit is set, 0x0000 when clear, 0 out of range;
 * A0 = the row. Returns 1 when the Z flag is set (the caller's `beq`). */
static int Sub_BD516(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                     uint32_t *pd0, uint32_t *pd1, uint32_t *pa0)
{
    uint32_t d0 = *pd0, d1 = *pd1, a0 = *pa0;
    int z;

    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x0998u)));
    d0 = set_word(d0, (uint16_t)((int16_t)(uint16_t)d0 >> 2));
    if ((uint16_t)d0 >= rd16(m, a5 + 0x099Cu)) {     /* 0x0BD51C bcc */
        d0 = 0u;
        z = 1;
    } else {
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x099Au)));
        d1 = set_word(d1, (uint16_t)((int16_t)(uint16_t)d1 >> 1));
        if ((uint16_t)d1 >= rd16(m, a5 + 0x099Eu)) { /* 0x0BD52C bcc */
            d0 = 0u;
            z = 1;
        } else {
            int32_t bit = (int32_t)(int16_t)(uint16_t)d0; /* 0x0BD52E ext.l d0 */

            a0 = rd32(m, a5 + 0x0994u);
            a0 += (uint32_t)(int32_t)(int16_t)
                  rd16(m, a0 + 2u * (uint32_t)(int32_t)(int16_t)(uint16_t)d1);
            d0 = (uint32_t)bit;
            /* 0x0BD538 bftst (a0){d0:1}: byte bit>>3, mask 0x80 >> (bit & 7) */
            d0 = set_word(d0,
                          (rd8(m, (uint32_t)((int32_t)a0 + (bit >> 3))) &
                           (uint8_t)(0x80u >> (unsigned)(bit & 7))) != 0u
                          ? 0xFFFFu : 0x0000u);
            z = (d0 & 0xFFFFu) == 0u;
        }
    }
    *pd0 = d0;
    *pd1 = d1;
    *pa0 = a0;
    r->a[7] += 4u;
    return z;
}

/* cmp2.w (ea),Dn: 1 when the word is outside the signed {lo, hi} pair. */
static int outside_bounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);

    return (int16_t)v < lo || (int16_t)v > hi;
}

/* 0x0BD546 (not translated yet): region lookup over the count at 0x41B818 and
 * the record pointers at 0x41B81C (record: box 1 = +0x34/+0x3C, optional box 2
 * = +0x24/+0x2C). Returns 1 (box 1 hit) or 2 (box 2 hit) with D0 = that code
 * and A0 = the record; 0 (D0 = 0, the caller's `bne` not taken either) when
 * nothing matched; -1 when the grid cell is solid, in which case the movem.w
 * at 0x0BD54C has already put the caller's x back in D0, so the caller's `bgt`
 * decides on that x while its `bne` would branch too. */
static int Sub_BD546(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                     uint32_t *pd0, uint32_t *pd1, uint32_t *pa0)
{
    uint32_t d0 = *pd0, d1 = *pd1, d2 = r->d[2], a0 = *pa0, a1 = r->a[1];
    uint16_t d2w;
    int z, res = 0;

    push_word(m, r, (uint16_t)d1);                   /* 0x0BD546 movem.w d0-d1 */
    push_word(m, r, (uint16_t)d0);
    push_long(m, r, 0x0BD54Cu);                      /* 0x0BD54A bsr 0x0BD516 */
    z = Sub_BD516(m, r, a5, &d0, &d1, &a0);
    d0 = set_word(d0, rd16(m, r->a[7] + 0u));        /* 0x0BD54C movem.w (a7)+ */
    d1 = set_word(d1, rd16(m, r->a[7] + 2u));
    r->a[7] += 4u;
    if (!z) {                                        /* 0x0BD550 bne 0x0BD59C */
        *pd0 = d0;                                   /* D0/D1 were restored */
        *pd1 = d1;
        *pa0 = a0;
        r->a[7] += 4u;
        return -1;                                   /* solid cell (Z clear) */
    }
    if ((int16_t)rd16(m, 0x0041B818u) < 0) {         /* 0x0BD552 bmi 0x0BD59A */
        *pd0 = 0u;
        *pd1 = d1;
        *pa0 = a0;
        r->a[7] += 4u;
        return 0;
    }
    push_long(m, r, a1);                             /* 0x0BD55A movem.l d2/a1 */
    push_long(m, r, d2);
    d2w = rd16(m, 0x0041B818u);                      /* 0x0BD55E move.w */
    a1 = 0x0041B81Cu;                                /* 0x0BD564 lea */
    for (;;) {
        a0 = rd32(m, a1);                            /* 0x0BD56A movea.l (a1)+,a0 */
        a1 += 4u;
        if (!outside_bounds(m, a0 + 0x0034u, (uint16_t)d0) &&
            !outside_bounds(m, a0 + 0x003Cu, (uint16_t)d1)) {
            res = 1;                                 /* 0x0BD57A bcc 0x0BD5A6 */
            break;
        }
        if (rd32(m, a0 + 0x0024u) != 0u &&           /* 0x0BD57C tst.l */
            !outside_bounds(m, a0 + 0x0024u, (uint16_t)d0) &&
            !outside_bounds(m, a0 + 0x002Cu, (uint16_t)d1)) {
            res = 2;                                 /* 0x0BD590 bcc 0x0BD59E */
            break;
        }
        d2w = (uint16_t)(d2w - 1u);                  /* 0x0BD592 dbra d2 */
        if (d2w == 0xFFFFu)
            break;
    }
    d2 = rd32(m, r->a[7] + 0u);                      /* movem.l (a7)+,d2/a1 */
    a1 = rd32(m, r->a[7] + 4u);
    r->a[7] += 8u;
    r->d[2] = d2;
    r->a[1] = a1;
    *pd0 = (res == 0) ? 0u : (uint32_t)res;          /* 0x0BD59A / 0x0BD5A2 / A6 */
    *pd1 = d1;
    *pa0 = a0;
    r->a[7] += 4u;                                   /* 0x0BD59C / 0x0BD5A4 / A8 */
    return res;
}

/* 0x0BDE8C (not translated yet): sample the 4-bit tile attribute of the cell at
 * (D0.w, D1.w) in the 8-pixel grid based at A5+0x9A0. D0 = the nibble, 0 when
 * the point is out of the grid. */
static void Sub_BDE8C(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                      uint32_t *pd0, uint32_t *pd1, uint32_t *pa0)
{
    uint32_t d0 = *pd0, d1 = *pd1, a0 = *pa0;

    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x09A6u)));
    d1 = set_word(d1, (uint16_t)((int16_t)(uint16_t)d1 >> 3));  /* asr.w #3 */
    if ((uint16_t)d1 >= rd16(m, a5 + 0x09AAu))       /* 0x0BDE92 bcc */
        goto zero;
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x09A4u)));
    d0 = set_word(d0, (uint16_t)((int16_t)(uint16_t)d0 >> 3));
    if ((uint16_t)d0 >= rd16(m, a5 + 0x09A8u))       /* 0x0BDE9E bcc */
        goto zero;
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;   /* 0x0BDEA4 ext.l */
    d0 <<= 2;                                        /* 0x0BDEA6 lsl.l #2 */
    /* 0x0BDEA8 movea.l ([$9a0,a5],d1.w*4),a0: the row table is indirect */
    a0 = rd32(m, rd32(m, a5 + 0x09A0u) +
              4u * (uint32_t)(int32_t)(int16_t)(uint16_t)d1);
    d0 = (uint32_t)((rd8(m, a0) >> 4) & 0x0Fu);      /* 0x0BDEAE bfextu {0:4} */
    *pd0 = d0;
    *pd1 = d1;
    *pa0 = a0;
    r->a[7] += 4u;                                   /* 0x0BDEB2 rts */
    return;
zero:                                                /* 0x0BDEB4 */
    *pd0 = 0u;
    *pd1 = d1;
    *pa0 = a0;
    r->a[7] += 4u;
}

/* 0x0034A0 (not translated yet): play sound `id` (the word argument at 4(SP))
 * with the pitch byte in the argument's second word at 6(SP), keyed by the
 * sound record's second/third bytes and the note table at A5-0x6171. The
 * command block is built at A5-0x7E2C and sent through 0x002E8C. D0, D1, A0,
 * A1 and A2 are saved and restored. */
static void Sub_34A0(const f3mem_t *m, f3regs_t *r, uint32_t a5)
{
    const uint16_t id = rd16(m, r->a[7] + 4u);       /* 0x0034A4 move.w $18(a7),d0 */
    const uint8_t pitch = rd8(m, r->a[7] + 7u);      /* 0x0034EC move.b $1b(a7) */
    uint32_t d0s = r->d[0], d1s = r->d[1];
    uint32_t a0s = r->a[0], a1s = r->a[1], a2s = r->a[2];
    uint32_t a0 = rd32(m, a5 - 0x7EB4u);             /* 0x0034A8 */
    uint32_t a1 = 0x00C00000u;                       /* 0x0034D0 lea.l $c00000 */
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x480u) >> 1); /* 0x0034D6/0x0034DA */
    uint16_t d0;

    a0 = rd32(m, a0 +
              4u * (uint32_t)(int32_t)(int16_t)id);  /* 0x0034AC (a0,d0.w*4) */
    a0 += 1u;                                        /* 0x0034B0 adda.w #$1 */
    d0 = (uint16_t)((uint16_t)rd8(m, a0) - 1u);      /* 0x0034B4/0x0034B8 */
    d0 = (uint16_t)(d0 << 3);                        /* 0x0034BC asl.w #3 */
    d0 = (uint16_t)((d0 & 0xFF00u) |
                    (uint8_t)((uint8_t)d0 + rd8(m, a0 + 2u))); /* add.b $2(a0) */
    d0 = (uint16_t)rd8(m, a5 - 0x6171u + (uint32_t)d0); /* 0x0034C2 $9e8f(a5) */
    if (d0 == 0u)                                    /* 0x0034C8 beq 0x0034F8 */
        goto restore;
    if ((uint8_t)d0 != rd8(m, a0 + 5u))              /* 0x0034CA cmp.b $5(a0) */
        goto restore;
    wr16(m, a5 - 0x7E2Cu, 0x058Bu);                  /* 0x0034E0 move.w #$58b,(a2)+ */
    wr8(m, a5 - 0x7E2Au, rd8(m, a0));                /* 0x0034E4 */
    wr8(m, a5 - 0x7E29u, rd8(m, a0 + 2u));           /* 0x0034E6 */
    wr8(m, a5 - 0x7E28u, pitch);                     /* 0x0034EA */
    r->a[1] = a1;
    r->d[1] = set_word(r->d[1], d1);
    push_long(m, r, 0x0034F2u);                      /* 0x0034EE bsr.w $2e8c */
    Ea_002E8C(m, r);
    d1 = (uint16_t)(d1 + d1);                        /* 0x0034F2 add.w d1,d1 */
    wr16(m, a1 + 0x480u, d1);                        /* 0x0034F4 */

restore:                                             /* 0x0034F8 movem.l (a7)+ */
    r->d[0] = d0s;
    r->d[1] = d1s;
    r->a[0] = a0s;
    r->a[1] = a1s;
    r->a[2] = a2s;
    r->a[7] += 4u;                                   /* 0x0034FC rts */
}

/* 0x0C412E (not translated yet): drop A2 from the handler list at A4+0x10C
 * (count at A4+0x108), clearing the matching slot. */
static void Sub_C412E(const f3mem_t *m, f3regs_t *r, uint32_t a4)
{
    uint32_t a0 = a4 + 0x010Cu;                      /* 0x0C412E lea.l $10c(a4),a0 */
    uint16_t d0 = rd16(m, a4 + 0x0108u);             /* 0x0C4132 */

    for (;;) {
        uint32_t v = rd32(m, a0);                    /* 0x0C4136 cmpa.l (a0)+,a2 */
        a0 += 4u;
        if (v == r->a[2]) {
            wr32(m, a0 - 4u, 0u);                    /* 0x0C413A clr.l -$4(a0) */
            break;
        }
        d0 = (uint16_t)(d0 - 1u);                    /* 0x0C4140 dbra d0 */
        if ((uint16_t)d0 == 0xFFFFu)
            break;
    }
    r->d[0] = set_word(r->d[0], d0);
    r->a[0] = a0;
    r->a[7] += 4u;                                   /* 0x0C4144 rts */
}

/* 0x0C416C (not translated yet): scan the object list at A1 for a live object
 * (byte +0x1E clear, not A2 itself) whose position is within the cmp2 bounds
 * pair, and kill the pair. Returns the list pointer and D0/D2/D3 clobbered. */
static void Sub_C416C(const f3mem_t *m, f3regs_t *r, uint32_t a2,
                      uint32_t *pd0, uint32_t *pd2, uint32_t *pd3,
                      uint32_t d4, uint32_t d5, uint32_t *pa1)
{
    uint32_t d0 = *pd0, d2 = *pd2, d3 = *pd3, a1 = *pa1;

    for (;;) {
        d2 = rd32(m, a1);                            /* 0x0C416C move.l (a1)+,d2 */
        a1 += 4u;
        if ((int32_t)d2 <= 0)                        /* 0x0C416E ble 0x0C41A2 */
            goto next;
        {
            uint32_t a0 = d2;                        /* 0x0C4170 movea.l d2,a0 */

            if (rd8(m, a0 + 0x1Eu) != 0u)            /* 0x0C4172 bne 0x0C41A2 */
                goto next;
            if (a0 == a2)                            /* 0x0C4178 beq 0x0C41A2 */
                goto next;
            d2 = set_word(d2, (uint16_t)(rd16(m, a0 + 0x0008u) - (uint16_t)d4));
            d3 = set_word(d3, (uint16_t)(rd16(m, a0 + 0x000Cu) - (uint16_t)d5));
            /* 0x0C4188 / 0x0C4190: the inline cmp2.w pair at 0x0C41A6 */
            if (outside_bounds(m, 0x000C41A6u, (uint16_t)d2))
                goto next;
            if (outside_bounds(m, 0x000C41A6u, (uint16_t)d3))
                goto next;
            wr32(m, a1 - 4u, 0u);                    /* 0x0C4198 clr.l -$4(a1) */
            wr32(m, a0 + 0x0004u, 0u);               /* 0x0C419C clr.l $4(a0) */
            wr16(m, a0, 0u);                         /* 0x0C41A0 clr.w (a0) */
        }
next:                                                /* 0x0C41A2 dbra d0,0x0C416C */
        d0 = set_word(d0, (uint16_t)(d0 - 1u));
        if ((uint16_t)d0 == 0xFFFFu)
            break;
    }
    *pd0 = d0;
    *pd2 = d2;
    *pd3 = d3;
    *pa1 = a1;
    r->a[7] += 4u;                                   /* 0x0C41A6 rts */
}

/* 0x0C4146 (not translated yet): run 0x0C416C over the two object lists at
 * A5-0x6F2 (0x30 entries, count at A5-0x6F6) and A5+0x1EA (count at A5+0x1E6)
 * with the object's own position as the reference. */
static void Sub_C4146(const f3mem_t *m, f3regs_t *r, uint32_t a5, uint32_t a2)
{
    uint32_t d0 = r->d[0], d2 = r->d[2], d3 = r->d[3], a1 = r->a[1];
    uint32_t d4 = set_word(r->d[4], rd16(m, a2 + 0x0008u));  /* 0x0C4146 */
    uint32_t d5 = set_word(r->d[5], rd16(m, a2 + 0x000Cu));  /* 0x0C414A */

    d0 = set_word(d0, rd16(m, a5 - 0x06F6u));        /* 0x0C414E */
    if ((int16_t)(uint16_t)d0 >= 0) {                /* 0x0C4152 bmi 0x0C415C */
        a1 = a5 - 0x06F2u;                           /* 0x0C4154 lea.l -$6f2(a5) */
        push_long(m, r, 0x0C415Cu);                  /* 0x0C4158 bsr.w $c416c */
        Sub_C416C(m, r, a2, &d0, &d2, &d3, d4, d5, &a1);
    }
    d0 = set_word(d0, rd16(m, a5 + 0x01E6u));        /* 0x0C415C */
    if ((int16_t)(uint16_t)d0 >= 0) {                /* 0x0C4160 bmi 0x0C416A */
        a1 = a5 + 0x01EAu;                           /* 0x0C4162 lea.l $1ea(a5) */
        push_long(m, r, 0x0C416Au);                  /* 0x0C4166 bsr.w $0c416c */
        Sub_C416C(m, r, a2, &d0, &d2, &d3, d4, d5, &a1);
    }
    r->d[0] = d0;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[5] = d5;
    r->a[1] = a1;
    r->a[7] += 4u;                                   /* 0x0C416A rts */
}

int Ea_0C3F44(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a5 = r->a[5];
    const uint32_t a4 = r->a[4];
    const uint32_t a2 = r->a[2];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t a0 = r->a[0], a1 = r->a[1];
    int code;

    if ((int8_t)rd8(m, a2 + 0x001Cu) < 0)            /* 0x0C3F44 tst.b ; bmi */
        goto unlink;                                 /* 0x0C4122 */
    if ((int8_t)rd8(m, a2 + 0x001Cu) > 0)            /* 0x0C3F4C bgt 0x0C4022 */
        goto busy;

    d1 = set_word(d1, rd16(m, a2 + 0x000Cu));        /* 0x0C3F50 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));        /* 0x0C3F54 */
    push_long(m, r, 0x0C3F5Eu);                      /* 0x0C3F58 jsr $bd546 */
    code = Sub_BD546(m, r, a5, &d0, &d1, &a0);
    if (code < 0) {                                  /* 0x0BD550 bne: solid */
        if ((int16_t)(uint16_t)d0 > 0)               /* 0x0C3F5E bgt 0x0C400E */
            goto hit;
        goto busy;                                   /* 0x0C3F62 bne 0x0C4022 */
    }
    if (code > 0)                                    /* bgt 0x0C400E */
        goto hit;
    /* code == 0: D0 = 0, Z set, `bne` not taken -> the step path */

    /* 0x0C3F66: the probe found a free cell, step the object */
    d0 = set_word(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a2 + 0x0010u)); /* ext.w */
    wr16(m, a2 + 0x0008u,
         (uint16_t)(rd16(m, a2 + 0x0008u) + (uint16_t)d0));  /* 0x0C3F6C */
    if (rd8(m, a2 + 0x0001u) != 0u)                  /* 0x0C3F70 bne 0x0C3FA0 */
        goto up;

    /* 0x0C3F76: walking towards A2+0x18 */
    d0 = set_word(d0, rd16(m, a2 + 0x0018u));
    d1 = set_word(d1, rd16(m, a2 + 0x0008u));
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;   /* ext.l d0 */
    d1 = (uint32_t)(int32_t)(int16_t)(uint16_t)d1;   /* ext.l d1 */
    d0 -= d1;                                        /* 0x0C3F82 sub.l d1,d0 */
    if ((int32_t)d0 < 0)                             /* 0x0C3F84 bmi 0x0C402E */
        goto alt;
    d1 = set_word(d1, rd16(m, a2 + 0x0008u));        /* 0x0C3F88 */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x1BC4u)));
    if ((int16_t)(uint16_t)d1 > (int16_t)rd16(m, 0x000C412Cu)) /* 0x0C3F90 bgt */
        goto unlink;
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - 0x000Fu)); /* 0x0C3F98 subi.w */
    d3 = 0x00000001u;                                /* 0x0C3F9C moveq #$1,d3 */
    goto draw;                                       /* 0x0C3F9E bra 0x0C3FC4 */

up:                                                  /* 0x0C3FA0 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));
    d1 = set_word(d1, rd16(m, a2 + 0x0018u));
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;   /* 0x0C3FA8 ext.l d0 */
    d1 = (uint32_t)(int32_t)(int16_t)(uint16_t)d1;   /* 0x0C3FAA ext.l d1 */
    d0 -= d1;                                        /* 0x0C3FAC sub.l d1,d0 */
    if ((int32_t)d0 < 0)                             /* 0x0C3FAE bmi 0x0C402E */
        goto alt;
    d1 = set_word(d1, rd16(m, a2 + 0x0008u));        /* 0x0C3FB2 */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x1BC4u)));
    if ((int16_t)(uint16_t)d1 < (int16_t)rd16(m, 0x000C412Au)) /* 0x0C3FBE blt */
        goto unlink;
    d3 = 0x00000000u;                                /* 0x0C3FC2 moveq #$0,d3 */

draw:                                                /* 0x0C3FC4 */
    d2 = set_word(d2, rd16(m, a2 + 0x000Cu));
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BC6u)));
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - 7u)); /* 0x0C3FCC subq.w #7 */
    d0 = 0x0003FBBAu;                                /* 0x0C3FCE move.l #$3fbba,d0 */
    if (rd16(m, a2 + 0x0016u) != 0u)                 /* 0x0C3FD4 beq */
        d0 = 0x0003FBDAu;                            /* 0x0C3FDA */
    if (rd16(m, a5 + 0x1CF2u) != 0u) {               /* 0x0C3FE0 beq 0x0C4004 */
        a0 = rd32(m, a5 + 0x1D04u);                  /* 0x0C3FE6 */
        a0 -= 2u;                                    /* 0x0C3FEA clr.w -(a0) */
        wr16(m, a0, 0u);
        a0 -= 2u;                                    /* 0x0C3FEC move.w d3,-(a0) */
        wr16(m, a0, (uint16_t)d3);
        a0 -= 2u;                                    /* 0x0C3FEE move.w #$44,-(a0) */
        wr16(m, a0, 0x0044u);
        a0 -= 2u;                                    /* 0x0C3FF2 move.w d2,-(a0) */
        wr16(m, a0, (uint16_t)d2);
        a0 -= 2u;                                    /* 0x0C3FF4 move.w d1,-(a0) */
        wr16(m, a0, (uint16_t)d1);
        a0 -= 2u;                                    /* 0x0C3FF6 clr.w -(a0) */
        wr16(m, a0, 0u);
        a0 -= 2u;                                    /* 0x0C3FF8 clr.w -(a0) */
        wr16(m, a0, 0u);
        a0 -= 4u;                                    /* 0x0C3FFA move.l d0,-(a0) */
        wr32(m, a0, d0);
        wr16(m, a5 + 0x1CF2u,
             (uint16_t)(rd16(m, a5 + 0x1CF2u) - 1u)); /* 0x0C3FFC */
        wr32(m, a5 + 0x1D04u, a0);                   /* 0x0C4000 */
    }
    wr32(m, a2 + 0x0004u, 0x000C3F44u);              /* 0x0C4004 */
    goto done;                                       /* 0x0C400C rts */

hit:                                                 /* 0x0C400E */
    wr32(m, a2 + 0x0018u, a0);                       /* 0x0C400E move.l a0,$18(a2) */
    wr8(m, a2 + 0x001Fu, 0xFFu);                     /* 0x0C4012 st.b $1f(a2) */
    a1 = rd32(m, 0x000C4DD2u);                       /* 0x0C4016 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));        /* 0x0C401C */
    goto common;                                     /* 0x0C4020 bra 0x0C4038 */

busy:                                                /* 0x0C4022 */
    a1 = rd32(m, 0x000C4DFCu);                       /* 0x0C4022 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));        /* 0x0C4028 */
    goto common;                                     /* 0x0C402C bra 0x0C4038 */

alt:                                                 /* 0x0C402E */
    a1 = rd32(m, 0x000C4DFCu);                       /* 0x0C402E */
    d0 = set_word(d0, rd16(m, a2 + 0x0018u));        /* 0x0C4034 */

common:                                              /* 0x0C4038 */
    /* movem.l d0-d3/a0-a1,-(a7): D0 ends at SP+0 */
    push_long(m, r, a1);
    push_long(m, r, a0);
    push_long(m, r, d3);
    push_long(m, r, d2);
    push_long(m, r, d1);
    push_long(m, r, d0);
    {
        uint32_t fd0 = d0, fd1 = d1, fd2 = d2;
        uint32_t fd3 = set_word(d3, (uint16_t)d0);   /* 0x0C403C move.w d0,d3 */

        fd1 = set_word(fd1, rd16(m, a2 + 0x000Cu));  /* 0x0C403E */
        push_long(m, r, 0x0C4048u);                  /* 0x0C4042 jsr $bde8c */
        Sub_BDE8C(m, r, a5, &fd0, &fd1, &a0);
        /* 0x0C4048 move.w (a1,d0.w*2),d2 */
        fd2 = set_word(fd2, rd16(m, a1 +
                      2u * (uint32_t)(int32_t)(int16_t)(uint16_t)fd0));
        fd3 = set_word(fd3, (uint16_t)((uint16_t)fd3 - rd16(m, a5 + 0x1BC4u)));
        fd3 = set_word(fd3, (uint16_t)((uint16_t)fd3 - 0x00A0u)); /* 0x0C4050 */
        fd3 = set_word(fd3, (uint16_t)((int16_t)(uint16_t)fd3 >> 1)); /* asr.w #1 */
        push_word(m, r, (uint16_t)fd3);              /* 0x0C4056 */
        push_word(m, r, (uint16_t)fd2);              /* 0x0C4058 */
        push_long(m, r, 0x0C4060u);                  /* 0x0C405A jsr $34a0 */
        Sub_34A0(m, r, a5);
        r->a[7] += 4u;                               /* 0x0C4060 addq.l #4,a7 */
        push_word(m, r, (uint16_t)fd2);              /* 0x0C4062 */
        push_long(m, r, 0x0C406Au);                  /* 0x0C4064 jsr $2f3a */
        snd_2F3A(m, r);
        r->a[7] += 2u;                               /* 0x0C406A addq.l #2,a7 */
    }
    /* 0x0C406C movem.l (a7)+,d0-d3/a0-a1 */
    d0 = rd32(m, r->a[7] + 0u);
    d1 = rd32(m, r->a[7] + 4u);
    d2 = rd32(m, r->a[7] + 8u);
    d3 = rd32(m, r->a[7] + 12u);
    a0 = rd32(m, r->a[7] + 16u);
    a1 = rd32(m, r->a[7] + 20u);
    r->a[7] += 24u;
    wr16(m, a2 + 0x0008u, (uint16_t)d0);             /* 0x0C4070 */
    wr16(m, a2 + 0x0002u, 0xFFFFu);                  /* 0x0C4074 */
    wr16(m, a2 + 0x0014u, 0u);                       /* 0x0C407A clr.w */
    wr8(m, a2 + 0x001Eu, 0u);                        /* 0x0C407E sf.b */
    push_long(m, r, 0x0C4086u);                      /* 0x0C4082 bsr.w $c412e */
    Sub_C412E(m, r, a4);
    push_long(m, r, 0x0C408Au);                      /* 0x0C4086 bsr.w $c4146 */
    Sub_C4146(m, r, a5, a2);
    /* 0x0C408A is the entry of the next routine (subq.w #$1,$14(A2) ...), which
     * this path falls into: not translated here. */
    return -1;

unlink:                                              /* 0x0C4122 */
    push_long(m, r, 0x0C4126u);                      /* 0x0C4122 bsr.w $c412e */
    Sub_C412E(m, r, a4);
    /* 0x0C4126 bra.w $c3eda: continues before this routine's entry. */
    return -1;

done:                                                /* 0x0C400C rts */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[7] += 4u;
    return 0;
}
