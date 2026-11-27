/* ROM 0x13DAD4 stage end (called from 0x13F5B8 at 0x13F698), in segments
 * (src/kernel/seg.h). Translated from the ROM listing; every resume PC is the
 * routine entry or the instruction after one of its TRAP #5 sites.
 *
 *   13DAD4  stage != 0 && level != 7: jingle 0x14; st.b A5+0x1D1C
 *   13DB00  level == 7: jingle 0x31; then jingle 0x17
 *   13DB32  clear PF1 + line RAM (0x5956), clear bit 2 of the 0xE8-word tables
 *           at 0x62B400/0x626200 (0x13E8A2 and two 0x700E/0x0FF9 fills)
 *   13DB7A  palette upload 0x204C0 -> 0xA00, 0x20 entries; spawn 0x13DE92
 *   13DBB2  D7 = 5, then wait 5 frames on bit 1 of A5-0x144B (dbra style)
 *   13DBC6  seed the scroll projection 0x32EE..0x32F8 and scroll (0x13E91E)
 *   13DBF8  trap #5, trap #5 (two frames)
 *   13DBFC  tilemap 0x66DC2 -> 0x612000 (0x53B2), then 16 pages of the
 *           0xF00/0x780 scroll ramp, one per frame
 *   13DC3E  reset the playfield via 0x145654 (D0/A0 saved), spawn 0x13E12C,
 *           spawn 0x13DEF0 swirls until the pool is full, 0x28 frames
 *   13DCA8  bump 0x32FE, refill 0x626200 with 0x9FD0 (0x13DE08 queues the
 *           0x198E -> 0x200C0 ramp), ramp 0x16980 -> 0x13DE62, 0x18 frames
 *   13DD0A  0x13DE08 again, fade 0x13DE62 in (0x1890), spawn a swirl, 0x18 frames
 *   13DD5C  jingle 0x18 anywhere but level 7, 0x13DE08, another swirl, 0x18 frames
 *   13DDAA  0x32 frames
 *   13DDBE  level 7: forced jingle 0x31; clear 0x1D36 bit 4, 0x13F206, 0x3C frames
 *   13DDFC  clear PF1 + line RAM, bump 0x32FE, rts (back into 0x13F5B8)
 *
 * The callees keep the registers they do not return in (0x13E8A2, 0x53B2,
 * 0x13E91E, 0x13DE08, 0x13F206 save and restore theirs; 0x1057C returns only
 * D0; 0x183C/0x18BA/0x1890 return only D0; 0x145654 runs under its own
 * D0/A0 movem), so the segments set only what the ROM itself writes. */
#include "../kernel/seg.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "../video/palette.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"
#include "../common/fn_leaf1.h"

/* dbra Dn: decrement the low word; 1 = branch taken (result != -1) */
static int seg_dbra(uint32_t *d)
{
    seg_setw(d, (uint16_t)(*d - 1u));
    return (uint16_t)*d != 0xFFFFu;
}

/* 0x13DB3E/0x13DB5C: fill 0xE8 words at 0x62xxxx + (-0x5D56)(A5). The ROM saves
 * and restores D7/A0 around it, so no register carries out. */
static void fill_e8_words(const f3mem_t *m, uint32_t base, uint16_t v)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (unsigned i = 0; i < 0xE8u; i++) {
        wr16(m, a0, v);
        a0 += 2u;
    }
}

/* 0x13E91E: project A5+0x32EE/0x32F0 through the 8-byte record at 0x13E9D8 or
 * 0x13E9E0 (picked by A5-0x5D58), add the A5+0x32F2/0x32F4 offsets, store the
 * pair at 0x628400 + (A5+0x3300).w and the two longs at A5+0x4214/0x4218.
 * Saves and restores D0/D1/D2/A0 (no register change). */
