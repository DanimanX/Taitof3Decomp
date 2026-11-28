/* ROM 0x1A00E6 test-mode screen "SOUND TEST" (one of the two screens the
 * wrapper 0x1A0942 picks on bit 1 of the cabinet switch byte 0x402220), in
 * segments (src/kernel/seg.h). It yields with jsr 0x001F62 (Sys_Yield call
 * gate) at 0x1A0162, 0x1A0348 and 0x1A038A: a segment stops AT that jsr and
 * returns its PC; the next one resumes 6 bytes later (0x1A0168, 0x1A034E,
 * 0x1A0390). The screen ends at the RTS at 0x1A043A.
 *
 * Entry 0x1A00E6 links a 0xC-byte frame and saves D2-D7/A2-A4: A2 = the text
 * base 0x61C000, A3 = the volume word 0x402240, A4 = 0x5666 (jsr (A4) draws),
 * -0xC(A6) = 3 rows, -8(A6) = the volume read from 0x402240, D6 = the sound
 * number (1), D3 = the sound last played (-1), D7 = the exit flag. It clears
 * the layers, loads the 0x48-long character block 0x1A1C78 -> 0x61E200,
 * silences the sound chip (0x39D8 with 0) and reloads the settings (0x4D06),
 * draws the title "SOUND TEST" at 0x61C09C and falls into the frame loop.
 * The `clr.w -(SP)` at 0x1A0130 leaves its word on the stack: the lea 0xC(SP)
 * at 0x1A015A drops it together with the ten argument bytes of the title, so
 * SP is back at the saved-register block (the capture's SP is A6-0x30).
 *
 * Each 0x1A0168 frame walks the selection D2 with the P2 switch byte 0x402225
 * (bit 1 down, bit 0 up, wrapping through 3 rows), plays a sound when the
 * selection changes, then draws the three rows through jsr 0x1A1BA6 at
 * 0x61C40E, 0x61C528/0x61C58E/0x61C5A8 and 0x61C98E - the SOUND NO line, the
 * volume line ("MIN  MAX" + "SOUND VOLUME:" + the bar drawn by 0x1A0086) and
 * the EXIT line. The frame finishes by publishing the edited volume
 * (`move.l -8(A6),D0 ; ext.w D0 ; move.w D0,0x402240`) and sending it to the
 * sound chip with 0x3CBA, then yields unless the EXIT row armed D7.
 *
 * D7 set: the last played sound is retriggered (0x3B66), the layers are
 * cleared and the screen yields once more (0x1A0348) before the confirm page:
 * 0x1A034E draws "SOUND TEST" / "SAVE MODIFIED DATA" / "ARE YOU SURE?", sets
 * D2 = D7 = 0 and falls to the loop test, so its first frame ends at the
 * 0x1A038A yield.
 *
 * Each 0x1A0390 frame runs the YES/NO cursor through the shared editor
 * 0x19FFF0 (P1 switch byte 0x402224, range 0..1, step 1), draws the
 * "YES       NO" line at 0x61C79C and paints the choice with the colour-8
 * YES (0x1A20FA) or NO (0x1A20FE) arm of the word/jmp table at 0x1A03D4. Bit
 * 4 of 0x402225 confirms: D2 == 0 writes the volume byte to 0x406C62 and runs
 * 0x4DEE, whose serial write-back cannot succeed without the device and
 * enters the 0x01014E reset loop (SEG_LEAVE, as in seg_1A0A52.c); D2 == 1
 * reloads the settings with 0x4D06; both then clear the layers and return
 * through the RTS at 0x1A043A.
 *
 * Verification: seg_1A0168 (30 pairs) is 30/30 bit-exact - RAM, every
 * register, SP and the yield PC 0x1A0162. Those frames all have the P2 switch
 * byte 0xFF, so they cover the entry, the title, the three row draws, the
 * volume publish (0x3CBA) and the yield, but NOT: the sound cursor movement
 * under 0x1A0168..0x1A01E2 (both direction bits set in every captured frame),
 * the editor arms of rows 1 and 2, the exit path 0x1A0332..0x1A0348, the
 * confirm page 0x1A034E and the confirm loop 0x1A0390 (which has no capture:
 * seg_1A00E6, seg_1A034E and seg_1A09AC exist but are empty). Those were read
 * against the bytes and exercised with a synthetic driver: the cursor walk
 * 0->1->2, the EXIT confirm (yield 0x1A0348), the confirm page (0x1A038A),
 * both YES (0x4DEE -> SEG_LEAVE) and NO (0x4D06 -> epilogue -> RTS) arms. */

