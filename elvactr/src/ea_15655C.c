/* ROM 0x15655C: Elevator Action Returns object state step. Gates on the counter
 * $96(A4) and the controller record at $4(A4), ticks $56(A4) down or re-arms
 * state 0x156726 (D0.W = 1), calls the controller-flag handler 0x150B5A when
 * nothing transitioned, then either runs the spawn block (five 0x393C records
 * 0x127/0x12B/0x12F/0x12A/0x12E plus one sound command) or the state handler
 * 0x14F2E0, and finally tail-jumps into the generic object step 0x3AAC. */
#include "ea_auto.h"

static int S_002F3A(const f3mem_t *m, f3regs_t *r);
static int S_00393C(const f3mem_t *m, f3regs_t *r);
static void S_0039C0(const f3mem_t *m, f3regs_t *r);
static void S_A68FE(const f3mem_t *m, f3regs_t *r);
static int S_003AAC(const f3mem_t *m, f3regs_t *r);
static void S_004856(const f3mem_t *m, f3regs_t *r);
static void S_0BDE8C(const f3mem_t *m, f3regs_t *r);
static int S_14F2E0(const f3mem_t *m, f3regs_t *r);
static void S_14F392(const f3mem_t *m, f3regs_t *r);
static void S_14F3F8(const f3mem_t *m, f3regs_t *r);
static int S_14FA3C(const f3mem_t *m, f3regs_t *r);
static int S_14F41E(const f3mem_t *m, f3regs_t *r);
static void S_0039F0(const f3mem_t *m, f3regs_t *r);
static void S_15073C(const f3mem_t *m, f3regs_t *r);
static void S_A6954(const f3mem_t *m, f3regs_t *r);
static int S_15033C(const f3mem_t *m, f3regs_t *r);
static int S_15039A(const f3mem_t *m, f3regs_t *r);
static void S_150B26(const f3mem_t *m, f3regs_t *r);
static int S_150B5A(const f3mem_t *m, f3regs_t *r);
static void S_15696A(const f3mem_t *m, f3regs_t *r);

/* Word-sized write into a data register: .w ops keep the upper half. */
static uint32_t word_of(uint32_t reg)
{
    return reg & 0xFFFFu;
}

/* 0x002F3A: queue one sound command. The id is a word stack argument at
 * 0x1C(SP) (return address + the five longs the movem below pushes). While the
 * sound lock byte at -$7EB0(A5) is set, or the id is negative or greater than
 * the word at 0x8334, the command is dropped. Otherwise the id indexes the
 * record-pointer table at -$7EB4(A5), the record's first byte selects a handler
 * through the word table at 0x2F84, and the command counter at 0xC00480 doubles
 * once the handler ran. */
static int S_002F3A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    int rc = 0;

    /* 0x002F3A movem.l d0-d2/a0-a2,-(a7): predecrement stores A2,A1,A0,D2,D1,D0 */
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);
    /* 0x002F3E tst.b -$7eb0(a5) ; bne.b $2f7e */
    if (rd8(m, a5 - 0x7EB0u) == 0u) {
        /* 0x002F44 move.w $1c(a7),d0 ; bmi.b $2f7e */
        int16_t id = (int16_t)rd16(m, r->a[7] + 0x1Cu);
        /* 0x002F4A move.w $8334.l,d2 ; cmp.w d2,d0 ; bgt.b $2f7e */
        if (id >= 0 && id <= (int16_t)rd16(m, 0x8334u)) {
            const uint32_t a1 = 0xC00000u;                    /* 0x002F54 lea.l $c00000.l,a1 */
            uint32_t a0, tgt;
            /* 0x002F5A move.w $480(a1),d1 ; 0x002F5E lsr.w #$1,d1 */
            r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, a1 + 0x480u) >> 1));
            /* 0x002F60 movea.l -$7eb4(a5),a0 ; 0x002F64 movea.l (a0,d0.w*4),a0 */
            a0 = rd32(m, rd32(m, a5 - 0x7EB4u) + (uint32_t)((int32_t)id * 4));
            r->a[0] = a0;
            /* 0x002F68 moveq #$0,d0 ; 0x002F6A move.b (a0)+,d0 */
            r->d[0] = 0u;
            r->d[0] = set_word(r->d[0], rd8(m, a0));
            r->a[0] = a0 + 1u;
            /* 0x002F6C move.w $2f84(pc,d0.w),d0: word offsets based at 0x2F84 */
            r->d[0] = set_word(r->d[0],
                               rd16(m, 0x2F84u + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0] * 2u));
            /* 0x002F74 jsr $2f84(pc,d0.w) */
            tgt = 0x2F84u + (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
            switch (tgt) {
            case 0x3134u:
                /* 0x3134 is a bare rts: the command is only counted. */
                break;
            case 0x2F8Eu:   /* 0x2F8E: id latch + 0x381/0x486 command pair */
            case 0x3024u:   /* 0x3024: same with a masked id + 0x488 argument */
            case 0x307Au:   /* 0x307A: 0x58F/0x68D/0x68E block with a duration */
            case 0x311Eu:   /* 0x311E: byte ring-buffer fill through A1 */
                rc = -1;    /* handler bodies not translated */
                break;
            default:
                rc = -1;
                break;
            }
            /* 0x002F78 add.w d1,d1 ; 0x002F7A move.w d1,$480(a1) */
            r->d[1] = set_word(r->d[1], (uint16_t)((uint16_t)r->d[1] + (uint16_t)r->d[1]));
            wr16(m, a1 + 0x480u, (uint16_t)r->d[1]);
        }
    }
    /* 0x002F7E movem.l (a7)+,d0-d2/a0-a2 */
    r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->d[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->d[2] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[2] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[7] += 4u;                                          /* 0x002F82 rts */
    return rc;
}

/* 0x0039C0: take a 0x3A-long object slot off the 0x60EE(A5) free list into A4
 * (D0 = it) and zero it, or return D0 = 0 when the list reached its end marker
 * 0x403090. Contains the 0x0039E0 fill loop it bsr's. */
static void S_0039C0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    /* 0x0039C0 movea.l -$60ee(a5),a0 ; 0x0039C4 cmpa.l #$403090,a0 ; bcc.b $39dc */
    uint32_t a0 = rd32(m, a5 - 0x60EEu);
    r->a[0] = a0;
    if (a0 < 0x403090u) {
        /* 0x0039CC movea.l (a0),a4 ; 0x0039CE move.l a4,d0 ; 0x0039D0 clr.l (a0)+ */
        r->a[4] = rd32(m, a0);
        r->d[0] = r->a[4];
        wr32(m, a0, 0u);
        /* 0x0039D2 move.l a0,-$60ee(a5) */
        wr32(m, a5 - 0x60EEu, a0 + 4u);
        /* 0x0039D6 bsr.w $39e0: movea.l a4,a0 ; move.w #$39,d2 ; moveq #$0,d1 ;
         * move.l d1,(a0)+ ; dbra d2 (0x3A longs), then the rts of 0x39DE. */
        r->d[1] = 0u;
        r->d[2] = set_word(r->d[2], 0x0039u);
        {
            uint32_t p = r->a[4];
            for (int i = 0; i <= 0x39; i++) {
                wr32(m, p, r->d[1]);
                p += 4u;
            }
            r->a[0] = p;
        }
        r->d[2] = set_word(r->d[2], (uint16_t)((uint16_t)r->d[2] - 1u)); /* dbra ends on -1 */
        /* 0x0039DA bra.b $39de */
    } else {
        r->d[0] = 0u;                                       /* 0x0039DC moveq #$0,d0 */
    }
    r->a[7] += 4u;                                          /* 0x0039DE rts */
}

/* 0x0A68FE: take a sub-record off the 0xAE8(A5) free list into $4(A4) and
 * initialise it; D0 = the record, or 0 when the list reached 0x407518. */