static void fx_13E91E(const f3mem_t *m)
{
    uint32_t a0 = (rd8(m, F3_A5 - 0x5D58u) != 0u) ? 0x0013E9E0u : 0x0013E9D8u;
    f3regs_t s;
    uint16_t d0w, d1w;

    for (unsigned i = 0; i < 8u; i++) {
        s.d[i] = 0u;
        s.a[i] = 0u;
    }
    s.a[0] = a0;
    s.d[0] = rd16(m, F3_A5 + 0x32F6u);                  /* 0x13E93A */
    s.d[1] = rd16(m, F3_A5 + 0x32EEu);                  /* 0x13E93E */
    Leaf_13E994(m, &s);                                 /* 0x13E942 */
    wr32(m, F3_A5 - 0x5D98u, s.d[0]);                   /* 0x13E946 */
    d0w = rd16(m, F3_A5 + 0x32F2u);                     /* 0x13E94A add.w D0w */
    wr16(m, F3_A5 - 0x5D98u, (uint16_t)(rd16(m, F3_A5 - 0x5D98u) + d0w));

    s.d[0] = rd16(m, F3_A5 + 0x32F8u);                  /* 0x13E952 */
    s.d[1] = rd16(m, F3_A5 + 0x32F0u);                  /* 0x13E956 */
    Leaf_13E9B0(m, &s);                                 /* 0x13E95A */
    wr32(m, F3_A5 - 0x5D94u, s.d[0]);                   /* 0x13E95E */
    d0w = rd16(m, F3_A5 + 0x32F4u);                     /* 0x13E962 add.w D0w */
    wr16(m, F3_A5 - 0x5D94u, (uint16_t)(rd16(m, F3_A5 - 0x5D94u) + d0w));

    /* bfins D0,D1{0x10:0x8}: D1 bits 15..8 = D0 bits 15..8 (MSB-first bits). */
    d1w = (uint16_t)(((rd16(m, F3_A5 + 0x32F6u) & 0xFFu) << 8) |
                     (rd16(m, F3_A5 + 0x32F8u) & 0xFFu)); /* 0x13E96A..0x13E972 */
    wr16(m, 0x00628400u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 + 0x3300u),
         d1w);                                          /* 0x13E976..0x13E980 */
    wr32(m, F3_A5 + 0x4214u, rd32(m, F3_A5 - 0x5D98u)); /* 0x13E982 */
    wr32(m, F3_A5 + 0x4218u, rd32(m, F3_A5 - 0x5D94u)); /* 0x13E988 */
}

/* 0x13DC1A..0x13DC38: one step of the scroll ramp, then the frame yield.
 * Returns the TRAP #5 PC the segment stops at. */
static uint32_t ramp_step_13DC1A(const f3mem_t *m, f3regs_t *r)
{
    seg_setw(&r->d[0], (uint16_t)(r->d[0] + 0x0F00u));  /* 13DC1A addi.w */
    seg_setw(&r->d[1], (uint16_t)(r->d[1] - 0x0780u));  /* 13DC1E subi.w */
    r->d[2] = (r->d[0] >> 8) & 0xFFu;                   /* 13DC22 bfextu D0{16:8} */
    wr16(m, F3_A5 + 0x32F6u, (uint16_t)r->d[2]);        /* 13DC26 */
    r->d[2] = (r->d[1] >> 8) & 0xFFu;                   /* 13DC2A bfextu D1{16:8} */
    wr16(m, F3_A5 + 0x32F8u, (uint16_t)r->d[2]);        /* 13DC2E */
    fx_13E91E(m);                                       /* 13DC32 */
    return 0x0013DC38u;                                 /* 13DC38 trap #5 */
}

/* 0x13DC72/0x13DD34/0x13DD82: keep spawning fn until the pool refuses (D0 > 0
 * loops), leaving the last allocation (or -1) in D0. A0 is the parent. */
static uint32_t spawn_until_full(const f3mem_t *m, const f3regs_t *r,
                                 uint32_t fn, uint32_t arg)
{
    uint32_t d0;
    do {
        d0 = Obj_Spawn(m, r->a[0], fn, 0u, 7u, arg);
    } while ((int32_t)d0 > 0);
    return d0;
}

