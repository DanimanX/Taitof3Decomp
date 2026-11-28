/* ROM 0x0AC8E4: UNVERIFIED (no capture): task body of the bank/scroll debug
 * overlay (kernel task slot initialiser).
 * Before its first TRAP #5 it clears the sound upload word at 0x440000,
 * reinitialises line RAM (Gfx_InitLineRam, 0x005B0C) and, when the debug list
 * pointer A5-0xC4C is still null, seeds it with the ROM list head 0x000ACB92
 * (a chain of 12-byte records {next.l, prev.l, tilemap.l, name.l, ...}).
 * The frame body after the yield is Task_0AC8E4_Frame below.
 *
 * 0x0AC906 loop: bsr 0x0AC91C (the debug screen), then set A5-0x144B bit 0
 * unless bit 6 of A5-0x5DDB selects it, and yield at 0x0AC904 again.
 *
 * 0x0AC91C draws the bank/scroll debug page: it polls 0x19FFF0 with the input
 * byte A5-0x5DDC as the button mask and the menu value 1 in [0..2] and walks
 * the record list in the direction that value says (1 = stay, 0 = through the
 * previous link, 2 = through the next link, stopping at a negative terminator),
 * re-polls while the new record's +4 word equals the old one's and A5-0x5DDC
 * bit 4 is clear. It then clears the old record's rect (0x0054AC with
 * 0x610084), blits the new record's tilemap (0x0053B2), nudges the bank byte
 * A5-0xC48 with the A5-0x5DDB bits 0/1 and clamps it into 0..7, reloads that
 * sound bank (0x001504 with D0 = the byte), stores 0x8000 to 0x440000, nudges
 * the scroll words A5-0x5DA8 / A5-0x5DA4 from the A5-0x5DD8 direction bits and
 * prints the record name, the descriptor size bytes, the bank number and the
 * descriptor address on the text layer (0x005666 / 0x00571A / 0x005690). */
#include "fn_auto.h"
#include "../kernel/fn_010352.h"
#include "../kernel/fn_010420.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x001504 + 0x001520: reload sound bank 1 with block index (D0.w & 0xFF)
 * % 8 (0x001516 divu.w #8 leaves the remainder in the low word after the
 * swap), copying each {count.w, {src_off.w, dst_off.w}*count} block from
 * 0x1FB40 through to the upload buffer 0x440000; a negative block count ends
 * the copy. The leaf is a straight-line loop with no save frame, so its
 * leftovers land in A0/A2 and D0-D3; they are reported through the out
 * parameters (the caller only saves A0/A1, so A2/D0-D3 really are live at the
 * end of 0x0AC91C). */
static void sound_bank1_001504(const f3mem_t *m, uint8_t idx,
                               uint32_t *a2_out, uint32_t *d0_out,
                               uint32_t *d1_out, uint32_t *d2_out,
                               uint32_t *d3_out)
{
    uint32_t tbl = 0x000168C0u;                              /* 0x001504 lea */
    uint32_t src = 0x0001FB40u;                              /* 0x00150A lea */
    uint16_t d0 = (uint16_t)((uint16_t)idx & 0xFFu);          /* 0x001510 andi.l */
    uint16_t q = (uint16_t)(d0 / 8u);                         /* 0x001516 divu.w */
    uint16_t r = (uint16_t)(d0 % 8u);
    uint32_t a2 = rd32(m, tbl + (uint32_t)r * 8u);            /* 0x001520 movea.l */
    int32_t d3 = (int32_t)rd32(m, tbl + 4u + (uint32_t)r * 8u); /* 0x001524 move.l */
    uint32_t d0l = ((uint32_t)r << 16) | q;                   /* 0x00151A swap */
    uint32_t d1l = 0;

    *d2_out = 0;
    if (d3 < 0) {                                             /* 0x001528 bmi 0x00154C */
        *a2_out = a2;
        *d0_out = d0l;
        *d1_out = d1l;
        *d3_out = (uint32_t)d3;
        return;
    }
    do {
        d3--;
        int16_t d2 = (int16_t)(rd16(m, a2) - 1u);              /* 0x001530 move.w (A2)+,D2 */
        a2 += 2u;
        do {
            d2--;
            d0l = (d0l & 0xFFFF0000u) | rd16(m, a2);           /* 0x001534 moveq #0,D0 */
            d1l = (d1l & 0xFFFF0000u) | rd16(m, a2 + 2u);
            a2 += 4u;
            wr32(m, 0x00440000u + (uint16_t)d1l, rd32(m, src + d0l)); /* 0x00153A */
            d0l += 4u;                                         /* 0x001540 addq.l #4,D0 */
            d1l = (d1l & 0xFFFF0000u) | (uint16_t)(d1l + 4u);  /* 0x001542 addq.w #4,D1 */
            *d2_out = (uint32_t)(int32_t)d2;
        } while (d2 >= 0);                                     /* 0x001544 dbra D2 */
        *d2_out = 0xFFFFFFFFu;
    } while (d3 >= 0);                                         /* 0x001548 dbra D3 */
    *a2_out = a2;
    *d0_out = d0l;
    *d1_out = d1l;
    *d3_out = (uint32_t)d3;
}

