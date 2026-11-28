/* ROM 0x0D0EC2: per-frame horizontal step for the a5 motion state.  It folds
 * the four direction inputs into a displacement, dead-zones it against one of
 * two bound tables (0x0D0EA4 / 0x0D0EA8), and when it leaves the zone snaps
 * $1CB8 to a 0x8000-tagged position and runs one of the two segment-scan
 * helpers (0x0D1070 / 0x0D10C0) to move along the track bitmap at $1BB8. */
#include "ea_auto.h"

/* 0x0D1034: smoothing tail.  Pulls $1CBC toward $1CB8, clamps the half-delta
 * to +-0x20000 into $1CC0 and advances $1CBC.  D1 = ($1CC0_old - v) >> 2. */
static uint32_t body_0D1034(const f3mem_t *m, uint32_t *d1_io)
{
    uint32_t a5 = 0x408000u;
    uint32_t d1 = rd32(m, a5 + 0x1CC0u);
    uint32_t d0 = rd32(m, a5 + 0x1CB8u);
    int32_t v;

    d0 = d0 - rd32(m, a5 + 0x1CBCu);            /* 0x0D103C sub.l $1cbc */
    v = (int32_t)d0 >> 1;                       /* 0x0D1040 asr.l #1 */
    /* 0x0D1042 cmp2.l bounds at 0x0D1068 = [-0x20000, 0x20000]; bcc=in range */
    if (v < -0x20000 || v > 0x20000) {
        int32_t d2 = v;                         /* 0x0D104A move.l d0,d2 */
        d0 = 0x00020000u;                       /* 0x0D104C move.l #$20000 */
        if (d2 < 0)                             /* 0x0D1052 tst.l d2 ; bpl */
            d0 = 0xFFFE0000u;                   /* 0x0D1056 neg.l d0 */
    }
    wr32(m, a5 + 0x1CC0u, d0);                  /* 0x0D1058 */
    {
        int32_t dv = (int32_t)d1 - (int32_t)d0; /* 0x0D105C sub.l d0,d1 */
        dv >>= 2;                               /* 0x0D105E asr.l #2 */
        d0 = d0 + (uint32_t)dv;                 /* 0x0D1060 add.l d1,d0 */
        *d1_io = (uint32_t)dv;
    }
    wr32(m, a5 + 0x1CBCu, rd32(m, a5 + 0x1CBCu) + d0); /* 0x0D1062 */
    return d0;
}

/* 0x0D1070: walk the $1BB8 nibble bitmap forward to the next nibble change. */
static uint32_t bfextu_mem32(const f3mem_t *m, uint32_t ea, int32_t offset, uint32_t width)
{
    uint32_t data;

    ea += (uint32_t)(offset / 8);
    offset %= 8;
    if (offset < 0) {
        offset += 8;
        ea -= 1u;
    }
    data = rd32(m, ea) << (uint32_t)offset;
    if ((uint32_t)(offset + (int32_t)width) > 32u)
        data |= (rd8(m, ea + 4u) << (uint32_t)offset) >> 8;
    return data >> (32u - width);
}