#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */
#include "../audio/sound_cmd.h"     /* Sound_PlayCmd 0x39D8, Sound_PlayCmdForced
                                     * 0x3B66, Sound_SetVolume 0x3CBA */
#include "../auto/fn_auto.h"        /* Fn_19FFF0, Fn_1A1BA6, Fn_1A0086 */
#include "../auto/eeprom_block.h"   /* settings_apply_4D06 / refresh_4DEE */

#define SEG_YIELD1 0x001A0162u      /* jsr 0x001F62, the frame loop */
#define SEG_YIELD2 0x001A0348u      /* jsr 0x001F62, leave the editor */
#define SEG_YIELD3 0x001A038Au      /* jsr 0x001F62, the confirm loop */
#define P1_SWITCH  0x00402224u
#define P2_SWITCH  0x00402225u
#define VOLUME     0x00402240u

/* 0x1A00E6 link.w a6,#-0xC ; 0x1A00EA movem.l d2-d7/a2-a4,-(SP) */
static void save_frame(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[6]);
    r->a[6] = r->a[7];
    r->a[7] -= 0xCu;
    r->a[7] -= 36u;
    for (unsigned i = 0; i < 6u; ++i)
        wr32(m, r->a[7] + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 3u; ++i)
        wr32(m, r->a[7] + 24u + 4u * i, r->a[2u + i]);
}

/* 0x1A0434 movem.l (SP)+,d2-d7/a2-a4 ; 0x1A0438 unlk a6 ; 0x1A043A rts */
static uint32_t restore_frame(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 0; i < 6u; ++i)
        r->d[2u + i] = rd32(m, r->a[7] + 4u * i);
    for (unsigned i = 0; i < 3u; ++i)
        r->a[2u + i] = rd32(m, r->a[7] + 24u + 4u * i);
    r->a[7] += 36u;
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return Seg_Run(m, r, seg_pop32(m, r));
}

/* jsr (A4) -> 0x5666: the three arguments stay until the caller's lea, the
 * callee's frame is written below them and only D0 (the end of the line)
 * survives. */
static void draw_text(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      uint32_t dst, uint32_t src, uint16_t col)
{
    uint32_t sp = r->a[7] - 10u;
    r->a[7] = sp;
    wr32(m, sp, src);
    wr32(m, sp + 4u, dst);
    wr16(m, sp + 8u, col);
    wr32(m, sp - 4u, ret);
    wr32(m, sp - 8u, r->a[6]);
    wr32(m, sp - 20u, r->d[7]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 12u, r->a[1]);
    r->d[0] = Text_DrawString(m, src, dst, col);
    r->a[7] = sp;                       /* the callee's rts pops its return PC */
}

/* jsr 0x1A1BA6: the caller pushes [arguments..., last first] then fmt.l,
 * col.w and dst.l; the callee formats into its own buffer, draws it and
 * returns with SP just past its return address, so the argument bytes stay
 * for the caller's lea. */
static void draw_format(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    Fn_1A1BA6(m, r);
}

/* 0x19FFF0 with the five argument words the callers push (input byte, current
 * value, low bound, high bound, step) - see seg_1A1944.c. */
static void edit_value(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                       uint16_t in, uint16_t cur, uint16_t lo, uint16_t hi,
                       uint16_t step)
{
    push_word(m, r, step);              /* push order of the call sites: step
                                         * first, the input byte last */
    push_word(m, r, hi);
    push_word(m, r, lo);
    push_word(m, r, cur);
    push_word(m, r, in);
    push_long(m, r, ret);
    Fn_19FFF0(m, r);                    /* leaves the five words in place */
}

/* jsr 0x1A0086 (src/auto/fn_1A0086.c): the bar arguments dst.l, value.l,
 * slots.l and colour.w are pushed last-first, then the return PC. */
static void draw_bar(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                     uint32_t dst, uint32_t value, uint32_t slots, uint16_t col)
{
    push_word(m, r, col);
    push_long(m, r, slots);
    push_long(m, r, value);
    push_long(m, r, dst);
    push_long(m, r, ret);
    Fn_1A0086(m, r);
}

/* `move.w #cmd,-(SP) ; jsr 0x39D8 ; addq.l #2,SP` - the callee's rts pops the
 * return address, the addq the argument. */
static void play_cmd(const f3mem_t *m, f3regs_t *r, uint32_t ret, int16_t cmd)
{
    push_word(m, r, (uint16_t)cmd);
    push_long(m, r, ret);
    Sound_PlayCmd(m, cmd);
    r->a[7] += 6u;
}