/* 0x0AC91C: redraw the debug page for the selected record. */
static void screen_0AC91C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = rd32(m, F3_A5 - 0x0C4Cu);              /* 0x0AC91C movea.l */
    uint32_t a2, a3, a4;
    uint16_t d1w;

    a2 = rd32(m, a1); a1 += 4u;                          /* 0x0AC920 movea.l (A1)+,A2 */
    a3 = rd32(m, a1); a1 += 4u;                          /* 0x0AC922 movea.l (A1)+,A3 */
    r->a[2] = a2;
    r->a[3] = a3;

    {
        /* 0x0AC924 movem.l a0-a3,-(A7) ; 0x0AC92C movem.l d0,-(A7) ;
         * 0x0AC930 movem.l a0,-(A7): a 24-byte save image whose only purpose
         * is that A0/A1/D0 survive the poll (A2/A3 too, via the movem pair).
         * The arguments of 0x19FFF0, from the lowest address up, are the
         * sign-extended A5-0x5DDC byte, the value 1, lower 0, upper 2, step 1;
         * the jsr pushes its own return address below them. */
        uint32_t saved_a0 = r->a[0], saved_a1 = r->a[1];
        uint32_t saved_a2 = r->a[2], saved_a3 = r->a[3], saved_d0 = r->d[0];
        uint32_t sp = r->a[7];

        sp -= 4u; wr32(m, sp, saved_a3);                 /* 0x0AC928 a3 */
        sp -= 4u; wr32(m, sp, saved_a2);                 /*           a2 */
        sp -= 4u; wr32(m, sp, saved_a1);                 /*           a1 */
        sp -= 4u; wr32(m, sp, saved_a0);                 /*           a0 */
        sp -= 4u; wr32(m, sp, saved_d0);                 /* 0x0AC92C d0 */
        sp -= 4u; wr32(m, sp, saved_a0);                 /* 0x0AC930 a0 */
        sp -= 2u; wr16(m, sp, 0x0001u);                  /* 0x0AC934 step */
        sp -= 2u; wr16(m, sp, 0x0002u);                  /* 0x0AC938 upper */
        sp -= 2u; wr16(m, sp, 0x0000u);                  /* 0x0AC93C lower */
        sp -= 2u; wr16(m, sp, 0x0001u);                  /* 0x0AC940 value */
        sp -= 2u; wr16(m, sp, (uint16_t)(int16_t)(int8_t)rd8(m, F3_A5 - 0x5DDCu));
        sp -= 4u; wr32(m, sp, 0x000AC950u);              /* the jsr's return address */
        r->a[7] = sp;
        (void)Fn_19FFF0(m, r);                           /* 0x0AC94A jsr 0x19FFF0 */
        d1w = (uint16_t)r->d[0];                         /* 0x0AC958 move.w D0,D1 */
        r->d[0] = saved_d0;                              /* pops in reverse order */
        r->a[0] = saved_a0;
        r->a[1] = saved_a1;
        r->a[2] = saved_a2;
        r->a[3] = saved_a3;
        /* 0x19FFF0 already popped the return address (+4); then
         * 0x0AC950 lea 0xA(A7),A7 and the three movem pops (+0x18). */
        r->a[7] = sp + 0x26u;
    }

    a4 = a1;                                             /* 0x0AC962 movea.l A1,A4 */
    for (;;) {                                           /* 0x0AC964 */
        if (d1w == 1u)                                   /* 0x0AC964 cmpi.w #1 / beq 0xAC9A0 */
            break;
        if ((int16_t)d1w < 1) {                          /* 0x0AC96C blt 0xAC97C */
            if ((int16_t)rd16(m, a2) < 0)                /* 0x0AC97C tst.w (A2) / bmi 0xAC9A0 */
                break;
            a1 = a2;                                     /* 0x0AC982 movea.l A2,A1 */
        } else {
            if ((int16_t)rd16(m, a3) < 0)                /* 0x0AC970 tst.w (A3) / bmi 0xAC9A0 */
                break;
            a1 = a3;                                     /* 0x0AC976 movea.l A3,A1 */
        }
        wr32(m, F3_A5 - 0x0C4Cu, a1);                    /* 0x0AC984 move.l A1,(A5-0xC4C) */
        a2 = rd32(m, a1); a1 += 4u;                      /* 0x0AC988 movea.l (A1)+,A2 */
        a3 = rd32(m, a1); a1 += 4u;                      /* 0x0AC98A movea.l (A1)+,A3 */
        r->a[2] = a2;
        r->a[3] = a3;
        if (rd8(m, F3_A5 - 0x5DDCu) & 0x10u)             /* 0x0AC98C btst.b #4 / bne 0xAC9A0 */
            break;
        if (rd16(m, a1 + 4u) != rd16(m, a4 + 4u))        /* 0x0AC996 cmp.w (0x4,A4),D0 / beq 0xAC964 */
            break;
    }

    r->a[1] = a1;
    if (a1 != a4) {                                      /* 0x0AC9A0 cmpa.l A1,A4 / beq 0xAC9AC */
        uint32_t a0 = rd32(m, a4);                       /* 0x0AC9A6 movea.l (A4)+,A0 */
        a4 += 4u;
        Video_ClearRect(m, a0, 0x00610084u);             /* 0x0AC9A8 bsr 0x0ACB52 -> 0x0054AC */
        r->a[1] = a1;
    }
    {
        uint32_t a0 = rd32(m, a1);                       /* 0x0AC9AC movea.l (A1)+,A0 */
        a1 += 4u;
        r->a[1] = a1;
        r->a[0] = a0;
        Gfx_DrawCompressedTilemap(m, a0, 0x00610084u, 0u); /* 0x0AC9AE bsr 0x0ACB32 */
    }

    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x01u) == 0u)            /* 0x0AC9B2 btst.b #0 / bne 0xAC9C4 */
        wr8(m, F3_A5 - 0x0C48u, (uint8_t)(rd8(m, F3_A5 - 0x0C48u) - 1u)); /* 0x0AC9BC */
    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x02u) == 0u) {           /* 0x0AC9C4 btst.b #1 / bne 0xACA24 */
        uint8_t bnk;

        wr8(m, F3_A5 - 0x0C48u, (uint8_t)(rd8(m, F3_A5 - 0x0C48u) + 1u)); /* 0x0AC9CE */
        bnk = rd8(m, F3_A5 - 0x0C48u);
        if ((int8_t)bnk < 0) {                               /* 0x0AC9D2 cmpi.b #0 / bge 0xAC9EC */
            bnk = (uint8_t)(bnk + 7);                        /* 0x0AC9DC addi.b #7 */
            bnk = (uint8_t)(bnk - 0);                        /* 0x0AC9E2 subi.b #0 */
            bnk = (uint8_t)(bnk + 1);                        /* 0x0AC9E8 addq.b #1 */
        }
        if ((int8_t)bnk > 7) {                               /* 0x0AC9EC cmpi.b #7 / ble 0xACA06 */
            bnk = (uint8_t)(bnk - 7);                        /* 0x0AC9F6 subi.b #7 */
            bnk = (uint8_t)(bnk + 0);                        /* 0x0AC9FC addi.b #0 */
            bnk = (uint8_t)(bnk - 1);                        /* 0x0ACA02 subq.b #1 */
        }
        wr8(m, F3_A5 - 0x0C48u, bnk);                        /* 0x0ACA06 stores D0.b */
        /* 0x0ACA0C movem.l a0-a1,-(A7) ; jsr 0x001504 ; 0x0ACA16 ;
         * 0x0ACA20 movem.l (A7)+,a0-a1 -- only A0/A1 come back. */
        {
            uint32_t sp = r->a[7];
            uint32_t a2, d0, d1, d2, d3;

            sp -= 4u; wr32(m, sp, r->a[1]);
            sp -= 4u; wr32(m, sp, r->a[0]);
            r->a[7] = sp;
            sound_bank1_001504(m, bnk, &a2, &d0, &d1, &d2, &d3);
            r->a[7] = sp + 8u;
            r->a[2] = a2;
            r->d[0] = d0;
            r->d[1] = d1;
            r->d[2] = d2;
            r->d[3] = d3;
        }
        wr32(m, 0x00440000u, 0x00008000u);                   /* 0x0ACA16 move.l #0x8000,(0x440000) */
    }

    /* 0x0ACA24..0x0ACA58: direction bits of A5-0x5DD8 nudge the two scroll
     * words (bit 3/2 -> A5-0x5DA8, bit 0/1 -> A5-0x5DA4). */
    if ((rd8(m, F3_A5 - 0x5DD8u) & 0x08u) == 0u)
        wr16(m, F3_A5 - 0x5DA8u, (uint16_t)(rd16(m, F3_A5 - 0x5DA8u) - 1u));
    if ((rd8(m, F3_A5 - 0x5DD8u) & 0x04u) == 0u)
        wr16(m, F3_A5 - 0x5DA8u, (uint16_t)(rd16(m, F3_A5 - 0x5DA8u) + 1u));
    if ((rd8(m, F3_A5 - 0x5DD8u) & 0x01u) == 0u)
        wr16(m, F3_A5 - 0x5DA4u, (uint16_t)(rd16(m, F3_A5 - 0x5DA4u) + 1u));
    if ((rd8(m, F3_A5 - 0x5DD8u) & 0x02u) == 0u)
        wr16(m, F3_A5 - 0x5DA4u, (uint16_t)(rd16(m, F3_A5 - 0x5DA4u) - 1u));

    /* 0x0ACA5C: print the 15-space filler at 0x61C004 (colour 7). */
    r->d[0] = Kernel_005666(m, 0x000ACB7Fu, 0x0061C004u, 7u);
    /* 0x0ACA76: the record's name string at A1 over the same destination. */
    r->d[0] = Kernel_005666(m, a1, 0x0061C004u, 7u);
    /* 0x0ACA8C: "SIZE:  *  " at 0x61C01A (colour 10). */
    r->d[0] = Kernel_005666(m, 0x000ACB6Eu, 0x0061C01Au, 0x000Au);
    /* 0x0ACAA6: the two descriptor bytes as 2-digit decimals. A0 points two
     * bytes into the descriptor record (it was advanced by the two reads). */
    {
        uint32_t a0 = r->a[0];
        uint8_t v;

        v = rd8(m, a0); a0 += 1u;                        /* 0x0ACAA8 move.b (A0)+,D0 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | v;
        r->d[0] = Gfx_DrawDecimal(m, r->d[0], 2u, 0x0061C02Au, 7u); /* 0x0ACABA jsr 0x571A */
        v = rd8(m, a0); a0 += 1u;                        /* 0x0ACAC6 move.b (A0)+,D0 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | v;
        r->d[0] = Gfx_DrawDecimal(m, r->d[0], 2u, 0x0061C024u, 7u); /* 0x0ACAD8 jsr 0x571A */
        r->a[0] = a0;
    }
    /* 0x0ACAE2: "BANK:" at 0x61C032 (colour 5). */
    r->d[0] = Kernel_005666(m, 0x000ACB79u, 0x0061C032u, 5u);
    /* 0x0ACAFC: the bank byte as one hex digit at the string's end. */
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | rd8(m, F3_A5 - 0x0C48u);
    r->d[0] = Gfx_DrawHex(m, r->d[1], 1u, r->d[0], 7u);  /* 0x0ACB0C jsr 0x5690 */
    /* 0x0ACB16: the descriptor pointer as 6 hex digits at 0x61C042. */
    r->d[0] = Gfx_DrawHex(m, r->a[0], 6u, 0x0061C042u, 8u); /* 0x0ACB26 jsr 0x5690 */
    /* 0x0ACB30 rts: the callees preserve A1-A3, so only the pointers change. */
    r->a[1] = a1;
    r->a[2] = a2;
    r->a[3] = a3;
    r->a[4] = a4;
}