static void S_A68FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    /* 0x0A68FE movea.l -$ae8(a5),a0 ; 0x0A6902 cmpa.l #$407518,a0 ; bcc.b $a6936 */
    uint32_t a0 = rd32(m, a5 - 0xAE8u);
    r->a[0] = a0;
    if (a0 < 0x407518u) {
        uint32_t a1 = rd32(m, a0);                          /* 0x0A690A movea.l (a0),a1 */
        r->a[1] = a1;
        r->d[0] = a1;                                       /* 0x0A690C move.l a1,d0 */
        wr32(m, a0, 0u);                                    /* 0x0A690E clr.l (a0)+ */
        wr32(m, a5 - 0xAE8u, a0 + 4u);                      /* 0x0A6910 */
        wr32(m, r->a[4] + 4u, a1);                          /* 0x0A6914 move.l a1,$4(a4) */
        wr16(m, a1, 0u);                                    /* 0x0A6918 clr.w (a1) */
        wr16(m, a1 + 0x08u, 0u);                            /* 0x0A691A clr.w $8(a1) */
        wr16(m, a1 + 0x04u, 0u);                            /* 0x0A691E clr.w $4(a1) */
        wr16(m, a1 + 0x0Cu, 0u);                            /* 0x0A6922 clr.w $c(a1) */
        wr16(m, a1 + 0x10u, 0u);                            /* 0x0A6926 clr.w $10(a1) */
        wr8(m, a1 + 0x12u, 0xFFu);                          /* 0x0A692A move.b #$ff,$12(a1) */
        wr8(m, a1 + 0x13u, 0u);                             /* 0x0A6930 clr.b $13(a1) */
    } else {
        r->d[0] = 0u;                                       /* 0x0A6936 moveq #$0,d0 */
    }
    r->a[7] += 4u;                                          /* 0x0A6938 rts */
}

/* 0x0A6954: hand the sub-record at $4(A4) back to the 0xAE8(A5) free list. */
static void S_A6954(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    /* 0x0A6954 move.l $4(a4),d0 ; beq.b $a6974 */
    uint32_t d0 = rd32(m, r->a[4] + 4u);
    if (d0 != 0u) {
        uint32_t a1 = d0;                                   /* 0x0A695A movea.l d0,a1 */
        uint32_t a0 = rd32(m, a5 - 0xAE8u);                 /* 0x0A695C */
        r->a[1] = a1;
        wr32(m, a0 - 4u, a1);                               /* 0x0A6960 move.l a1,-(a0) */
        wr32(m, a5 - 0xAE8u, a0 - 4u);                      /* 0x0A6962 */
        r->a[0] = a0 - 4u;
        wr8(m, a1 + 0x13u, 0u);                             /* 0x0A6966 move.b #$0,$13(a1) */
        wr32(m, r->a[4] + 4u, 0u);                          /* 0x0A696C move.l #$0,$4(a4) */
    }
    r->a[7] += 4u;                                          /* 0x0A6974 rts */
}

/* 0x00393C: create one object with the staged spawn record (-$60C0/-$60C6/
 * -$60C4/-$60C2 at A5) and dispatch the per-type initialiser picked from the
 * 0xA6352 pointer table by (id-1)&0xFF. The staged record is written by the
 * caller right before each call. */
static int S_00393C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t d1;
    int rc = 0;

    /* 0x00393C movem.l d1-d2/a0-a2/a4,-(a7): stores A4,A2,A1,A0,D2,D1 */
    push_long(m, r, r->a[4]);
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    /* 0x003940 bsr.w $39c0 ; 0x003944 movem.l d0,-(a7) */
    push_long(m, r, 0x3944u);
    S_0039C0(m, r);
    push_long(m, r, r->d[0]);
    /* 0x003948 tst.l d0 ; beq.b $39b6 */
    if (r->d[0] != 0u) {
        /* 0x00394C jsr $a68fe.l ; 0x003952 tst.l d0 ; beq.b $39b6 */
        push_long(m, r, 0x3952u);
        S_A68FE(m, r);
        if (r->d[0] != 0u) {
            uint32_t a4 = r->a[4];
            /* 0x003956 addq.w #$1,-$60f0(a5) */
            wr16(m, a5 - 0x60F0u, (uint16_t)(rd16(m, a5 - 0x60F0u) + 1u));
            /* 0x00395A move.l #$0,$38(a4) */
            wr32(m, a4 + 0x38u, 0u);
            /* 0x003962 move.l -$60e2(a5),d1 ; bne.b $396c */
            d1 = rd32(m, a5 - 0x60E2u);
            if (d1 == 0u)
                wr32(m, a5 - 0x60E6u, a4);                 /* 0x003968 */
            /* 0x00396C move.l a4,-$60e2(a5) ; 0x003970 move.l d1,$34(a4) ; beq.b $397c */
            wr32(m, a5 - 0x60E2u, a4);
            wr32(m, a4 + 0x34u, d1);
            if (d1 != 0u)                                   /* 0x003974 */
                wr32(m, d1 + 0x38u, a4);                    /* 0x003976 movea.l d1,a0 ; move.l a4,$38(a0) */
            /* 0x00397C move.w -$60c0(a5),$5c(a4) */
            wr16(m, a4 + 0x5Cu, rd16(m, a5 - 0x60C0u));
            /* 0x003982 move.w $1bac(a5),$d2(a4) */
            wr16(m, a4 + 0xD2u, rd16(m, a5 + 0x1BACu));
            /* 0x003988 move.w -$60c6(a5),$46(a4) */
            wr16(m, a4 + 0x46u, rd16(m, a5 - 0x60C6u));
            /* 0x00398E move.w -$60c4(a5),$4a(a4) */
            wr16(m, a4 + 0x4Au, rd16(m, a5 - 0x60C4u));
            /* 0x003994 move.w -$60c2(a5),d1 ; 0x003998 move.w d1,$5a(a4) */
            r->d[1] = set_word(r->d[1], rd16(m, a5 - 0x60C2u));
            wr16(m, a4 + 0x5Au, (uint16_t)r->d[1]);
            /* 0x00399C subq.w #$1,d1 ; 0x00399E andi.w #$ff,d1 */
            r->d[1] = set_word(r->d[1], (uint16_t)((uint16_t)r->d[1] - 1u));
            r->d[1] = set_word(r->d[1], (uint16_t)(word_of(r->d[1]) & 0x00FFu));
            /* 0x0039A2 lea.l $a6352.l,a0 ; 0x0039A8 movea.l (a0,d1.w*4),a1 */
            r->a[0] = 0xA6352u;
            r->a[1] = rd32(m, 0xA6352u + (uint32_t)(int32_t)(int16_t)word_of(r->d[1]) * 4u);
            /* 0x0039AC movem.l d0,-(a7) ; 0x0039B0 jsr (a1) ; 0x0039B2 movem.l (a7)+,d0 */
            rc = -1;    /* the per-type initialiser reached through 0xA6352 is not translated */
        }
    }
    /* 0x0039B6 movem.l (a7)+,d0 */
    r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    /* 0x0039BA movem.l (a7)+,d1-d2/a0-a2/a4 */
    r->d[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->d[2] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[2] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[4] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[7] += 4u;                                          /* 0x0039BE rts */
    return rc;
}

/* 0x004856: copy the object's position/size box from A4 into the on-screen
 * record at $4(A4) and stamp it valid. */
