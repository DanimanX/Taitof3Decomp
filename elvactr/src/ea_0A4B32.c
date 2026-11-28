/* ROM 0x0A4B32: the "REST TIME" countdown of the stage HUD (state word at
 * A5-0x60FA, 32-bit counter at A5-0x60F8, tick value at A5-0x60F6):
 *   -1 -> clear the two text rows through 0x0A4C70 and arm the state to 1,
 *   >0 -> nothing, 0 -> decrement the counter and, while it is at or above
 *   0xE10 (one hour), do nothing; below that draw "REST TIME", the " : "
 *   separator and the remaining minutes/seconds as two 2-digit fields at
 *   0x61C8A4 / 0x61C8AA, playing the chime 0x63 / 0xA3 on the exact tick
 *   values the sound helper 0x0A4BE0 watches for. Once the counter is <= 0 the
 *   ROM tail-jumps into the shared "time over" block at 0x0A4CAE, which is not
 *   translated yet (it calls 0x00393C). */
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
/* 0x006AA8 (not translated yet): write `count` decimal digit words, right to
 * left, filling the words dest+0 .. dest+2*(count-1). Every word is
 * ((attr & 0x3F) << 9) | ('0' + digit): 0x0A30-based tiles for attr 5.
 * It saves D1-D5/A0, so only D0 (= dest + 2*count) survives. */
static void Sub_6AA8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d3 = rd32(m, r->a[7] + 4u);             /* 0x006AB0 move.l $8(a6),d3 */
    uint16_t d1 = rd16(m, r->a[7] + 8u);             /* 0x006AB4 move.w $c(a6),d1 */
    uint32_t a0 = rd32(m, r->a[7] + 10u);            /* 0x006AB8 movea.l $e(a6),a0 */
    uint16_t d2 = rd16(m, r->a[7] + 14u);            /* 0x006ABC move.w $12(a6),d2 */
    uint32_t d0, d4;

    d2 = (uint16_t)(d2 & 0x003Fu);                   /* 0x006AC0 andi.w #$3f */
    d2 = (uint16_t)(d2 + d2);                        /* 0x006AC4 add.w d2,d2 */
    d2 = (uint16_t)(d2 << 8);                        /* 0x006AC6 lsl.w #8 */
    d0 = (uint32_t)(uint16_t)(d1 << 1);              /* 0x006AC8/0x006ACA */
    a0 += (uint32_t)(int32_t)(int16_t)d0;            /* 0x006ACC adda.w d0,a0 */
    d1 = (uint16_t)(d1 - 1u);                        /* 0x006ACE subq.w #1,d1 */
    d0 = a0;                                         /* 0x006AD0 move.l a0,d0 */
    d4 = d3 & 0xFFFFu;                               /* 0x006AD2/0x006AD4 */
    d3 &= 0xFFFF0000u;                               /* 0x006AD6 clr.w d3 */
    d3 = (d3 << 16) | (d3 >> 16);                    /* 0x006AD8 swap d3 */

    for (;;) {
        if ((uint16_t)d3 == 0u && (uint16_t)d4 == 0u) {   /* 0x006ADA/0x006AE0 */
            if (a0 == d0) {                          /* 0x006AE6 cmpa.l d0,a0 */
                d2 = (uint16_t)(d2 + 0x0030u);       /* 0x006AE8 beq 0x006B10 */
            } else {
                d2 = (uint16_t)((d2 & 0xFF00u) | 0x0030u); /* 0x006AEC */
            }
        } else {
            uint32_t q;

            q = d3 / 10u;                            /* 0x006AF4 divu.w #$a,d3 */
            if (q <= 0xFFFFu)                        /* overflow leaves d3 alone */
                d3 = ((d3 % 10u) << 16) | q;
            d4 += d3 & 0xFFFF0000u;                  /* 0x006AF8..0x006AFC */
            d3 = (d3 << 16) | (d3 >> 16);            /* 0x006AFE swap d3 */
            d3 &= 0xFFFF0000u;                       /* 0x006B00 clr.w d3 */
            d3 = (d3 << 16) | (d3 >> 16);            /* 0x006B02 swap d3 */
            q = d4 / 10u;                            /* 0x006B04 divu.w #$a,d4 */
            if (q <= 0xFFFFu)
                d4 = ((d4 % 10u) << 16) | q;
            d4 = (d4 << 16) | (d4 >> 16);            /* 0x006B08 swap d4 */
            d2 = (uint16_t)((d2 & 0xFF00u) | (uint8_t)d4); /* 0x006B0A */
            d4 &= 0xFFFF0000u;                       /* 0x006B0C clr.w d4 */
            d4 = (d4 << 16) | (d4 >> 16);            /* 0x006B0E swap d4 */
            d2 = (uint16_t)(d2 + 0x0030u);           /* 0x006B10 addi.b #$30 */
        }
        a0 -= 2u;                                    /* 0x006B14 move.w d2,-(a0) */
        wr16(m, a0, d2);
        d1 = (uint16_t)(d1 - 1u);                    /* 0x006B16 dbra d1 */
        if (d1 == 0xFFFFu)
            break;
    }
    r->d[0] = d0;                                    /* D0 is not in the movem */
    r->a[7] += 4u;                                   /* 0x006B20 rts */
}