/* the same with the forced dispatcher 0x3B66 */
static void play_forced(const f3mem_t *m, f3regs_t *r, uint32_t ret, uint16_t cmd)
{
    push_word(m, r, cmd);
    push_long(m, r, ret);
    Sound_PlayCmdForced(m, (int16_t)cmd);
    r->a[7] += 6u;
}

/* 0x1A020C: row 0, the sound number. The selected row runs the number through
 * the editor (1..[0x6750], step 1) and, when bit 4 of the P2 switch byte
 * confirms, retriggers the previous sound and plays the new one. Every row
 * then draws "SOUND NO    :" and the value as %2.2X at 0x61C40E. */
static void row_sound_no(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[4] == r->d[2]) {                       /* 0x1A020C cmp.l D2,D4 */
        r->d[0] = 0u;                               /* 0x1A0220 moveq #0,D0 */
        r->d[0] |= rd8(m, P1_SWITCH);               /* 0x1A0222 move.b */
        edit_value(m, r, 0x001A022Eu, (uint16_t)r->d[0], (uint16_t)r->d[6],
                   1u, rd16(m, 0x00006750u), 1u);
        r->a[7] += 0xAu;                            /* 0x1A022E lea 0xA(SP) */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.l D0 */
        r->d[6] = r->d[0];                          /* 0x1A0234 move.l D0,D6 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (!(r->d[0] & 0xFFu)) {                   /* 0x1A0240 bne */
            r->d[0] = r->d[3];                      /* 0x1A0242 move.l D3,D0 */
            r->d[0] += 1u;                          /* 0x1A0244 addq.l #1 */
            if (r->d[0] != 0u)                      /* 0x1A0246 beq */
                play_forced(m, r, 0x001A0250u, (uint16_t)r->d[3]); /* 0x3B66 */
            play_cmd(m, r, 0x001A025Au, (int16_t)(uint16_t)r->d[6]); /* 0x39D8 */
            r->d[3] = r->d[6];                      /* 0x1A025C move.l D6,D3 */
        }
    }
    push_long(m, r, r->d[6]);                       /* 0x1A025E */
    push_long(m, r, 0x001A209Du);                   /* "SOUND NO    :%2.2X" */
    push_word(m, r, (uint16_t)r->d[5]);
    push_long(m, r, 0x0061C40Eu);
    draw_format(m, r, 0x001A0272u);
    r->a[7] += 0xEu;                                /* 0x1A0272 lea 0xE(SP) */
}

/* 0x1A027A: row 1, the sound volume. The selected row edits -8(A6) in
 * 0..0x3F, then the row draws "MIN  MAX" at 0x61C528, "SOUND VOLUME:" at
 * 0x61C58E and the eight-slot bar at 0x61C5A8 (0x1A0086). */
static void row_volume(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[4] == r->d[2]) {                       /* 0x1A027A cmp.l D2,D4 */
        r->d[0] = rd8(m, P1_SWITCH);                /* 0x1A028E moveq/move.b */
        edit_value(m, r, 0x001A029Cu, (uint16_t)r->d[0],
                   (uint16_t)rd32(m, r->a[6] - 8u), 0u, 0x3Fu, 1u);
        r->a[7] += 0xAu;                            /* 0x1A029C lea 0xA(SP) */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.l D0 */
        wr32(m, r->a[6] - 8u, r->d[0]);             /* 0x1A02A2 move.l D0,-8(A6) */
    }
    push_long(m, r, 0x001A20B0u);                   /* 0x1A02A6 "MIN  MAX" */
    push_word(m, r, (uint16_t)r->d[5]);
    push_long(m, r, 0x0061C528u);
    draw_format(m, r, 0x001A02B8u);
    push_long(m, r, 0x001A20B9u);                   /* 0x1A02B8 "SOUND VOLUME:" */
    push_word(m, r, (uint16_t)r->d[5]);
    push_long(m, r, 0x0061C58Eu);
    draw_format(m, r, 0x001A02CAu);
    draw_bar(m, r, 0x001A02DCu, 0x0061C5A8u, rd32(m, r->a[6] - 8u), 8u,
             (uint16_t)r->d[5]);
    r->a[7] += 0x22u;                               /* 0x1A02DC lea 0x22(SP) */
}

/* 0x1A02E2: row 2, EXIT. Bit 4 of the P2 switch byte on the selected row arms
 * the screen's exit flag D7. */