static void S_004856(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a0, d0, d1;
    /* 0x004856 move.l $4(a4),d0 ; beq.w $48f4 */
    d0 = rd32(m, a4 + 0x04u);
    if (d0 == 0u) {
        r->a[7] += 4u;
        return;
    }
    a0 = d0;                                                /* 0x00485E movea.l d0,a0 */
    r->a[0] = a0;
    r->d[0] = d0;
    /* 0x004860 tst.w $ac(a4) ; bne.b $4876 */
    if (rd16(m, a4 + 0xACu) == 0u) {
        wr16(m, a4 + 0xA8u, rd16(m, a4 + 0x86u));           /* 0x004866 */
        d0 = set_word(d0, rd16(m, a4 + 0x70u));             /* 0x00486C */
        d1 = set_word(r->d[1], rd16(m, a4 + 0x7Au));        /* 0x004870 */
    } else {
        wr16(m, a4 + 0xA8u, rd16(m, a4 + 0x88u));           /* 0x004876 */
        d0 = set_word(d0, rd16(m, a4 + 0x72u));             /* 0x00487C */
        d1 = set_word(r->d[1], rd16(m, a4 + 0x7Cu));        /* 0x004880 */
    }
    wr16(m, a0 + 0x00u, (uint16_t)d0);                      /* 0x004884 move.w d0,(a0) */
    d0 = set_word(d0, (uint16_t)(word_of(d0) + rd16(m, a4 + 0x76u)));
    wr16(m, a0 + 0x02u, (uint16_t)d0);                      /* 0x00488A */
    d0 = set_word(d0, rd16(m, a4 + 0x74u));                 /* 0x00488E */
    wr16(m, a0 + 0x04u, (uint16_t)d0);                      /* 0x004892 */
    d0 = set_word(d0, (uint16_t)(word_of(d0) + rd16(m, a4 + 0x78u)));
    wr16(m, a0 + 0x06u, (uint16_t)d0);                      /* 0x00489A */
    wr16(m, a0 + 0x08u, (uint16_t)d1);                      /* 0x00489E */
    d1 = set_word(d1, (uint16_t)(word_of(d1) + rd16(m, a4 + 0x80u)));
    wr16(m, a0 + 0x0Au, (uint16_t)d1);                      /* 0x0048A6 */
    d0 = set_word(d0, rd16(m, a4 + 0x7Eu));                 /* 0x0048AA */
    wr16(m, a0 + 0x0Cu, (uint16_t)d0);                      /* 0x0048AE */
    d0 = set_word(d0, (uint16_t)(word_of(d0) + rd16(m, a4 + 0x82u)));
    wr16(m, a0 + 0x0Eu, (uint16_t)d0);                      /* 0x0048B6 */
    /* 0x0048BA move.w $8a(a4),$aa(a4) */
    wr16(m, a4 + 0xAAu, rd16(m, a4 + 0x8Au));
    /* 0x0048C0..0x0048D2: camera-relative offset added to both box ends */
    d0 = set_word(d0, rd16(m, a4 + 0x46u));
    d1 = set_word(d1, rd16(m, a4 + 0x4Au));
    d0 = set_word(d0, (uint16_t)(word_of(d0) + rd16(m, a4 + 0x52u)));
    d1 = set_word(d1, (uint16_t)(word_of(d1) + rd16(m, a4 + 0x54u)));
    wr16(m, a0 + 0x00u, (uint16_t)(rd16(m, a0 + 0x00u) + word_of(d0)));
    wr16(m, a0 + 0x02u, (uint16_t)(rd16(m, a0 + 0x02u) + word_of(d0)));
    wr16(m, a0 + 0x04u, (uint16_t)(rd16(m, a0 + 0x04u) + word_of(d1)));
    wr16(m, a0 + 0x06u, (uint16_t)(rd16(m, a0 + 0x06u) + word_of(d1)));
    wr16(m, a0 + 0x08u, (uint16_t)(rd16(m, a0 + 0x08u) + word_of(d0)));
    wr16(m, a0 + 0x0Au, (uint16_t)(rd16(m, a0 + 0x0Au) + word_of(d0)));
    wr16(m, a0 + 0x0Cu, (uint16_t)(rd16(m, a0 + 0x0Cu) + word_of(d1)));
    wr16(m, a0 + 0x0Eu, (uint16_t)(rd16(m, a0 + 0x0Eu) + word_of(d1)));
    wr8(m, a0 + 0x13u, 1u);                                 /* 0x0048EE move.b #$1,$13(a0) */
    /* 0x0048F4 rts */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[7] += 4u;
}

/* 0x003AAC: the generic object step (position box, view clip, sprite-list emit)
 * that 0x15655C reaches with a jmp, so its rts belongs to our caller.  The
 * 0x3B92/0x3A72 tail re-arms the object entry at 0x3A7A and returns. */
static int S_003AAC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a5 = r->a[5];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t a1;

    /* 0x003AAC clr.w $ae(a4) */
    wr16(m, a4 + 0xAEu, 0u);
    /* 0x003AB0 tst.w $b2(a4) ; bne.w $3b92 */
    if (rd16(m, a4 + 0xB2u) == 0u) {
        /* 0x003AB8 tst.w $ac(a4) ; bne.b $3ac4 */
        if (rd16(m, a4 + 0xACu) == 0u)
            d0 = set_word(d0, rd16(m, a4 + 0x70u));         /* 0x003ABE */
        else
            d0 = set_word(d0, rd16(m, a4 + 0x72u));         /* 0x003AC4 */
        wr16(m, a4 + 0x98u, (uint16_t)d0);                  /* 0x003AC8 */
        d0 = set_word(d0, (uint16_t)(word_of(d0) + rd16(m, a4 + 0x76u)));
        wr16(m, a4 + 0x9Au, (uint16_t)d0);                  /* 0x003AD0 */
        d0 = set_word(d0, rd16(m, a4 + 0x74u));             /* 0x003AD4 */
        wr16(m, a4 + 0x9Eu, (uint16_t)d0);                  /* 0x003AD8 */
        d0 = set_word(d0, (uint16_t)(word_of(d0) - rd16(m, a4 + 0x78u)));
        wr16(m, a4 + 0x9Cu, (uint16_t)d0);                  /* 0x003AE0 */
        /* 0x003AE4..0x003AF8: position + camera offset */
        d0 = set_word(d0, rd16(m, a4 + 0x46u));
        d1 = set_word(d1, rd16(m, a4 + 0x4Au));
        d0 = set_word(d0, (uint16_t)(word_of(d0) + rd16(m, a4 + 0x52u)));
        d1 = set_word(d1, (uint16_t)(word_of(d1) + rd16(m, a4 + 0x54u)));
        d0 = set_word(d0, (uint16_t)(word_of(d0) - rd16(m, a5 + 0x1BC4u)));
        d1 = set_word(d1, (uint16_t)(word_of(d1) - rd16(m, a5 + 0x1BC6u)));
        /* 0x003AFC movea.l $8(a4),a1 ; shape box */
        a1 = rd32(m, a4 + 0x08u);
        r->a[1] = a1;
        d3 = set_word(d3, rd16(m, a1 + 0x00u));             /* 0x003B00 */
        d2 = set_word(d2, rd16(m, a1 + 0x02u));             /* 0x003B02 */
        d2 = set_word(d2, (uint16_t)(word_of(d2) + 1u));    /* 0x003B06 */
        d3 = set_word(d3, (uint16_t)(word_of(d3) + 1u));    /* 0x003B08 */
        d2 = set_word(d2, (uint16_t)(word_of(d2) << 4));    /* 0x003B0A lsl.w #$4,d2 */
        d3 = set_word(d3, (uint16_t)(word_of(d3) << 4));    /* 0x003B0C lsl.w #$4,d3 */
        /* 0x003B0E..0x003B2A: the visible box = 0x140 x 0xE8 grown by the size */
        wr32(m, a5 - 0x60DEu, 0x00000140u);
        wr32(m, a5 - 0x60DAu, 0x000000E8u);
        wr16(m, a5 - 0x60DEu, (uint16_t)(rd16(m, a5 - 0x60DEu) - word_of(d2)));
        wr16(m, a5 - 0x60DAu, (uint16_t)(rd16(m, a5 - 0x60DAu) - word_of(d3)));
        wr16(m, a5 - 0x60DCu, (uint16_t)(rd16(m, a5 - 0x60DCu) + word_of(d2)));
        wr16(m, a5 - 0x60D8u, (uint16_t)(rd16(m, a5 - 0x60D8u) + word_of(d3)));
        /* 0x003B2E cmp2.w -$60de(a5),d0 ; bcs.w $3b8c (signed bounds) */
        if ((int16_t)word_of(d0) >= (int16_t)rd16(m, a5 - 0x60DEu)
            && (int16_t)word_of(d0) <= (int16_t)rd16(m, a5 - 0x60DCu)) {
            /* 0x003B38 cmp2.w -$60da(a5),d1 ; bcs.w $3b8c */
            if ((int16_t)word_of(d1) >= (int16_t)rd16(m, a5 - 0x60DAu)
                && (int16_t)word_of(d1) <= (int16_t)rd16(m, a5 - 0x60D8u)) {
                uint32_t a2;
                /* 0x003B42..0x003B4A: centre the box */
                d2 = set_word(d2, (uint16_t)(word_of(d2) + 1u));
                d3 = set_word(d3, (uint16_t)(word_of(d3) + 1u));
                d2 = set_word(d2, (uint16_t)(word_of(d2) >> 1));
                d0 = set_word(d0, (uint16_t)(word_of(d0) - word_of(d2)));
                d1 = set_word(d1, (uint16_t)(word_of(d1) - word_of(d3)));
                /* 0x003B4C move.w $4e(a4),d2 ; andi.w #$7,d2 */
                d2 = set_word(d2, rd16(m, a4 + 0x4Eu));
                d2 = set_word(d2, (uint16_t)(word_of(d2) & 0x0007u));
                /* 0x003B54 lea.l $1cf0(a5),a2 ; lea.l (a2,d2.w*2),a2 ; tst.w (a2) */
                a2 = a5 + 0x1CF0u + (uint32_t)(int32_t)(int16_t)word_of(d2) * 2u;
                if (rd16(m, a2) != 0u) {
                    /* 0x003B60 subq.w #$1,(a2) */
                    wr16(m, a2, (uint16_t)(rd16(m, a2) - 1u));
                    /* 0x003B62 lea.l $1d00(a5),a2 ; lea.l (a2,d2.w*4),a2 ;
                     * move.l a2,d2 ; movea.l (a2),a2 */
                    a2 = a5 + 0x1D00u + (uint32_t)(int32_t)(int16_t)word_of(d2) * 4u;
                    d2 = a2;
                    a2 = rd32(m, a2);
                    /* 0x003B6E..0x003B86: push the display record, last pushed first */
                    wr16(m, a2 - 2u, rd16(m, a4 + 0xCEu));  a2 -= 2u;
                    wr16(m, a2 - 2u, rd16(m, a4 + 0xACu));  a2 -= 2u;
                    wr16(m, a2 - 2u, rd16(m, a4 + 0x96u));  a2 -= 2u;
                    wr16(m, a2 - 2u, (uint16_t)d1);         a2 -= 2u;
                    wr16(m, a2 - 2u, (uint16_t)d0);         a2 -= 2u;
                    wr16(m, a2 - 2u, rd16(m, a4 + 0xD0u));  a2 -= 2u;
                    wr16(m, a2 - 2u, rd16(m, a4 + 0x84u));  a2 -= 2u;
                    wr32(m, a2 - 4u, a1);                   a2 -= 4u;
                    r->a[2] = a2;
                    /* 0x003B88 move.l a2,(d2.l) */
                    wr32(m, d2, a2);
                }
            }
        }
        r->d[0] = d0;
        r->d[1] = d1;
        r->d[2] = d2;
        r->d[3] = d3;
        /* 0x003B8C jsr $4856.l: it leaves its own D0/D1 behind, so nothing is
         * written back over them here. */
        push_long(m, r, 0x3B92u);
        S_004856(m, r);
    } else {
        r->d[0] = d0;
        r->d[1] = d1;
        r->d[2] = d2;
        r->d[3] = d3;
    }
    /* 0x003B92 bra.w $3a72: lea.l $3a7a(pc),a6 ; move.l a6,(a4) ; rts */
    r->a[6] = 0x3A7Au;
    wr32(m, a4, 0x00003A7Au);
    return 0;
}