static void body_0D1070(const f3mem_t *m, uint32_t *d0_io, uint32_t *d1_io, uint32_t *a0_io)
{
    uint32_t a5 = 0x408000u;
    uint32_t d0 = *d0_io;
    uint32_t d2 = rd32(m, a5 + 0x1BD2u) & 0xFFFF0000u; /* 0x0D1070/74 */
    uint32_t d1 = rd32(m, a5 + 0x1BACu);               /* 0x0D1076 */
    uint32_t a0, d3, d4;

    d1 = d1 + d0;                               /* 0x0D107A add.l d0,d1 */
    d2 = d2 - d1;                               /* 0x0D107C sub.l d1,d2 */
    if ((int32_t)d2 < 0)                        /* 0x0D107E bpl $d1082 */
        d0 = d0 + d2;                           /* 0x0D1080 add.l d2,d0 */
    a0 = rd32(m, a5 + 0x1BB8u);                 /* 0x0D1082 */
    d2 = rd32(m, a5 + 0x1BACu);                 /* 0x0D1086 */
    d3 = d2;
    d2 = d2 & 0xFFFF0000u;                      /* 0x0D108C clr.w d2 */
    d2 = (d2 << 16) | (d2 >> 16);               /* 0x0D108E swap d2 */
    d2 = d2 << 2;                               /* 0x0D1090 lsl.l #2 */
    d4 = bfextu_mem32(m, a0, (int32_t)d2, 4u);  /* 0x0D1092 bfextu {D2:4},d4 */
    d3 = d3 + d0;                               /* 0x0D1096 add.l d0,d3 */
    d3 = d3 & 0xFFFF0000u;                      /* 0x0D1098 clr.w d3 */
    d3 = (d3 << 16) | (d3 >> 16);               /* 0x0D109A swap d3 */
    d3 = d3 << 2;                               /* 0x0D109C lsl.l #2 */
    d3 = bfextu_mem32(m, a0, (int32_t)d3, 4u);  /* 0x0D109E bfextu {D3:4},d3 */
    if ((uint16_t)d4 != (uint16_t)d3) {         /* 0x0D10A2 cmp.w d4,d3 */
        do {
            d2 = d2 + 4u;                       /* 0x0D10A6 addq.l #4,d2 */
            d3 = bfextu_mem32(m, a0, (int32_t)d2, 4u);
        } while ((uint16_t)d4 == (uint16_t)d3); /* 0x0D10AE beq $d10a6 */
        d2 = d2 >> 2;                           /* 0x0D10B0 lsr.l #2 */
        d2 = (d2 << 16) | (d2 >> 16);           /* 0x0D10B2 swap d2 */
        d2 = d2 - rd32(m, a5 + 0x1BACu);        /* 0x0D10B4 sub.l $1bac */
        d0 = d2;                                /* 0x0D10B8 move.l d2,d0 */
    }
    wr32(m, a5 + 0x1BB0u, d0);                  /* 0x0D10BA */
    *d0_io = d0;
    *d1_io = d1;
    *a0_io = a0;
}

/* 0x0D10C0: mirror of 0x0D1070, walking the bitmap backwards. */
static void body_0D10C0(const f3mem_t *m, uint32_t *d0_io, uint32_t *d1_io, uint32_t *a0_io)
{
    uint32_t a5 = 0x408000u;
    uint32_t d0 = *d0_io;
    uint32_t d2 = rd32(m, a5 + 0x1BD0u) & 0xFFFF0000u; /* 0x0D10C0/44 */
    uint32_t d1 = rd32(m, a5 + 0x1BACu);               /* 0x0D10C6 */
    uint32_t a0, d3, d4;

    d1 = d1 + d0;                               /* 0x0D10CA add.l d0,d1 */
    d1 = d1 - d2;                               /* 0x0D10CC sub.l d2,d1 */
    if ((int32_t)d1 < 0)                        /* 0x0D10CE bpl $d10d2 */
        d0 = d0 - d1;                           /* 0x0D10D0 sub.l d1,d0 */
    a0 = rd32(m, a5 + 0x1BB8u);                 /* 0x0D10D2 */
    d2 = rd32(m, a5 + 0x1BACu);                 /* 0x0D10D6 */
    d3 = d2;
    d2 = d2 & 0xFFFF0000u;                      /* 0x0D10DC clr.w d2 */
    d2 = (d2 << 16) | (d2 >> 16);               /* 0x0D10DE swap d2 */
    d2 = d2 << 2;                               /* 0x0D10E0 lsl.l #2 */
    d4 = bfextu_mem32(m, a0, (int32_t)d2, 4u);  /* 0x0D10E2 */
    d3 = d3 + d0;                               /* 0x0D10E6 */
    d3 = d3 & 0xFFFF0000u;                      /* 0x0D10E8 clr.w d3 */
    d3 = (d3 << 16) | (d3 >> 16);               /* 0x0D10EA swap d3 */
    d3 = d3 << 2;                               /* 0x0D10EC */
    d3 = bfextu_mem32(m, a0, (int32_t)d3, 4u);  /* 0x0D10EE */
    if ((uint16_t)d4 != (uint16_t)d3) {         /* 0x0D10F2 cmp.w d4,d3 */
        do {
            d2 = d2 - 4u;                       /* 0x0D10F6 subq.l #4,d2 */
            d3 = bfextu_mem32(m, a0, (int32_t)d2, 4u);
        } while ((uint16_t)d4 == (uint16_t)d3); /* 0x0D10FE beq $d10f6 */
        d2 = d2 + 4u;                           /* 0x0D1100 addq.l #4,d2 */
        d2 = d2 >> 2;                           /* 0x0D1102 lsr.l #2 */
        d2 = (d2 << 16) | (d2 >> 16);           /* 0x0D1104 swap d2 */
        d2 = d2 - rd32(m, a5 + 0x1BACu);        /* 0x0D1106 sub.l $1bac */
        d0 = d2;                                /* 0x0D110A */
    }
    wr32(m, a5 + 0x1BB0u, d0);                  /* 0x0D110C */
    *d0_io = d0;
    *d1_io = d1;
    *a0_io = a0;
}