static void row_exit(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[4] == r->d[2]) {                       /* 0x1A02E2 cmp.l D2,D4 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (!(r->d[0] & 0xFFu))                     /* 0x1A02F0 bne */
            r->d[7] = 1u;                           /* 0x1A02F2 moveq #1,D7 */
    }
    push_long(m, r, 0x001A20C7u);                   /* 0x1A02F4 "EXIT" */
    push_word(m, r, (uint16_t)r->d[5]);
    push_long(m, r, 0x0061C98Eu);
    draw_format(m, r, 0x001A0306u);
    r->a[7] += 0xAu;                                /* 0x1A0306 lea 0xA(SP) */
}

/* 0x1A0168..0x1A032C: one frame of the editor. Returns SEG_YIELD1 when the
 * frame yields, SEG_YIELD2 when the EXIT row armed D7. */
static uint32_t frame_step(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t rows = rd32(m, r->a[6] - 0xCu);

    /* 0x1A0168: bit 1 of the P2 switch byte steps the selection down */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x02u);
    if (!(r->d[0] & 0xFFu)) {                       /* 0x1A0172 bne */
        r->d[2] += 1u;                              /* 0x1A0174 addq.l #1,D2 */
        if ((int32_t)r->d[2] >= (int32_t)rows)      /* 0x1A017A blt */
            r->d[2] = 0u;                           /* 0x1A017C moveq #0,D2 */
        r->d[0] = r->d[3];
        r->d[0] += 1u;                              /* 0x1A0180 addq.l #1,D0 */
        if (r->d[0] != 0u)                          /* 0x1A0182 beq */
            play_forced(m, r, 0x001A018Cu, (uint16_t)r->d[3]);
        r->d[0] = r->d[2];
        r->d[0] -= 1u;                              /* 0x1A0190 subq.l #1,D0 */
        if (r->d[0] == 0u) {                        /* 0x1A0192 bne 0x1A01A4 */
            play_cmd(m, r, 0x001A019Eu, 0x4C);
            r->d[3] = 0x4Cu;                        /* 0x1A01A0 moveq #0x4C,D3 */
        } else {
            r->d[3] = 0xFFFFFFFFu;                  /* 0x1A01A4 moveq #-1,D3 */
        }
    }

    /* 0x1A01A6: bit 1 of the P2 switch byte steps the selection up */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x01u);
    if (!(r->d[0] & 0xFFu)) {                       /* 0x1A01B0 bne */
        r->d[2] -= 1u;                              /* 0x1A01B2 subq.l #1,D2 */
        if ((int32_t)r->d[2] < 0) {                 /* 0x1A01B4 bpl */
            r->d[2] = 0xFFFFFFFFu;                  /* 0x1A01B6 moveq #-1,D2 */
            r->d[2] += rows;                        /* 0x1A01B8 add.l */
        }
        r->d[0] = r->d[3];
        r->d[0] += 1u;
        if (r->d[0] != 0u)                          /* 0x1A01C0 beq */
            play_forced(m, r, 0x001A01CAu, (uint16_t)r->d[3]);
        r->d[0] = r->d[2];
        r->d[0] -= 1u;                              /* 0x1A01CE subq.l #1,D0 */
        if (r->d[0] == 0u) {                        /* 0x1A01D0 bne 0x1A01E2 */
            play_cmd(m, r, 0x001A01DCu, 0x4C);
            r->d[3] = 0x4Cu;                        /* 0x1A01DE */
        } else {
            r->d[3] = 0xFFFFFFFFu;                  /* 0x1A01E2 */
        }
    }

    /* 0x1A01E4: one row per line. D5 is the colour: 8 for the selected row. */
    r->d[4] = 0u;
    while ((int32_t)r->d[4] < (int32_t)rows) {      /* 0x1A030C cmp/blt */
        r->d[5] = (r->d[4] == r->d[2]) ? 8u : 7u;   /* 0x1A01EA..0x1A01F2 */
        r->d[0] = r->d[4];
        r->d[0] -= 2u;                              /* 0x1A01F6 subq.l #2,D0 */
        /* 0x1A01F8 bhi.w 0x1A030A: not taken exactly when the subtraction
         * borrowed or gave zero, i.e. for rows 0, 1 and 2. */
        if (!(r->d[4] < 2u) && r->d[0] != 0u) {
            r->d[4] += 1u;
            continue;
        }
        {
            int32_t idx = (int32_t)(int16_t)(uint16_t)r->d[0];
            uint16_t off = rd16(m, (uint32_t)(0x001A020Au + 2 * idx));
            seg_setw(&r->d[0], off);                /* 0x1A01FC move.w */
            switch (0x001A0206u + (uint32_t)(int16_t)off) {
            case 0x1A020Cu: row_sound_no(m, r); break;
            case 0x1A027Au: row_volume(m, r); break;
            case 0x1A02E2u: row_exit(m, r); break;
            }
        }
        r->d[4] += 1u;                              /* 0x1A030A addq.l #1,D4 */
    }

    /* 0x1A0314: publish the volume and push it to the sound chip four times
     * (0x3CBA writes the four 0xC007F8 bytes). */
    r->d[0] = rd32(m, r->a[6] - 8u);
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* 0x1A0318 ext.w */
    wr16(m, VOLUME, (uint16_t)r->d[0]);                         /* 0x1A031A */
    for (unsigned i = 0; i < 4u; ++i)
        push_word(m, r, rd16(m, VOLUME));
    push_long(m, r, 0x001A032Au);
    {
        uint8_t v = (uint8_t)r->d[0];
        Sound_SetVolume(m, v, v, v, v);                         /* 0x1A0324 */
    }
    r->a[7] += 0xCu;                    /* rts + 0x1A032A addq.l #8,SP */

    if ((r->d[7] & 0xFFFFu) == 0u)                  /* 0x1A032C tst.w D7 */
        return SEG_YIELD1;                          /* 0x1A032E beq */

    /* 0x1A0332: leaving - retrigger the last sound, clear the layers */
    r->d[0] = r->d[3];
    r->d[0] += 1u;
    if (r->d[0] != 0u)                              /* 0x1A0336 beq */
        play_forced(m, r, 0x001A0340u, (uint16_t)r->d[3]);
    Gfx_ClearAllLayers(m);                          /* 0x1A0342 */
    return SEG_YIELD2;                              /* 0x1A0348 jsr 0x001F62 */
}