/* 0x0BDE8C: tile-grid probe. D0 = x, D1 = y; returns the 4-bit tile attribute
 * of the cell (or 0 outside the grid) in D0.W. */
static void S_0BDE8C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    /* 0x0BDE8C sub.w $9a6(a5),d1 ; 0x0BDE90 asr.w #$3,d1 */
    r->d[1] = set_word(r->d[1], (uint16_t)(word_of(r->d[1]) - rd16(m, a5 + 0x9A6u)));
    r->d[1] = set_word(r->d[1], (uint16_t)((int16_t)word_of(r->d[1]) >> 3));
    /* 0x0BDE92 cmp.w $9aa(a5),d1 ; bcc.b $bdeb4 */
    if ((uint16_t)word_of(r->d[1]) >= rd16(m, a5 + 0x9AAu)) {
        r->d[0] = 0u;                                       /* 0x0BDEB4 moveq #$0,d0 */
        r->a[7] += 4u;
        return;
    }
    /* 0x0BDE98 sub.w $9a4(a5),d0 ; 0x0BDE9C asr.w #$3,d0 */
    r->d[0] = set_word(r->d[0], (uint16_t)(word_of(r->d[0]) - rd16(m, a5 + 0x9A4u)));
    r->d[0] = set_word(r->d[0], (uint16_t)((int16_t)word_of(r->d[0]) >> 3));
    /* 0x0BDE9E cmp.w $9a8(a5),d0 ; bcc.b $bdeb4 */
    if ((uint16_t)word_of(r->d[0]) >= rd16(m, a5 + 0x9A8u)) {
        r->d[0] = 0u;
        r->a[7] += 4u;
        return;
    }
    /* 0x0BDEA4 ext.l d0 ; lsl.l #$2,d0 ; 0x0BDEA8 movea.l ([$9a0,a5],d1.w*4),a0
     * (memory indirect: the row pointer comes from the table at $9a0(A5)) */
    {
        uint32_t row = rd32(m, rd32(m, a5 + 0x09A0u)
                              + (uint32_t)(int32_t)(int16_t)word_of(r->d[1]) * 4u);
        r->a[0] = row;
        /* 0x0BDEAE bfextu (a0){0:4},d0 */
        r->d[0] = (uint32_t)(rd8(m, row) >> 4);
    }
    r->a[7] += 4u;                                          /* 0x0BDEB2 rts */
}

/* 0x150B26: on-screen test of the object against the box at 0x150B52/0x150B56
 * (two cmp2.w bound pairs); D0 = 0 when inside, 0xFF when outside. */
static void S_150B26(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a5 = r->a[5];
    /* 0x150B26 move.w $46(a4),d0 ; sub.w $1bc4(a5),d0 */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, a4 + 0x46u) - rd16(m, a5 + 0x1BC4u)));
    /* 0x150B2E cmp2.w (0x150b52,pc),d0 ; bcs.w $150b4e: signed bounds at 0x150B52 */
    if ((int16_t)word_of(r->d[0]) < (int16_t)rd16(m, 0x150B52u)
        || (int16_t)word_of(r->d[0]) > (int16_t)rd16(m, 0x150B54u)) {
        r->d[0] = 0xFFFFFFFFu;                              /* 0x150B4E moveq #$ff,d0 */
        r->a[7] += 4u;
        return;
    }
    /* 0x150B38 move.w $4a(a4),d0 ; sub.w $1bc6(a5),d0 */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, a4 + 0x4Au) - rd16(m, a5 + 0x1BC6u)));
    /* 0x150B40 cmp2.w (0x150b56,pc),d0 ; bcs.w $150b4e */
    if ((int16_t)word_of(r->d[0]) < (int16_t)rd16(m, 0x150B56u)
        || (int16_t)word_of(r->d[0]) > (int16_t)rd16(m, 0x150B58u)) {
        r->d[0] = 0xFFFFFFFFu;                              /* 0x150B4E */
        r->a[7] += 4u;
        return;
    }
    r->d[0] = 0u;                                           /* 0x150B4A moveq #$0,d0 */
    r->a[7] += 4u;                                          /* 0x150B4C rts */
}

/* 0x150B5A: controller-flag handler. When the controller record has a live
 * state word it spawns 0x159/0x15A (by bit 1 of $10) and queues a sound, then
 * clears $96(A4); with no state word it just returns. */