int Ea_0D0EC2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t d4 = r->d[4], d5 = r->d[5], d6 = r->d[6], d7 = r->d[7];
    uint32_t a0 = r->a[0], a1 = r->a[1];
    uint16_t w;

    /* 0x0D0EC2 move.l $1cb8(a5),$1cb4(a5) */
    wr32(m, a5 + 0x1CB4u, rd32(m, a5 + 0x1CB8u));

    /* 0x0D0EC8..0x0D0EE4 early-out tests -> $d0eac */
    if (rd16(m, a5 + 0x34FAu) != 0) goto early;
    if ((int16_t)rd16(m, a5 + 0x1BD6u) <= 0) goto early;
    if (rd32(m, a5 + 0x1BDCu) != 0) goto early;
    if (rd32(m, a5 + 0x1BB0u) != 0) goto early;
    if (rd16(m, a5 + 0x1BDAu) != 0) goto early;

    /* 0x0D0EE6 moveq #0,d0 ; 0x0D0EE8 moveq #$ff,d1 */
    d0 = 0u;
    d1 = 0xFFFFFFFFu;

    /* 0x0D0EEA tst.w -$840(a5) ; bgt ; 0x0D0EF0 tst.w -$842(a5) ; bpl */
    if ((int16_t)rd16(m, a5 - 0x840u) > 0 ||
        (int16_t)rd16(m, a5 - 0x842u) < 0) {
        /* 0x0D0EF6 */
        d0 = (d0 & 0xFFFF0000u) |
             (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 - 0x7A2u));
        d0 = (d0 & 0xFFFF0000u) |
             (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 - 0x760u));
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((d1 & 0xFFFFu) + 1u);
    }

    /* 0x0D0F00 */
    if ((int16_t)rd16(m, a5 + 0x9Cu) > 0 ||
        (int16_t)rd16(m, a5 + 0x9Au) < 0) {
        /* 0x0D0F0C */
        d0 = (d0 & 0xFFFF0000u) |
             (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 + 0x13Au));
        d0 = (d0 & 0xFFFF0000u) |
             (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 + 0x17Cu));
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((d1 & 0xFFFFu) + 1u);
    }

    /* 0x0D0F16 tst.w d1 ; bpl.b $d0f24 */
    if ((int16_t)(d1 & 0xFFFFu) < 0) {
        /* 0x0D0F1A moveq #0,d1 ; 0x0D0F1C move.w $1bc8(a5),d0 ;
         * 0x0D0F20 addi.w #$a0,d0 */
        d1 = 0u;
        d0 = (d0 & 0xFFFF0000u) | rd16(m, a5 + 0x1BC8u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((d0 & 0xFFFFu) + 0xA0u);
    }

    /* 0x0D0F24 lea.l $d0ea4(pc),a0 ; 0x0D0F28 tst.w d1 ; ble $d0f32 */
    a0 = 0x0D0EA4u;
    if ((int16_t)(d1 & 0xFFFFu) > 0) {
        /* 0x0D0F2C lea.l $d0ea8(pc),a0 ; 0x0D0F30 asr.w #1,d0 */
        a0 = 0x0D0EA8u;
        d0 = (d0 & 0xFFFF0000u) |
             (uint16_t)((int16_t)(d0 & 0xFFFFu) >> 1);
    }

    /* 0x0D0F32 sub.w $1bc8(a5),d0 ; 0x0D0F36 subi.w #$a0,d0 */
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) - rd16(m, a5 + 0x1BC8u));
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((d0 & 0xFFFFu) - 0xA0u);

    /* 0x0D0F3A cmp2.w (a0),d0 ; bcc.w $d1022  (in range -> no motion) */
    {
        int16_t cmp = (int16_t)(d0 & 0xFFFFu);
        int16_t lo = (int16_t)rd16(m, a0);
        int16_t hi = (int16_t)rd16(m, a0 + 2u);
        int inrange = (lo <= hi) ? (cmp >= lo && cmp <= hi)
                                 : (cmp > hi || cmp < lo);
        if (inrange) goto L1022;
    }

    /* 0x0D0F42 move.w $1bd4(a5),d2 ; andi.w #$8,d2 ; bne $d0fba */
    w = (uint16_t)(rd16(m, a5 + 0x1BD4u) & 0x0008u);
    d2 = (d2 & 0xFFFF0000u) | w;
    if (w != 0) goto L0FBA;

    /* 0x0D0F4E tst.w d0 ; bmi $d0f88 */
    if ((int16_t)(d0 & 0xFFFFu) < 0) goto L0F88;

    /* 0x0D0F50 cmp.w $2(a0),d0 ; ble.w $d1022 */
    if ((int16_t)(d0 & 0xFFFFu) <= (int16_t)rd16(m, a0 + 2u)) goto L1022;
    /* 0x0D0F5A tst.b $1bd9(a5) ; beq.w $d1022 */
    if (rd8(m, a5 + 0x1BD9u) == 0) goto L1022;

    /* 0x0D0F62 sub.w $2(a0),d0 ; add.w $1bc8(a5),d0 ;
     * 0x0D0F6A swap d0 ; 0x0D0F6C move.w #$8000,d0 */
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) - rd16(m, a0 + 2u));
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 + 0x1BC8u));
    d0 = (d0 << 16) | (d0 >> 16);
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;
    wr32(m, a5 + 0x1CB8u, d0);                  /* 0x0D0F70 */
    /* 0x0D0F74 cmp.l $1cb4(a5),d0 ; blt.w $d1022 */
    if ((int32_t)d0 < (int32_t)rd32(m, a5 + 0x1CB4u)) goto L1022;
    d0 = body_0D1034(m, &d1);                   /* 0x0D0F7C */
    d0 = 0u - d0;                               /* 0x0D0F80 neg.l d0 */
    body_0D10C0(m, &d0, &d1, &a0);              /* 0x0D0F82 */
    goto ret;                                   /* 0x0D0F86 rts */