uint32_t Seg_13DAD4(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x0013DAD4u: {                                    /* entry */
        if (rd16(m, F3_A5 + 0x32DCu) != 0u &&              /* 13DAD4 beq */
            rd16(m, F3_A5 + 0x32DEu) != 7u)                /* 13DADE beq */
            Sound_PlayHooked(m, 0x0014u);                  /* 13DAEC + 13DAF2 */
        wr8(m, F3_A5 + 0x1D1Cu, 0xFFu);                    /* 13DAFC st.b */
        if (rd16(m, F3_A5 + 0x32DEu) == 7u)                /* 13DB00 bne */
            Sound_PlayHooked(m, 0x0031u);                  /* 13DB0E + 13DB14 */
        Sound_PlayHooked(m, 0x0017u);                      /* 13DB22 + 13DB28 */
        Gfx_ClearPf1AndLineRam(m);                         /* 13DB32 */
        Fn_13E8A2(m, r);                                   /* 13DB38 (keeps D0/D1/A0) */
        fill_e8_words(m, 0x0062B400u, 0x700Eu);            /* 13DB3E..13DB58 */
        fill_e8_words(m, 0x00626200u, 0x0FF9u);            /* 13DB5C..13DB76 */
        Pal_QueueUpload(m, 0x000204C0u, 0x0A00u, 0x0020u); /* 13DB7A..13DB92 */
        r->d[0] = Obj_Spawn(m, r->a[0], 0x0013DE92u, 0u, 1u, 0u); /* 13DBA8 */
        seg_setw(&r->d[7], 0x0005u);                       /* 13DBB2 */
        seg_setw(&r->d[7], (uint16_t)(r->d[7] - 1u));      /* 13DBB6 subq.w #1 */
        return 0x0013DBB8u;                                /* 13DBB8 trap #5 */
    }

    case 0x0013DBBAu:                                      /* resumed after 13DBB8 */
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)               /* 13DBBA btst/bne */
            return 0x0013DBB8u;
        if (seg_dbra(&r->d[7]))                            /* 13DBC2 dbra */
            return 0x0013DBB8u;
        wr16(m, F3_A5 + 0x32EEu, rd16(m, F3_A5 + 0x32FAu)); /* 13DBC6 */
        wr16(m, F3_A5 + 0x32F0u, rd16(m, F3_A5 + 0x32FCu)); /* 13DBCC */
        wr16(m, F3_A5 + 0x32F2u, 0x0030u);                 /* 13DBD2 */
        wr16(m, F3_A5 + 0x32F4u, 0x0030u);                 /* 13DBD8 */
        wr16(m, F3_A5 + 0x32F6u, 0x0000u);                 /* 13DBDE */
        wr16(m, F3_A5 + 0x32F8u, 0x0080u);                 /* 13DBE4 */
        seg_setw(&r->d[0], 0x0000u);                       /* 13DBEA */
        seg_setw(&r->d[1], 0x8000u);                       /* 13DBEE */
        fx_13E91E(m);                                      /* 13DBF2 */
        return 0x0013DBF8u;                                /* 13DBF8 trap #5 */

    case 0x0013DBFAu:                                      /* resumed after 13DBF8 */
        return 0x0013DBFAu;                                /* 13DBFA trap #5 */

    case 0x0013DBFCu:                                      /* resumed after 13DBFA */
        /* 13DBFC move.w #0x200,-(SP) ; pea 0x612000 ; move.l #0x66DC2 ; 0x53B2
         * (0x53C6 reads the flags as a *long*, so the pushed word 0x0200 is the
         * high half: the XOR mask 0x53DE derives from it is 0x02000000.) */
        Gfx_DrawCompressedTilemap(m, 0x00066DC2u, 0x00612000u, 0x02000000u); /* 13DC0C */
        seg_setw(&r->d[3], 0x000Fu);                       /* 13DC16 */
        return ramp_step_13DC1A(m, r);                     /* falls into 13DC1A */

    case 0x0013DC3Au:                                      /* resumed after 13DC38 */
        if (seg_dbra(&r->d[3]))                            /* 13DC3A dbra */
            return ramp_step_13DC1A(m, r);
        {   /* 13DC3E movem D0/A0 around 0x145654: both restored */
            uint32_t d0 = r->d[0], a0 = r->a[0];
            Fn_145654(m, r);                               /* 13DC42 */
            r->d[0] = d0;
            r->a[0] = a0;
        }
        wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x10u)); /* 13DC4C */
        r->d[0] = Obj_Spawn(m, r->a[0], 0x0013E12Cu, 0u, 7u, 0u); /* 13DC64 */
        wr8(m, F3_A5 + 0x32FEu, 0x00u);                    /* 13DC6E sf.b */
        r->d[0] = spawn_until_full(m, r, 0x0013DEF0u, 0x0013E04Cu); /* 13DC86 */
        seg_setw(&r->d[7], 0x0028u);                       /* 13DC94 */
        seg_setw(&r->d[7], (uint16_t)(r->d[7] - 1u));      /* 13DC98 subq.w #1 */
        return 0x0013DC9Au;                                /* 13DC9A trap #5 */

    case 0x0013DC9Cu:                                      /* resumed after 13DC9A */
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)               /* 13DC9C btst/bne */
            return 0x0013DC9Au;
        if (seg_dbra(&r->d[7]))                            /* 13DCA4 dbra */
            return 0x0013DC9Au;
        wr8(m, F3_A5 + 0x32FEu, (uint8_t)(rd8(m, F3_A5 + 0x32FEu) + 1u)); /* 13DCA8 */
        fill_e8_words(m, 0x00626200u, 0x9FD0u);            /* 13DCAC..13DCC6 */
        Fn_13DE08(m, r);                                   /* 13DCCA bsr */
        /* 13DCCE..13DCF2 jsr 0x183C -> 0x1792: D0 = ramp slot (or -1) */
        {
            uint32_t slot = Pal_QueueRamp(m, 0x00016980u, 0x0013DE62u,
                                          0x00000A00u, 0x0018u, 0x000Cu);
            r->d[0] = (slot == 0u) ? 0xFFFFFFFFu : slot;
        }
        seg_setw(&r->d[7], 0x0018u);                       /* 13DCF6 */
        seg_setw(&r->d[7], (uint16_t)(r->d[7] - 1u));      /* 13DCFA subq.w #1 */
        return 0x0013DCFCu;                                /* 13DCFC trap #5 */

    case 0x0013DCFEu:                                      /* resumed after 13DCFC */
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)               /* 13DCFE btst/bne */
            return 0x0013DCFCu;
        if (seg_dbra(&r->d[7]))                            /* 13DD06 dbra */
            return 0x0013DCFCu;
        Fn_13DE08(m, r);                                   /* 13DD0A bsr */
        /* 13DD0E..13DD2C jsr 0x1890: its D0 (the ramp slot) is dead here, the
         * spawn loop below reloads D0 before the trap. */
        Pal_FadeIn001890(m, 0x0013DE62u, 0x00000A00u,
                         0x000000B4u, 0x0000000Cu);
        seg_setw(&r->d[7], 0x0018u);                       /* 13DD30 */
        r->d[0] = spawn_until_full(m, r, 0x0013DEF0u, 0x0013E05Cu); /* 13DD48 */
        return 0x0013DD56u;                                /* 13DD56 trap #5 */

    case 0x0013DD58u:                                      /* resumed after 13DD56 */
        if (seg_dbra(&r->d[7])) {                          /* 13DD58 dbra */
            r->d[0] = spawn_until_full(m, r, 0x0013DEF0u, 0x0013E05Cu);
            return 0x0013DD56u;
        }
        if (rd16(m, F3_A5 + 0x32DEu) != 7u)                /* 13DD5C beq */
            Sound_PlayHooked(m, 0x0018u);                  /* 13DD6A + 13DD70 */
        Fn_13DE08(m, r);                                   /* 13DD7A bsr */
        seg_setw(&r->d[7], 0x0018u);                       /* 13DD7E */
        r->d[0] = spawn_until_full(m, r, 0x0013DEF0u, 0x0013E06Cu); /* 13DD96 */
        return 0x0013DDA4u;                                /* 13DDA4 trap #5 */

    case 0x0013DDA6u:                                      /* resumed after 13DDA4 */
        if (seg_dbra(&r->d[7])) {                          /* 13DDA6 dbra */
            r->d[0] = spawn_until_full(m, r, 0x0013DEF0u, 0x0013E06Cu);
            return 0x0013DDA4u;
        }
        seg_setw(&r->d[7], 0x0032u);                       /* 13DDAA */
        seg_setw(&r->d[7], (uint16_t)(r->d[7] - 1u));      /* 13DDAE subq.w #1 */
        return 0x0013DDB0u;                                /* 13DDB0 trap #5 */

    case 0x0013DDB2u:                                      /* resumed after 13DDB0 */
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)               /* 13DDB2 btst/bne */
            return 0x0013DDB0u;
        if (seg_dbra(&r->d[7]))                            /* 13DDBA dbra */
            return 0x0013DDB0u;
        if (rd16(m, F3_A5 + 0x32DEu) == 7u)                /* 13DDBE bne */
            Sound_PlayCmdForced(m, Sound_PreCmdHook(m, 0x0031)); /* 13DDCC + 13DDD2 */
        wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & ~0x10u)); /* 13DDDC */
        Fn_13F206(m, r);                                   /* 13DDE2 (keeps D0/A0) */
        seg_setw(&r->d[7], 0x003Cu);                       /* 13DDE8 */
        seg_setw(&r->d[7], (uint16_t)(r->d[7] - 1u));      /* 13DDEC subq.w #1 */
        return 0x0013DDEEu;                                /* 13DDEE trap #5 */

    case 0x0013DDF0u:                                      /* resumed after 13DDEE */
        if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)               /* 13DDF0 btst/bne */
            return 0x0013DDEEu;
        if (seg_dbra(&r->d[7]))                            /* 13DDF8 dbra */
            return 0x0013DDEEu;
        Gfx_ClearPf1AndLineRam(m);                         /* 13DDFC */
        wr8(m, F3_A5 + 0x32FEu, (uint8_t)(rd8(m, F3_A5 + 0x32FEu) + 1u)); /* 13DE02 */
        return Seg_Run(m, r, seg_pop32(m, r));             /* 13DE06 rts */

    case 0x0013DBB8u:                                      /* resumed on a TRAP */
    case 0x0013DBF8u:
    case 0x0013DC38u:
    case 0x0013DC9Au:
    case 0x0013DCFCu:
    case 0x0013DD56u:
    case 0x0013DDA4u:
    case 0x0013DDB0u:
    case 0x0013DDEEu:
        return pc;
    }
    return SEG_LEAVE;
}