static int S_150B5A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    /* 0x150B5A movea.l $4(a4),a0 */
    uint32_t a0 = rd32(m, r->a[4] + 0x04u);
    int rc = 0;
    r->a[0] = a0;
    /* 0x150B5E tst.w $10(a0) ; beq.w $150c7a */
    if (rd16(m, a0 + 0x10u) == 0u)
        goto L150C7A;
    {
        /* 0x150B66 btst.b #$0,$11(a0) ; bne.w $150c08 */
        if ((rd8(m, a0 + 0x11u) & 0x01u) == 0u) {
            /* 0x150B70 btst.b #$7,$10(a0) ; beq.w $150c08 */
            if ((rd8(m, a0 + 0x10u) & 0x80u) != 0u) {
                /* 0x150B7A btst.b #$1,$10(a0) ; beq.b $150b9e */
                if ((rd8(m, a0 + 0x10u) & 0x02u) != 0u) {
                    /* 0x150B84 move.w $14(a0),-$60c6(a5) ; 0x150B8A move.w $16(a0),-$60c4(a5) */
                    wr16(m, a5 - 0x60C6u, rd16(m, a0 + 0x14u));
                    wr16(m, a5 - 0x60C4u, rd16(m, a0 + 0x16u));
                    wr16(m, a5 - 0x60C2u, 0x0159u);         /* 0x150B90 */
                    push_long(m, r, 0x150B9Cu);             /* 0x150B96 jsr $393c.l */
                    rc = S_00393C(m, r);
                } else {
                    /* 0x150B9E move.w $14(a0),-$60c6(a5) ; 0x150BA4 move.w $16(a0),-$60c4(a5) */
                    wr16(m, a5 - 0x60C6u, rd16(m, a0 + 0x14u));
                    wr16(m, a5 - 0x60C4u, rd16(m, a0 + 0x16u));
                    wr16(m, a5 - 0x60C2u, 0x015Au);         /* 0x150BAA */
                    push_long(m, r, 0x150BB6u);             /* 0x150BB0 jsr $393c.l */
                    rc = S_00393C(m, r);
                }
                /* 0x150BB6 movem.l d0-d1/a0,-(a7): stores A0,D1,D0 */
                push_long(m, r, r->a[0]);
                push_long(m, r, r->d[1]);
                push_long(m, r, r->d[0]);
                push_long(m, r, 0x150BBEu);                 /* 0x150BBA jsr $150b26(pc) */
                S_150B26(m, r);
                /* 0x150BBE bne.w $150bfe */
                if (r->d[0] == 0u) {
                    /* 0x150BC2 move.w $4a(a4),d1 ; 0x150BC6 move.w $46(a4),d0 */
                    r->d[1] = set_word(r->d[1], rd16(m, r->a[4] + 0x4Au));
                    r->d[0] = set_word(r->d[0], rd16(m, r->a[4] + 0x46u));
                    push_long(m, r, 0x150BD0u);             /* 0x150BCA jsr $bde8c.l */
                    S_0BDE8C(m, r);
                    /* 0x150BD0 move.w #$3,d1 ; 0x150BD4 lea.l $150bee(pc),a0 */
                    r->d[1] = set_word(r->d[1], 0x0003u);
                    r->a[0] = 0x150BEEu;
                    /* 0x150BDA move.w (a0,d0.w*2),d0 ; 0x150BDE add.w d0,d1 */
                    r->d[0] = set_word(r->d[0],
                                       rd16(m, 0x150BEEu + (uint32_t)(int32_t)(int16_t)word_of(r->d[0]) * 2u));
                    r->d[1] = set_word(r->d[1], (uint16_t)(word_of(r->d[1]) + word_of(r->d[0])));
                    /* 0x150BE0 move.w d1,-(a7) ; 0x150BE2 jsr $2f3a.l */
                    push_word(m, r, (uint16_t)r->d[1]);
                    push_long(m, r, 0x150BE8u);
                    {
                        int rcs = S_002F3A(m, r);
                        if (rcs != 0)
                            rc = rcs;
                    }
                    r->a[7] += 2u;                          /* 0x150BE8 lea.l $2(a7),a7 */
                }
                /* 0x150BFE tst.w #$3 (flags only), then 0x150C02 pop */
                /* 0x150C02 movem.l (a7)+,d0-d1/a0 */
                r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
                r->d[1] = rd32(m, r->a[7]); r->a[7] += 4u;
                r->a[0] = rd32(m, r->a[7]); r->a[7] += 4u;
                goto L150C66;                               /* 0x150C06 bra.b $150c66 */
            }
        }
        /* 0x150C08 tst.w $3786(a5) ; 0x150C0C bra.w $150c66 */
        (void)rd16(m, a5 + 0x3786u);
        goto L150C66;
    }
L150C66:
    /* 0x150C66 move.w #$0,$96(a4) */
    wr16(m, r->a[4] + 0x96u, 0u);
    /* 0x150C6C tst.w $56(a4) ; beq.w $150c7a */
    if (rd16(m, r->a[4] + 0x56u) != 0u)
        wr16(m, r->a[0] + 0x10u, 0u);                       /* 0x150C74 move.w #$0,$10(a0) */
L150C7A:
    /* 0x150C7A rts */
    r->a[7] += 4u;
    return rc;
}

/* 0x15696A: fill the 0x3D0-byte 0x4968(A5) block with 0xBB and clear the tile
 * counters next to it. */
static void S_15696A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t a0 = a5 + 0x4968u;                             /* 0x15696A lea.l $4968(a5),a0 */
    /* 0x15696E move.w #$3c,d0 ; 0x156972..0x15698A dbra d0 */
    r->d[0] = set_word(r->d[0], 0x003Cu);
    for (int i = 0; i <= 0x3C; i++) {
        wr32(m, a0, 0xBBBBBBBBu); a0 += 4u;                 /* move.l #$bbbbbbbb,(a0)+ x4 */
        wr32(m, a0, 0xBBBBBBBBu); a0 += 4u;
        wr32(m, a0, 0xBBBBBBBBu); a0 += 4u;
        wr32(m, a0, 0xBBBBBBBBu); a0 += 4u;
    }
    r->a[0] = a0;
    r->d[0] = set_word(r->d[0], (uint16_t)((uint16_t)r->d[0] - 1u));   /* dbra ends on -1 */
    wr16(m, a5 + 0x4DB8u, 0u);                              /* 0x15698E */
    wr16(m, a5 + 0x4DBAu, 0u);                              /* 0x156994 */
    wr16(m, a5 + 0x3792u, 0u);                              /* 0x15699A */
    wr16(m, a5 + 0x4DBCu, 0u);                              /* 0x1569A0 */
    r->a[7] += 4u;                                          /* 0x1569A6 rts */
}

/* 0x15073C: drop the callback record pointed at by $d6(A4). */
static void S_15073C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    /* 0x15073C movem.l d0-d2/a0-a1,-(a7): stores A1,A0,D2,D1,D0 */
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);
    /* 0x150740 tst.l $d6(a4) ; beq.w $15074e */
    if (rd32(m, a4 + 0xD6u) != 0u) {
        /* 0x150748 movea.l $d6(a4),a0 ; 0x15074C clr.l (a0) */
        r->a[0] = rd32(m, a4 + 0xD6u);
        wr32(m, r->a[0], 0u);
    }
    /* 0x15074E movem.l (a7)+,d0-d2/a0-a1 */
    r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->d[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->d[2] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[7] += 4u;                                          /* 0x150752 rts */
}

/* 0x14FA3C: walk the child chain of $38(A4) and tick each child that has a
 * live script (bit 14 of $5E) and an entry; also releases the $40(A4) hook. */
static int S_14FA3C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    int rc = 0;
    uint32_t a0;

    /* 0x14FA3C tst.l $40(a4) ; beq.w $14fa4a */
    if (rd32(m, a4 + 0x40u) != 0u) {
        push_long(m, r, 0x14FA48u);                         /* 0x14FA44 jsr $15073c(pc) */
        S_15073C(m, r);
    }
    for (;;) {
        /* 0x14FA4A movea.l $38(a4),a0 ; tst.l a0 ; beq.w $14fa84 */
        a0 = rd32(m, a4 + 0x38u);
        r->a[0] = a0;
        if (a0 == 0u)
            break;
        /* 0x14FA54 cmpa.l $3c(a0),a4 ; bne.w $14fa84 */
        if (rd32(m, a0 + 0x3Cu) != a4)
            break;
        /* 0x14FA5C tst.l (a0) ; beq.w $14fa84 */
        if (rd32(m, a0) == 0u)
            break;
        /* 0x14FA62 bftst $5e(a0){14:1} ; beq.w $14fa84 */
        if ((rd16(m, a0 + 0x5Eu) & 0x0002u) == 0u)
            break;
        /* 0x14FA6C movem.l d0/a0-a1,-(a7): stores A1,A0,D0 ; 0x14FA70 move.l a0,-(a7) */
        push_long(m, r, r->a[1]);
        push_long(m, r, r->a[0]);
        push_long(m, r, r->d[0]);
        push_long(m, r, a0);
        push_long(m, r, 0x14FA76u);                         /* 0x14FA72 jsr $14f41e(pc) */
        {
            int rcs = S_14F41E(m, r);
            if (rcs != 0)
                rc = rcs;
        }
        r->a[7] += 4u;                                      /* 0x14FA76 lea.l $4(a7),a7 */
        /* 0x14FA7A movem.l (a7)+,d0/a0-a1 */
        r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
        r->a[0] = rd32(m, r->a[7]); r->a[7] += 4u;
        r->a[1] = rd32(m, r->a[7]); r->a[7] += 4u;
        /* 0x14FA7E movea.l $38(a0),a0 ; 0x14FA82 bra.b $14fa4e */
    }
    /* 0x14FA84 rts */
    r->a[7] += 4u;
    return rc;
}