L0F88:
    /* 0x0D0F88 cmp.w (a0),d0 ; bge.w $d1022 */
    if ((int16_t)(d0 & 0xFFFFu) >= (int16_t)rd16(m, a0)) goto L1022;
    /* 0x0D0F8E tst.b $1bd8(a5) ; beq.w $d1022 */
    if (rd8(m, a5 + 0x1BD8u) == 0) goto L1022;
    /* 0x0D0F96 sub.w (a0),d0 ; add.w $1bc8(a5),d0 ; swap ; move.w #$8000 */
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) - rd16(m, a0));
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 + 0x1BC8u));
    d0 = (d0 << 16) | (d0 >> 16);
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;
    wr32(m, a5 + 0x1CB8u, d0);
    /* 0x0D0FA6 cmp.l $1cb4(a5),d0 ; bgt.w $d1022 */
    if ((int32_t)d0 > (int32_t)rd32(m, a5 + 0x1CB4u)) goto L1022;
    d0 = body_0D1034(m, &d1);                   /* 0x0D0FAE */
    d0 = 0u - d0;                               /* 0x0D0FB2 neg.l d0 */
    body_0D1070(m, &d0, &d1, &a0);              /* 0x0D0FB4 */
    goto ret;                                   /* 0x0D0FB8 rts */