uint32_t Seg_1A00E6(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A00E6u:
        save_frame(m, r);
        r->a[2] = 0x0061C000u;      /* text RAM base */
        r->a[3] = VOLUME;
        r->a[4] = 0x00005666u;      /* jsr (A4) draws text */
        r->d[0] = 3u;
        wr32(m, r->a[6] - 0xCu, r->d[0]);   /* rows */
        r->d[2] = 0u;                       /* the selection */
        r->d[6] = 0u;                       /* the sound number */
        r->d[3] = 0xFFFFFFFFu;              /* the sound last played */
        wr32(m, r->a[6] - 8u, 0u);          /* clr.l -8(A6) */
        r->d[7] = 0u;                       /* the exit flag */
        Gfx_ClearAllLayers(m);              /* 0x1A0112 */
        for (r->d[4] = 0u; ; r->d[4] += 1u) {
            uint32_t off = 4u * r->d[4];
            wr32(m, 0x0061E200u + off, rd32(m, 0x001A1C78u + off));
            r->d[0] = 0x48u;                /* 0x1A012A moveq #0x48,D0 */
            if ((int32_t)r->d[4] >= (int32_t)r->d[0])   /* 0x1A012E blt */
                break;
        }
        /* 0x1A0130 clr.w -(SP): the sound command 0; the word is left on the
         * stack and only dropped by the lea 0xC(SP) at 0x1A015A. */
        push_word(m, r, 0u);
        push_long(m, r, 0x001A0138u);
        Sound_PlayCmd(m, 0);                /* 0x1A0132 */
        r->a[7] += 4u;                      /* its rts; the argument word stays */
        push_long(m, r, 0x001A013Eu);
        settings_apply_4D06(m, r);          /* 0x1A0138 jsr 0x004D06 */
        r->a[0] = (uint32_t)(int32_t)(int16_t)rd16(m, r->a[3]);  /* 0x1A013E movea.w (A3),A0 */
        wr32(m, r->a[6] - 8u, r->a[0]);             /* 0x1A0140 */
        r->d[6] = 1u;                               /* 0x1A0144 */
        push_long(m, r, 0x001A2092u);               /* 0x1A0146 pea "SOUND TEST" */
        push_word(m, r, 0xAu);                      /* 0x1A014C move.w #0xA */
        push_long(m, r, 0x0061C09Cu);               /* 0x1A0150 pea 0x9C(A2) */
        draw_format(m, r, 0x001A015Au);             /* 0x1A0154 jsr 0x1A1BA6 */
        r->a[7] += 0xCu;                            /* 0x1A015A lea 0xC(SP) */
        return SEG_YIELD1;                  /* 0x1A032C/0x1A032E */

    case 0x1A0168u:
        return frame_step(m, r);

    case 0x1A034Eu:
        /* 0x1A034E..0x1A0380: the three confirm lines, then D2 = D7 = 0 and
         * the loop test at 0x1A040A sends the frame to the yield. */
        draw_text(m, r, 0x001A035Eu, 0x0061C09Cu, 0x001A2092u, 0xAu);
        draw_text(m, r, 0x001A036Eu, 0x0061C316u, 0x001A20CCu, 0x7u);
        draw_text(m, r, 0x001A037Eu, 0x0061C41Cu, 0x001A20DFu, 0x7u);
        r->a[7] += 0x1Eu;                   /* 0x1A037E lea 0x1E(SP) */
        r->d[2] = 0u;                       /* 0x1A0382 */
        r->d[7] = 0u;                       /* 0x1A0384 */
        return SEG_YIELD3;                  /* 0x1A040A/0x1A040C */

    case 0x1A0390u: {
        /* One frame of the YES/NO loop. */
        r->d[0] = rd8(m, P1_SWITCH);        /* 0x1A039C moveq/move.b */
        edit_value(m, r, 0x001A03AAu, (uint16_t)r->d[0], (uint16_t)r->d[2],
                   0u, 1u, 1u);
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.l D0 */
        r->d[2] = r->d[0];                  /* 0x1A03AC move.l D0,D2 */
        draw_text(m, r, 0x001A03BEu, 0x0061C79Cu, 0x001A20EDu, 0x7u);
        r->a[7] += 0x14u;                   /* 0x1A03BE lea 0x14(SP) */
        r->d[0] = r->d[2];
        r->d[0] -= 1u;                      /* 0x1A03C4 subq.l #1,D0 */
        /* 0x1A03C6 bhi.w 0x1A03FC: taken for every value above 1 */
        if (r->d[2] <= 1u) {
            int32_t idx = (int32_t)(int16_t)(uint16_t)r->d[0];
            uint16_t off = rd16(m, (uint32_t)(0x001A03D6u + 2 * idx));
            seg_setw(&r->d[0], off);        /* 0x1A03CA move.w */
            switch (0x001A03D4u + (uint32_t)(int16_t)off) {
            case 0x1A03D8u:                 /* D2 == 0: YES at 0x61C79C */
                draw_text(m, r, 0x001A03F8u, 0x0061C79Cu, 0x001A20FAu, 0x8u);
                break;
            case 0x1A03E8u:                 /* D2 == 1: NO at 0x61C7B0 */
                draw_text(m, r, 0x001A03F8u, 0x0061C7B0u, 0x001A20FEu, 0x8u);
                break;
            }
            r->a[7] += 0xAu;                /* 0x1A03F8 lea 0xA(SP) */
        }
        /* 0x1A03FC: bit 4 of the P2 switch byte confirms the choice */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (!(r->d[0] & 0xFFu))             /* 0x1A0406 bne */
            r->d[7] = 1u;                   /* 0x1A0408 moveq #1,D7 */
        if ((r->d[7] & 0xFFFFu) == 0u)      /* 0x1A040A tst.w D7 */
            return SEG_YIELD3;              /* 0x1A040C beq */

        if (r->d[2] == 0u) {                /* 0x1A0410 tst.l D2 ; bne */
            r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, VOLUME);
            wr8(m, 0x00406C62u, (uint8_t)r->d[0]);  /* 0x1A0416 */
            push_long(m, r, 0x001A0422u);
            /* 0x1A041C jsr 0x004DEE: the checksum recompute and the serial
             * write-back, which enters the 0x01014E reset loop when the device
             * does not answer - the harness has no I/O space, so this arm
             * never returns (same as seg_1A0A52.c). */
            push_long(m, r, 0x001A0422u);
            (void)settings_refresh_4DEE(m, r);
            return SEG_LEAVE;
        }
        r->d[2] -= 1u;                      /* 0x1A0424 subq.l #1,D2 */
        if (r->d[2] == 0u) {                /* 0x1A0426 bne */
            push_long(m, r, 0x001A042Eu);
            settings_apply_4D06(m, r);      /* 0x1A0428 jsr 0x004D06 */
        }
        Gfx_ClearAllLayers(m);              /* 0x1A042E */
        return restore_frame(m, r);         /* 0x1A0434..0x1A043A */
    }
    }
    return SEG_LEAVE;
}