/* 0x14F41E: call 0x14FA3C on the object passed on the stack, then retire it
 * (0x3B96 clears its long, 0x39F0 unlinks and frees it). */
static int S_14F41E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a6 = r->a[6];
    int rc = 0;
    /* 0x14F41E link.w a6,#$0 ; 0x14F422 move.l a4,-(a7) */
    push_long(m, r, a6);
    r->a[6] = r->a[7];
    push_long(m, r, r->a[4]);
    /* 0x14F424 movea.l $8(a6),a4 */
    r->a[4] = rd32(m, r->a[6] + 0x08u);
    /* 0x14F428 jsr $14fa3c(pc) */
    push_long(m, r, 0x14F42Eu);
    {
        int rcs = S_14FA3C(m, r);
        if (rcs != 0)
            rc = rcs;
    }
    /* 0x14F42E jsr $3b96.l */
    push_long(m, r, 0x14F434u);
    (void)Ea_003B96(m, r);
    /* 0x14F434 jsr $39f0.l */
    push_long(m, r, 0x14F43Au);
    S_0039F0(m, r);
    /* 0x14F43A movea.l (a7)+,a4 */
    r->a[4] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    /* 0x14F43C unlk a6 */
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] = r->a[6] + 4u;
    r->a[7] += 4u;                                          /* 0x14F43E rts */
    return rc;
}

/* 0x14F392: mark the object visible when its box intersects the screen. */
static void S_14F392(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a5 = r->a[5];
    uint32_t d0, d1 = r->d[1], a0;

    /* 0x14F392 move.l $8(a4),d0 ; ble.w $14f3f6 */
    d0 = rd32(m, a4 + 0x08u);
    r->d[0] = d0;
    if ((int32_t)d0 <= 0)
        goto L14F3F6;
    /* 0x14F39A movea.l d0,a0 ; 0x14F39C move.w $4a(a4),d0 ; sub.w $1bc6(a5),d0 */
    a0 = d0;
    r->a[0] = a0;
    d0 = set_word(d0, (uint16_t)(rd16(m, a4 + 0x4Au) - rd16(m, a5 + 0x1BC6u)));
    /* 0x14F3A4 move.w (a0),d1 ; addq.w #$1,d1 ; asl.w #$4,d1 */
    d1 = set_word(r->d[1], (uint16_t)(rd16(m, a0) + 1u));
    d1 = set_word(d1, (uint16_t)(word_of(d1) << 4));
    /* 0x14F3AA cmpi.w #$ff18,d0 ; blt.w $14f3f0 */
    if ((int16_t)word_of(d0) < (int16_t)0xFF18)
        goto L14F3F0;
    /* 0x14F3B2 add.w d1,d0 ; 0x14F3B4 cmpi.w #$1d0,d0 ; bgt.w $14f3f0 */
    d0 = set_word(d0, (uint16_t)(word_of(d0) + word_of(d1)));
    if ((int16_t)word_of(d0) > (int16_t)0x01D0)
        goto L14F3F0;
    /* 0x14F3BC move.w $46(a4),d0 ; sub.w $1bc4(a5),d0 */
    d0 = set_word(d0, (uint16_t)(rd16(m, a4 + 0x46u) - rd16(m, a5 + 0x1BC4u)));
    /* 0x14F3C4 move.w $2(a0),d1 ; addq.w #$1,d1 ; asl.w #$3,d1 */
    d1 = set_word(d1, (uint16_t)(rd16(m, a0 + 0x02u) + 1u));
    d1 = set_word(d1, (uint16_t)(word_of(d1) << 3));
    /* 0x14F3CC sub.w d1,d0 ; 0x14F3CE cmpi.w #$ff18,d0 ; blt.w $14f3f0 */
    d0 = set_word(d0, (uint16_t)(word_of(d0) - word_of(d1)));
    if ((int16_t)word_of(d0) < (int16_t)0xFF18)
        goto L14F3F0;
    /* 0x14F3D6 add.w d1,d0 ; 0x14F3D8 add.w d1,d0 ; cmpi.w #$228,d0 ; bgt.w $14f3f0 */
    d0 = set_word(d0, (uint16_t)(word_of(d0) + word_of(d1)));
    d0 = set_word(d0, (uint16_t)(word_of(d0) + word_of(d1)));
    if ((int16_t)word_of(d0) > (int16_t)0x0228)
        goto L14F3F0;
    /* 0x14F3E2 bfclr $5e(a4){1:1} ; 0x14F3E8 bfset $5e(a4){2:1} */
    wr16(m, a4 + 0x5Eu, (uint16_t)(rd16(m, a4 + 0x5Eu) & 0xBFFFu));
    wr16(m, a4 + 0x5Eu, (uint16_t)(rd16(m, a4 + 0x5Eu) | 0x2000u));
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[7] += 4u;
    return;
L14F3F0:
    /* 0x14F3F0 bfset $5e(a4){1:1} */
    wr16(m, a4 + 0x5Eu, (uint16_t)(rd16(m, a4 + 0x5Eu) | 0x4000u));
L14F3F6:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[7] += 4u;                                          /* 0x14F3F6 rts */
}

/* 0x14F3F8: force the object visible when it is in front of the camera. */
static void S_14F3F8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a5 = r->a[5];
    /* 0x14F3F8 tst.b $1bd9(a5) ; bne.w $14f41c */
    if (rd8(m, a5 + 0x1BD9u) == 0u) {
        /* 0x14F400 move.w $d2(a4),d0 ; addi.w #$200,d0 ; cmp.w $1bac(a5),d0 ; bge.w $14f41c */
        r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, a4 + 0xD2u) + 0x0200u));
        if ((int16_t)word_of(r->d[0]) < (int16_t)rd16(m, a5 + 0x1BACu)) {
            /* 0x14F410 bfset $5e(a4){1:1} ; 0x14F416 bfset $5e(a4){2:1} */
            wr16(m, a4 + 0x5Eu, (uint16_t)(rd16(m, a4 + 0x5Eu) | 0x4000u));
            wr16(m, a4 + 0x5Eu, (uint16_t)(rd16(m, a4 + 0x5Eu) | 0x2000u));
        }
    }
    r->a[7] += 4u;                                          /* 0x14F41C rts */
}

/* 0x14F2E0: per-state update hooks (visibility, tiles) of the player object;
 * runs the 0x14FA3C child/retire sweep when bits 1 and 2 of $5E are set. */
static int S_14F2E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    int rc = 0;

    /* 0x14F2E0 bsr.w $14f392 */
    push_long(m, r, 0x14F2E4u);
    S_14F392(m, r);
    /* 0x14F2E4 bsr.w $14f3f8 */
    push_long(m, r, 0x14F2E8u);
    S_14F3F8(m, r);
    /* 0x14F2E8 bftst $5e(a4){1:1} ; beq.b $14f314 */
    if ((rd16(m, a4 + 0x5Eu) & 0x4000u) != 0u) {
        /* 0x14F2F0 bftst $5e(a4){2:1} ; beq.w $14f314 */
        if ((rd16(m, a4 + 0x5Eu) & 0x2000u) != 0u) {
            /* 0x14F2FA bsr.w $14fa3c */
            push_long(m, r, 0x14F2FEu);
            rc = S_14FA3C(m, r);
            /* 0x14F2FE move.l #$3b96,$c(a4) */
            wr32(m, a4 + 0x0Cu, 0x00003B96u);
            /* 0x14F306 lea.l $34(a4),a0 ; 0x14F30A move.l a0,$20(a4) */
            r->a[0] = a4 + 0x34u;
            wr32(m, a4 + 0x20u, a4 + 0x34u);
            /* 0x14F30E move.w #$1,$44(a4) */
            wr16(m, a4 + 0x44u, 0x0001u);
        }
    }
    r->a[7] += 4u;                                          /* 0x14F314 rts */
    return rc;
}

/* 0x15039A: false when the tile at the object's x column is a solid/wall tile
 * (0xBBBB or 0x0B0B) and the 0x3790(A5) tile map is live; D0 = 0 / 0xFF. */