/* 0x0A4C70 (not translated yet): blank the two 8-character rows of the timer
 * text (0x61C820 and 0x61C8A0) with the space run at 0x0A4D58. Saves D0/D1. */
static void Sub_A4C70(const f3mem_t *m, f3regs_t *r)
{
    push_word(m, r, 0x0008u);                        /* 0x0A4C74 move.w #$8,-(a7) */
    push_long(m, r, 0x0061C820u);                    /* 0x0A4C78 pea.l */
    push_long(m, r, 0x000A4D58u);                    /* 0x0A4C7E pea.l $a4d58(pc) */
    push_long(m, r, 0x0A4C84u);                      /* 0x0A4C84 jsr $69f4 */
    Ea_0069F4(m, r);
    r->a[7] += 10u;                                  /* 0x0A4C8A lea.l $a(a7),a7 */
    push_word(m, r, 0x0008u);                        /* 0x0A4C8E */
    push_long(m, r, 0x0061C8A0u);                    /* 0x0A4C92 */
    push_long(m, r, 0x000A4D58u);                    /* 0x0A4C98 */
    push_long(m, r, 0x0A4C9Eu);                      /* 0x0A4C9E jsr $69f4 */
    Ea_0069F4(m, r);
    r->a[7] += 10u;                                  /* 0x0A4CA4 */
    r->a[7] += 4u;                                   /* 0x0A4CAC rts */
}

/* 0x0A4BE0 (not translated yet): play chime 0x63 on the hour marks of
 * A5-0x60F6 and chime 0xA3 on the half-hour marks (or when the counter is
 * exactly 0xDD4). */
static void Sub_A4BE0(const f3mem_t *m, f3regs_t *r, uint32_t a5)
{
    const uint16_t w = rd16(m, a5 - 0x60F6u);

    if (w == 0x0118u || w == 0x0244u || w == 0x0370u || w == 0x049Cu ||
        w == 0x05C8u || w == 0x06F4u || w == 0x0DFCu) {
        push_word(m, r, 0x0063u);                    /* 0x0A4C18 move.w #$63,-(a7) */
        push_long(m, r, 0x0A4C22u);                  /* 0x0A4C1C jsr $2f3a */
        snd_2F3A(m, r);
        r->a[7] += 2u;                               /* 0x0A4C22 lea.l 2(a7),a7 */
    }
    if (w == 0x00F0u || w == 0x021Cu || w == 0x0348u || w == 0x0474u ||
        w == 0x05A0u || w == 0x06CCu ||
        rd32(m, a5 - 0x60F8u) == 0x00000DD4u) {      /* 0x0A4C56 cmpi.l #$dd4 */
        push_word(m, r, 0x00A3u);                    /* 0x0A4C60 */
        push_long(m, r, 0x0A4C6Au);                  /* 0x0A4C64 jsr $2f3a */
        snd_2F3A(m, r);
        r->a[7] += 2u;                               /* 0x0A4C6A */
    }
    r->a[7] += 4u;                                   /* 0x0A4C6E rts */
}

/* divu.w #$3c,Dn: quotient in the low word, remainder in the high word of the
 * same register; an overflowing quotient leaves Dn untouched. */
static uint32_t divu60(uint32_t v)
{
    if (v / 0x3Cu > 0xFFFFu)
        return v;
    return ((v % 0x3Cu) << 16) | (v / 0x3Cu);
}

/* 0x0A4B56: the two 2-digit fields of the timer at 0x61C8A4 / 0x61C8AA. */
static void draw_field(const f3mem_t *m, f3regs_t *r, uint32_t dest, uint32_t value)
{
    push_word(m, r, 0x0005u);                        /* move.w #$5,-(a7) */
    push_long(m, r, dest);                           /* pea.l */
    push_word(m, r, 0x0002u);                        /* move.w #$2,-(a7) */
    push_long(m, r, value);                          /* move.l dX,-(a7) */
    push_long(m, r, 0u);                             /* jsr 0x006AA8 */
    Sub_6AA8(m, r);
    r->a[7] += 12u;                                  /* lea.l $c(a7),a7 */
}