int Fn_0AC8E4(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    wr32(m, 0x00440000u, 0x00008000u);              /* 0x0AC8E4 move.l #0x8000,(0x440000).l */
    Gfx_InitLineRam(m);                             /* 0x0AC8EE jsr 0x005B0C */
    if (rd32(m, F3_A5 - 0x0C4Cu) == 0u)             /* 0x0AC8F4 tst.l / bne 0xAC904 */
        wr32(m, F3_A5 - 0x0C4Cu, 0x000ACB92u);      /* 0x0AC8FC move.l #0xacb92,(A5-0xC4C) */
    return 0;                                       /* 0x0AC904 trap #5: the task yields */
}

/* One pass of the loop after the yield: 0x0AC906 -> 0x0AC91A. */
int Task_0AC8E4_Frame(const f3mem_t *m, f3regs_t *r)
{
    screen_0AC91C(m, r);                                 /* 0x0AC906 bsr.w 0x0AC91C */
    if ((rd8(m, F3_A5 - 0x5DDBu) & 0x40u) == 0u)         /* 0x0AC90A btst.b #6 / bne 0x0AC91A */
        wr8(m, F3_A5 - 0x144Bu, (uint8_t)(rd8(m, F3_A5 - 0x144Bu) | 0x01u)); /* 0x0AC914 bset.b #0 */
    return 0;                                            /* 0x0AC91A bra.w 0x0AC904 */
}