static int S_15039A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t a0, idx;

    /* 0x15039A tst.w $3790(a5) ; bne.w $1503be */
    if (rd16(m, a5 + 0x3790u) != 0u) {
        r->d[0] = 0xFFFFFFFFu;                              /* 0x1503BE moveq #$ff,d0 */
        r->a[7] += 4u;
        return -1;
    }
    /* 0x1503A2 lea.l $4a68(a5),a0 ; 0x1503A6 sub.w $1bc6(a5),d0 */
    a0 = a5 + 0x4A68u;
    r->a[0] = a0;
    r->d[0] = set_word(r->d[0], (uint16_t)(word_of(r->d[0]) - rd16(m, a5 + 0x1BC6u)));
    idx = a0 + (uint32_t)(int32_t)(int16_t)word_of(r->d[0]) * 2u;
    /* 0x1503AA cmpi.w #$bbbb,(a0,d0.w*2) ; beq.w $1503c2
     * 0x1503B4 cmpi.w #$b0b,(a0,d0.w*2) ; beq.w $1503c2 */
    if (rd16(m, idx) == 0xBBBBu || rd16(m, idx) == 0x0B0Bu) {
        r->d[0] = 0u;                                       /* 0x1503C2 moveq #$0,d0 */
        r->a[7] += 4u;                                      /* 0x1503C4 rts */
        return 0;
    }
    r->d[0] = 0xFFFFFFFFu;                                  /* 0x1503BE moveq #$ff,d0 */
    r->a[7] += 4u;                                          /* 0x1503C0 rts */
    return -1;
}

/* 0x15033C: add the object's tile-dependent weight to the 0x62 slot of the
 * palette/effect block at -$842(A5) (or $9a(A5) for the bit-3 variant). */
static int S_15033C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a5 = r->a[5];
    uint32_t a0, d1;
    int rc = 0;

    /* 0x15033C movem.l d0/a1,-(a7): stores A1,D0 */
    push_long(m, r, r->a[1]);
    push_long(m, r, r->d[0]);
    /* 0x150340 movea.l $4(a4),a0 ; 0x150344 lea.l -$842(a5),a1 */
    a0 = rd32(m, a4 + 0x04u);
    r->a[0] = a0;
    r->a[1] = a5 - 0x842u;
    /* 0x150348 btst.b #$5,$10(a0) ; bne.b $150394 */
    if ((rd8(m, a0 + 0x10u) & 0x20u) == 0u) {
        /* 0x150350 btst.b #$3,$10(a0) ; beq.b $15035c */
        if ((rd8(m, a0 + 0x10u) & 0x08u) != 0u)
            r->a[1] = a5 + 0x009Au;                         /* 0x150358 lea.l $9a(a5),a1 */
        /* 0x15035C lea.l $783c.l,a0 ; 0x150362 moveq #$0,d0 */
        a0 = 0x0000783Cu;
        r->a[0] = a0;
        r->d[0] = 0u;
        /* 0x150364 move.w $5a(a4),d0 ; 0x150368 subi.w #$101,d0 */
        r->d[0] = set_word(r->d[0], rd16(m, a4 + 0x5Au));
        r->d[0] = set_word(r->d[0], (uint16_t)(word_of(r->d[0]) - 0x0101u));
        /* 0x15036C move.l (a0,d0.w*4),d1 */
        d1 = rd32(m, a0 + (uint32_t)(int32_t)(int16_t)word_of(r->d[0]) * 4u);
        /* 0x150370 move.w $4a(a4),d0 */
        r->d[0] = set_word(r->d[0], rd16(m, a4 + 0x4Au));
        /* 0x150374 movem.l a0,-(a7) ; 0x150378 bsr.w $15039a ; 0x15037C movem.l (a7)+,a0 */
        push_long(m, r, r->a[0]);
        push_long(m, r, 0x15037Cu);
        {
            int solid = S_15039A(m, r);
            r->a[0] = rd32(m, r->a[7]);
            r->a[7] += 4u;
            /* 0x150380 beq.w $150386 */
            if (solid != 0)
                d1 = d1 + d1;                               /* 0x150384 asl.l #$1,d1 */
        }
        /* 0x150386 btst.b #$7,$10(a0) ; beq.b $150390 */
        if ((rd8(m, r->a[0] + 0x10u) & 0x80u) != 0u)
            d1 = d1 + d1;                                   /* 0x15038E asl.l #$1,d1 */
        /* 0x150390 add.l d1,$62(a1) */
        wr32(m, r->a[1] + 0x62u, rd32(m, r->a[1] + 0x62u) + d1);
        r->d[1] = d1;
    }
    /* 0x150394 movem.l (a7)+,d0/a1 */
    r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[7] += 4u;                                          /* 0x150398 rts */
    return rc;
}

/* 0x0039F0: unlink the object in A4 from the -$60E2(A5) list, hand its
 * sub-record back and push the slot onto the -$60EE(A5) free list. */
static void S_0039F0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t a4 = r->a[4];
    uint32_t d0, a0;

    /* 0x0039F0 movem.l d1,-(a7) */
    push_long(m, r, r->d[1]);
    /* 0x0039F4 move.l $34(a4),d0 ; bne.b $3a0a */
    d0 = rd32(m, a4 + 0x34u);
    r->d[0] = d0;
    if (d0 == 0u) {
        /* 0x0039FA move.l $38(a4),d0 ; 0x0039FE move.l d0,-$60e6(a5) ; bne.b $3a12 */
        d0 = rd32(m, a4 + 0x38u);
        r->d[0] = d0;
        wr32(m, a5 - 0x60E6u, d0);
        if (d0 == 0u)
            wr32(m, a5 - 0x60E2u, 0u);                      /* 0x003A04 clr.l -$60e2(a5) */
    } else {
        /* 0x003A0A movea.l d0,a0 ; 0x003A0C move.l $38(a4),$38(a0) */
        a0 = d0;
        r->a[0] = a0;
        wr32(m, a0 + 0x38u, rd32(m, a4 + 0x38u));
    }
    /* 0x003A12 move.l $38(a4),d0 ; beq.b $3a22 */
    d0 = rd32(m, a4 + 0x38u);
    r->d[0] = d0;
    if (d0 != 0u) {
        /* 0x003A18 movea.l d0,a0 ; 0x003A1A move.l $34(a4),$34(a0) */
        a0 = d0;
        r->a[0] = a0;
        wr32(m, a0 + 0x34u, rd32(m, a4 + 0x34u));
    } else {
        /* 0x003A22 move.l $34(a4),-$60e2(a5) */
        wr32(m, a5 - 0x60E2u, rd32(m, a4 + 0x34u));
    }
    /* 0x003A28 movea.l -$60ee(a5),a0 ; 0x003A2C move.l a4,-(a0) ; 0x003A2E move.l a0,-$60ee(a5) */
    a0 = rd32(m, a5 - 0x60EEu);
    wr32(m, a0 - 4u, a4);
    wr32(m, a5 - 0x60EEu, a0 - 4u);
    r->a[0] = a0 - 4u;
    /* 0x003A32 tst.l $4(a4) ; beq.w $3a40 */
    if (rd32(m, a4 + 0x04u) != 0u) {
        push_long(m, r, 0x3A40u);                           /* 0x003A3A jsr $a6954.l */
        S_A6954(m, r);
    }
    /* 0x003A40 subq.w #$1,-$60f0(a5) */
    wr16(m, a5 - 0x60F0u, (uint16_t)(rd16(m, a5 - 0x60F0u) - 1u));
    /* 0x003A44 movem.l (a7)+,d1 */
    r->d[1] = rd32(m, r->a[7]); r->a[7] += 4u;
    r->a[7] += 4u;                                          /* 0x003A48 rts */
}

int Ea_15655C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t a5 = r->a[5];
    const uint32_t entry_sp = r->a[7];
    int rc = 0;
    int zflag;      /* the Z flag the 0x1565AE bne reads */

    /* 0x15655C tst.w $96(a4) ; beq.w $1565ae */
    if (rd16(m, a4 + 0x96u) == 0u) {
        zflag = 1;
        goto L1565AE;
    }
    /* 0x156564 movea.l $4(a4),a0 ; tst.l a0 ; beq.w $1565ae */
    r->a[0] = rd32(m, a4 + 0x04u);
    if (r->a[0] == 0u) {
        zflag = 1;
        goto L1565AE;
    }
    /* 0x15656E tst.w $10(a0) ; beq.w $1565ae */
    if (rd16(m, r->a[0] + 0x10u) == 0u) {
        zflag = 1;
        goto L1565AE;
    }
    /* 0x156576 btst.b #$4,$10(a0) ; bne.w $156592 */
    if ((rd8(m, r->a[0] + 0x10u) & 0x10u) != 0u)
        goto L156592;
    /* 0x156580 btst.b #$6,$10(a0) ; bne.w $156592 */
    if ((rd8(m, r->a[0] + 0x10u) & 0x40u) != 0u)
        goto L156592;
    /* 0x15658A subq.w #$1,$56(a4) ; 0x15658E moveq #$0,d0 ; 0x156590 bra.b */
    wr16(m, a4 + 0x56u, (uint16_t)(rd16(m, a4 + 0x56u) - 1u));
    r->d[0] = 0u;
    zflag = 1;
    goto L1565AE;