int Ea_0A4B32(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a5 = r->a[5];
    uint32_t d0 = r->d[0], d2 = r->d[2], d3 = r->d[3];

    if ((int16_t)rd16(m, a5 - 0x60FAu) < 0) {        /* 0x0A4B32 blt 0x0A4B4A */
        push_long(m, r, 0x0A4B4Eu);                  /* 0x0A4B4A bsr.w $a4c70 */
        Sub_A4C70(m, r);
        wr16(m, a5 - 0x60FAu, 0x0001u);              /* 0x0A4B4E move.w #$1 */
        goto done;                                   /* 0x0A4B54 rts */
    }
    if ((int16_t)rd16(m, a5 - 0x60FAu) > 0)          /* 0x0A4B38 bgt 0x0A4B54 */
        goto done;
    wr32(m, a5 - 0x60F8u, rd32(m, a5 - 0x60F8u) - 1u); /* 0x0A4B3A subq.l #1 */
    if ((int32_t)rd32(m, a5 - 0x60F8u) >= 0x00000E10) /* 0x0A4B3E cmpi.l/blt */
        goto done;                                   /* 0x0A4B48 rts */

    push_long(m, r, 0x0A4B5Au);                      /* 0x0A4B56 bsr.w $a4be0 */
    Sub_A4BE0(m, r, a5);

    push_word(m, r, 0x0008u);                        /* 0x0A4B5A move.w #$8,-(a7) */
    push_long(m, r, 0x0061C820u);                    /* 0x0A4B5E pea.l $61c820 */
    push_long(m, r, 0x000A4D4Au);                    /* 0x0A4B64 pea.l $a4d4a(pc) */
    push_long(m, r, 0x0A4B6Au);                      /* 0x0A4B6A jsr $69f4 */
    Ea_0069F4(m, r);
    r->a[7] += 10u;                                  /* 0x0A4B70 lea.l $a(a7),a7 */

    push_word(m, r, 0x0005u);                        /* 0x0A4B74 move.w #$5,-(a7) */
    push_long(m, r, 0x0061C8A6u);                    /* 0x0A4B78 pea.l $61c8a6 */
    push_long(m, r, 0x000A4D54u);                    /* 0x0A4B7E pea.l $a4d54(pc) */
    push_long(m, r, 0x0A4B84u);                      /* 0x0A4B84 jsr $69f4 */
    Ea_0069F4(m, r);
    r->a[7] += 10u;                                  /* 0x0A4B8A lea.l $a(a7),a7 */

    d2 = 0u;                                         /* 0x0A4B8E moveq #$0,d2 */
    d2 = rd32(m, a5 - 0x60F8u);                      /* 0x0A4B90 move.l */
    if ((int32_t)d2 <= 0)                            /* 0x0A4B94 ble.w 0x0A4CAE */
        return -1;     /* 0x0A4CAE "time over" block (needs 0x00393C): not done */

    /* 0x0A4B98 divu.w #$3c ; ext.l ; divu.w #$3c ; move.l d2,d3 ; swap d3 ;
     * ext.l d2 ; ext.l d3: D2 = minutes, D3 = seconds of A5-0x60F8. */
    d2 = divu60(d2);                                 /* 0x0A4B98 */
    d2 = (uint32_t)(int32_t)(int16_t)(uint16_t)d2;   /* 0x0A4B9C ext.l d2 */
    d2 = divu60(d2);                                 /* 0x0A4B9E */
    d3 = d2;                                         /* 0x0A4BA2 move.l d2,d3 */
    d3 = (d3 << 16) | (d3 >> 16);                    /* 0x0A4BA4 swap d3 */
    d2 = (uint32_t)(int32_t)(int16_t)(uint16_t)d2;   /* 0x0A4BA6 ext.l d2 */
    d3 = (uint32_t)(int32_t)(int16_t)(uint16_t)d3;   /* 0x0A4BA8 ext.l d3 */

    draw_field(m, r, 0x0061C8A4u, d2);               /* 0x0A4BAA..0x0A4BC0 */
    d0 = r->d[0];
    draw_field(m, r, 0x0061C8AAu, d3);               /* 0x0A4BC4..0x0A4BDA */
    d0 = r->d[0];
    /* 0x0A4BDE rts */

done:
    r->d[0] = d0;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[7] += 4u;
    return 0;
}