L0FBA:
    /* 0x0D0FBA tst.w d0 ; bmi.b $d0ff2 */
    if ((int16_t)(d0 & 0xFFFFu) < 0) goto L0FF2;
    /* 0x0D0FBE cmp.w $2(a0),d0 ; ble.w $d1022 */
    if ((int16_t)(d0 & 0xFFFFu) <= (int16_t)rd16(m, a0 + 2u)) goto L1022;
    /* 0x0D0FC6 tst.b $1bd8(a5) ; beq.w $d1022 */
    if (rd8(m, a5 + 0x1BD8u) == 0) goto L1022;
    /* 0x0D0FCE sub.w $2(a0),d0 ; add.w $1bc8 ; swap ; move.w #$8000 */
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) - rd16(m, a0 + 2u));
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 + 0x1BC8u));
    d0 = (d0 << 16) | (d0 >> 16);
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;
    wr32(m, a5 + 0x1CB8u, d0);
    /* 0x0D0FE0 cmp.l $1cb4(a5),d0 ; blt.w $d1022 */
    if ((int32_t)d0 < (int32_t)rd32(m, a5 + 0x1CB4u)) goto L1022;
    d0 = body_0D1034(m, &d1);                   /* 0x0D0FE8 */
    body_0D1070(m, &d0, &d1, &a0);              /* 0x0D0FEC */
    goto ret;                                   /* 0x0D0FF0 rts */

L0FF2:
    /* 0x0D0FF2 cmp.w (a0),d0 ; bge.w $d1022 */
    if ((int16_t)(d0 & 0xFFFFu) >= (int16_t)rd16(m, a0)) goto L1022;
    /* 0x0D0FF8 tst.b $1bd9(a5) ; beq.w $d1022 */
    if (rd8(m, a5 + 0x1BD9u) == 0) goto L1022;
    /* 0x0D1000 sub.w (a0),d0 ; add.w $1bc8 ; swap ; move.w #$8000 */
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) - rd16(m, a0));
    d0 = (d0 & 0xFFFF0000u) |
         (uint16_t)((d0 & 0xFFFFu) + rd16(m, a5 + 0x1BC8u));
    d0 = (d0 << 16) | (d0 >> 16);
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;
    wr32(m, a5 + 0x1CB8u, d0);
    /* 0x0D1010 cmp.l $1cb4(a5),d0 ; bgt.w $d1022 */
    if ((int32_t)d0 > (int32_t)rd32(m, a5 + 0x1CB4u)) goto L1022;
    d0 = body_0D1034(m, &d1);                   /* 0x0D1018 */
    body_0D10C0(m, &d0, &d1, &a0);              /* 0x0D101C */
    goto ret;                                   /* 0x0D1020 rts */

L1022:
    /* 0x0D1022 move.l $1bc8(a5),d0 ; move.w #$8000,d0 ;
     * 0x0D102A move.l d0,$1cb8(a5) ; 0x0D102E move.l $1cb8,$1cbc */
    d0 = rd32(m, a5 + 0x1BC8u);
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;
    wr32(m, a5 + 0x1CB8u, d0);
    wr32(m, a5 + 0x1CBCu, d0);
    d0 = body_0D1034(m, &d1);                   /* falls into 0x0D1034 */
    goto ret;

early:
    /* 0x0D0EAC move.l $1bc8(a5),d0 ; move.w #$8000,d0 ;
     * 0x0D0EB4 move.l d0,$1cb8 ; 0x0D0EB8 move.l d0,$1cbc ;
     * 0x0D0EBC clr.l $1cc0 ; rts */
    d0 = rd32(m, a5 + 0x1BC8u);
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;
    wr32(m, a5 + 0x1CB8u, d0);
    wr32(m, a5 + 0x1CBCu, d0);
    wr32(m, a5 + 0x1CC0u, 0u);

ret:
    r->d[0] = d0; r->d[1] = d1; r->d[2] = d2; r->d[3] = d3;
    r->d[4] = d4; r->d[5] = d5; r->d[6] = d6; r->d[7] = d7;
    r->a[0] = a0; r->a[1] = a1;
    r->a[7] = sp + 4u;
    return 0;
}