L156592:
    /* 0x156592 move.l #$156726,$1c(a4) */
    wr32(m, a4 + 0x1Cu, 0x00156726u);
    /* 0x15659A lea.l $34(a4),a0 ; 0x15659E move.l a0,$20(a4) */
    r->a[0] = a4 + 0x34u;
    wr32(m, a4 + 0x20u, a4 + 0x34u);
    /* 0x1565A2 clr.w $44(a4) ; 0x1565A6 move.w #$0,$56(a4) */
    wr16(m, a4 + 0x44u, 0u);
    wr16(m, a4 + 0x56u, 0u);
    r->d[0] = 1u;                                           /* 0x1565AC moveq #$1,d0 */
    zflag = 0;
L1565AE:
    /* 0x1565AE bne.w $1565b8 */
    if (zflag != 0) {
        /* 0x1565B2 jsr $150b5a.l */
        push_long(m, r, 0x1565B8u);
        {
            int rcs = S_150B5A(m, r);
            if (rcs != 0)
                rc = rcs;
        }
    }
    /* 0x1565B8 tst.w $56(a4) ; bne.w $1566d0 */
    if (rd16(m, a4 + 0x56u) != 0u)
        goto L1566D0;
    {
        uint32_t d1, d2, d3, a1;
        static const uint16_t ids[5] = { 0x0127u, 0x012Bu, 0x012Fu, 0x012Au, 0x012Eu };
        static const uint32_t rets[5] = { 0x1565E8u, 0x1565FCu, 0x156610u, 0x156624u, 0x156638u };
        /* 0x1565C0 move.w $46(a4),d2 ; 0x1565C4 move.w $4a(a4),d1 */
        d2 = set_word(r->d[2], rd16(m, a4 + 0x46u));
        d1 = set_word(r->d[1], rd16(m, a4 + 0x4Au));
        /* 0x1565C8 movea.l $8(a4),a1 ; 0x1565CC move.w (a1),d3 ;
         * 0x1565CE addq.w #$1,d3 ; 0x1565D0 lsl.w #$2,d3 ; 0x1565D2 sub.w d3,d1 */
        a1 = rd32(m, a4 + 0x08u);
        r->a[1] = a1;
        d3 = set_word(r->d[3], (uint16_t)(rd16(m, a1) + 1u));
        d3 = set_word(d3, (uint16_t)(word_of(d3) << 2));
        d1 = set_word(d1, (uint16_t)(word_of(d1) - word_of(d3)));
        r->d[1] = d1;
        r->d[2] = d2;
        r->d[3] = d3;
        for (int i = 0; i < 5; i++) {
            /* each block: move.w d2,-$60c6(a5) ; move.w d1,-$60c4(a5) ;
             * move.w #id,-$60c2(a5) ; jsr $393c.l */
            wr16(m, a5 - 0x60C6u, (uint16_t)d2);
            wr16(m, a5 - 0x60C4u, (uint16_t)d1);
            wr16(m, a5 - 0x60C2u, ids[i]);
            push_long(m, r, rets[i]);
            {
                int rcs = S_00393C(m, r);
                if (rcs != 0)
                    rc = rcs;
            }
        }
        /* 0x156638 movem.l d0-d1/a0,-(a7): stores A0,D1,D0 */
        push_long(m, r, r->a[0]);
        push_long(m, r, r->d[1]);
        push_long(m, r, r->d[0]);
        /* 0x15663C jsr $150b26.l ; 0x156642 bne.w $156682 */
        push_long(m, r, 0x156642u);
        S_150B26(m, r);
        if (r->d[0] == 0u) {
            /* 0x156646 move.w $4a(a4),d1 ; 0x15664A move.w $46(a4),d0 */
            r->d[1] = set_word(r->d[1], rd16(m, a4 + 0x4Au));
            r->d[0] = set_word(r->d[0], rd16(m, a4 + 0x46u));
            /* 0x15664E jsr $bde8c.l */
            push_long(m, r, 0x156654u);
            S_0BDE8C(m, r);
            /* 0x156654 move.w #$1b,d1 ; 0x156658 lea.l $156672(pc),a0 */
            r->d[1] = set_word(r->d[1], 0x001Bu);
            r->a[0] = 0x00156672u;
            /* 0x15665E move.w (a0,d0.w*2),d0 ; 0x156662 add.w d0,d1 */
            r->d[0] = set_word(r->d[0],
                               rd16(m, 0x00156672u
                                    + (uint32_t)(int32_t)(int16_t)word_of(r->d[0]) * 2u));
            r->d[1] = set_word(r->d[1], (uint16_t)(word_of(r->d[1]) + word_of(r->d[0])));
            /* 0x156664 move.w d1,-(a7) ; 0x156666 jsr $2f3a.l */
            push_word(m, r, (uint16_t)r->d[1]);
            push_long(m, r, 0x15666Cu);
            {
                int rcs = S_002F3A(m, r);
                if (rcs != 0)
                    rc = rcs;
            }
            r->a[7] += 2u;                                  /* 0x15666C lea.l $2(a7),a7 */
        }
        /* 0x156682 tst.w #$1b (flags only) ; 0x156686 movem.l (a7)+,d0-d1/a0 */
        r->d[0] = rd32(m, r->a[7]); r->a[7] += 4u;
        r->d[1] = rd32(m, r->a[7]); r->a[7] += 4u;
        r->a[0] = rd32(m, r->a[7]); r->a[7] += 4u;
        /* 0x15668A addq.w #$1,$3790(a5) */
        wr16(m, a5 + 0x3790u, (uint16_t)(rd16(m, a5 + 0x3790u) + 1u));
        /* 0x15668E move.l #$1568f2,$d6(a4) */
        wr32(m, a4 + 0xD6u, 0x001568F2u);
        /* 0x156696 move.w $1568f2(pc),$d4(a4) */
        wr16(m, a4 + 0xD4u, rd16(m, 0x001568F2u));
        /* 0x15669E bsr.w $15696a */
        push_long(m, r, 0x1566A2u);
        S_15696A(m, r);
        /* 0x1566A2 jsr $15033c.l */
        push_long(m, r, 0x1566A8u);
        {
            int rcs = S_15033C(m, r);
            if (rcs != 0)
                rc = rcs;
        }
        /* 0x1566A8 jsr $a6954.l */
        push_long(m, r, 0x1566AEu);
        S_A6954(m, r);
        /* 0x1566AE move.l #$0,$4(a4) */
        wr32(m, a4 + 0x04u, 0u);
        /* 0x1566B6 move.l #$156726,$1c(a4) */
        wr32(m, a4 + 0x1Cu, 0x00156726u);
        /* 0x1566BE lea.l $34(a4),a0 ; 0x1566C2 move.l a0,$20(a4) */
        r->a[0] = a4 + 0x34u;
        wr32(m, a4 + 0x20u, a4 + 0x34u);
        /* 0x1566C6 clr.w $44(a4) ; 0x1566CA jmp $3aac.l */
        wr16(m, a4 + 0x44u, 0u);
        goto L1566CA;
    }
L1566D0:
    /* 0x1566D0 jsr $14f2e0.l ; 0x1566D6 jmp $3aac.l */
    push_long(m, r, 0x1566D6u);
    {
        int rcs = S_14F2E0(m, r);
        if (rcs != 0)
            rc = rcs;
    }
L1566CA:
    /* 0x1566CA/0x1566D6 jmp $3aac.l, whose 0x3A78 rts returns to our caller */
    {
        int rcs = S_003AAC(m, r);
        if (rcs != 0)
            rc = rcs;
    }
    r->a[7] = entry_sp + 4u;                                /* the rts pops the return address */
    return rc;
}
