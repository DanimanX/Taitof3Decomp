/* ROM 0x1A043C test-mode screen "SOUND TEST" (the arm the wrapper 0x1A0942
 * takes when bit 1 of the cabinet switch byte 0x402220 is clear), in segments
 * (src/kernel/seg.h).
 *
 * UNVERIFIED (no capture): work/ram has no seg_1A043C or seg_1A0922 data
 * (seg_1A043C_regs.tsv is empty), so every statement below was read from
 * work/maincpu.bin and checked against the byte-level disassembly only;
 * nothing here is confirmed against MAME. It was exercised with a synthetic
 * driver from a captured screen state: the cursor walk 0->1->10->11 (the
 * directory gate skips the TRACK rows) and 0->11->10 upwards, the TRACK rows
 * when the gate byte is forced to 1, the editors of rows 0, 1, 2 and 10, the
 * eight 0x3FD8 queues, and both exits (yield 0x1A091C, then 0x1A0922 ->
 * SEG_LEAVE through 0x4DEE).
 *
 * It yields with jsr 0x001F62 (Sys_Yield call gate) at 0x1A04D4 (the frame
 * loop) and 0x1A091C (leave the editor): a segment stops AT that jsr and
 * returns its PC, the next one resumes 6 bytes later (0x1A04DA, 0x1A0922).
 * The screen ends at the RTS at 0x1A0940.
 *
 * Entry 0x1A043C links a 0x28-byte frame and saves D2-D7/A2-A4: A2 = the text
 * base 0x61C000, A4 = 0x1A1BA6 (jsr (A4) formats and draws), D3 = the selected
 * sound (1), D4 = the selected row (0), D7 = the sound last played (-1),
 * -0x28(A6) = 12 rows, -0x24(A6) = the sound whose track volumes were last
 * loaded (-1), -0x20(A6) = the code volume, -0x1C(A6) = the master volume,
 * -0x18..-0x2(A6) = eight per-track volumes and -0x6(A6) = the exit flag. It
 * clears the layers, loads the same 0x48-long character block, silences the
 * chip (0x39D8 with 0), reloads the settings (0x4D06), initialises the record
 * directory (0x4088, which publishes 0x407418 as the directory base), seeds
 * the master volume from 0x402240, clears the eight track volumes and draws
 * the title "SOUND TEST" at 0x61C09C.
 *
 * Each 0x1A04DA frame walks the row selection D4 with the P2 switch byte
 * 0x402225 (bit 1 down, bit 0 up, wrapping through 12 rows). The directory
 * entry at 0x407418 + D3*4 gates the walk: while its type byte is above 1 the
 * TRACK rows 2..9 are skipped (the down step jumps 2..9 to 10, the up step to
 * 1), so only SOUND NO / CODE VOLUME / SOUND VOLUME / EXIT are reachable in
 * that case. The twelve rows are drawn at 0x61C000 + row*0x100 through the
 * word/jmp table at 0x1A0564: row 0 "SOUND NO    :%2.2X" (with the record
 * type byte > 1 it refills the eight track volumes with 0x7F when a new sound
 * is picked, 0 elsewhere), row 1 "MIN          MAX" / "CODE VOLUME :" / the
 * "%2.2X" value / a 0x10-slot bar, rows 2..9 "TRACK %1d     :" / the "%2.2X"
 * value / a 0x10-slot bar per track, row 10 "SOUND VOLUME:" and an 8-slot bar,
 * row 11 "EXIT", which arms the exit flag. Selecting a row runs its value
 * through the editor 0x19FFF0 (P1 switch byte 0x402224) and bit 4 of 0x402225
 * retriggers the last sound and plays the selected one.
 *
 * After the rows the frame queues the eight track volumes through 0x3FD8
 * (skipped while the record type byte is above 1; the 0x64E8 compare at
 * 0x1A08A8 is dead - D0 is -1 there), publishes the master volume to 0x402240
 * and sends it to the chip with 0x3CBA, then yields unless the EXIT row armed
 * -0x6(A6).
 *
 * 0x1A091C yields once more; 0x1A0922 then writes the volume byte to 0x406C62
 * and runs 0x4DEE, whose serial write-back cannot succeed without the device
 * and enters the 0x01014E reset loop (SEG_LEAVE, as in seg_1A0A52.c); the
 * epilogue behind it (0x1A0934 clear the layers, restore and RTS) is kept for
 * the arm where the write-back succeeds. */
#include "../kernel/seg.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Gfx_ClearAllLayers (ROM 0x005A12) */
#include "../audio/sound_cmd.h"     /* Sound_PlayCmd / Forced / SetVolume */
#include "../auto/fn_auto.h"        /* Fn_19FFF0, Fn_1A1BA6, Fn_1A0086,
                                     * Fn_004088, Fn_003FD8 */
#include "../auto/eeprom_block.h"   /* settings_apply_4D06 / refresh_4DEE */

#define SEG_YIELD1 0x001A04D4u      /* jsr 0x001F62, the frame loop */
#define SEG_YIELD2 0x001A091Cu      /* jsr 0x001F62, leave the editor */
#define P1_SWITCH  0x00402224u
#define P2_SWITCH  0x00402225u
#define VOLUME     0x00402240u
#define DIRECTORY  0x00407418u      /* the record pointer table 0x4088 builds */

#define L_ROWS     (r->a[6] - 0x28u)    /* 0x1A0452 */
#define L_LASTSND  (r->a[6] - 0x24u)    /* 0x1A045E */
#define L_CODEVOL  (r->a[6] - 0x20u)    /* 0x1A0462 */
#define L_MASTERV  (r->a[6] - 0x1Cu)    /* 0x1A0466 */
#define L_TRACK(i) (r->a[6] - 0x18u + 2u * (i))    /* -0x18(A6,Dn*2): track i */
#define L_TRACKROW(row) (r->a[6] - 0x1Cu + 2u * (row))  /* -0x1C(A6,Dn*2) */
#define L_EXIT     (r->a[6] - 0x06u)    /* 0x1A046A */

/* 0x1A043C link.w a6,#-0x28 ; 0x1A0440 movem.l d2-d7/a2-a4,-(SP) */
static void save_frame(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[6]);
    r->a[6] = r->a[7];
    r->a[7] -= 0x28u;
    r->a[7] -= 36u;
    for (unsigned i = 0; i < 6u; ++i)
        wr32(m, r->a[7] + 4u * i, r->d[2u + i]);
    for (unsigned i = 0; i < 3u; ++i)
        wr32(m, r->a[7] + 24u + 4u * i, r->a[2u + i]);
}

/* 0x1A093A movem.l (SP)+,d2-d7/a2-a4 ; 0x1A093E unlk a6 ; 0x1A0940 rts */
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

/* jsr (A4) -> 0x1A1BA6: the caller pushes [arguments..., fmt.l, col.w, dst.l]
 * and the callee returns with SP just past its return address. */
static void draw_format(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    Fn_1A1BA6(m, r);
}

/* 0x19FFF0 with its five argument words, pushed step first, input byte last -
 * the order 0x19FFF0 reads them in (see seg_1A1944.c). */
static void edit_value(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                       uint16_t in, uint32_t cur, uint16_t lo, uint16_t hi,
                       uint16_t step)
{
    push_word(m, r, step);
    push_word(m, r, hi);
    push_word(m, r, lo);
    push_word(m, r, (uint16_t)cur);
    push_word(m, r, in);
    push_long(m, r, ret);
    Fn_19FFF0(m, r);                    /* leaves the five words in place */
}

/* jsr 0x1A0086 with dst.l, value.l, slots.l, colour.w (last pushed last) */
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

/* `move.w #cmd,-(SP) ; jsr 0x39D8 ; addq.l #2,SP` (the callee's rts pops the
 * return address, the addq the argument) */
static void play_cmd(const f3mem_t *m, f3regs_t *r, uint32_t ret, int16_t cmd)
{
    push_word(m, r, (uint16_t)cmd);
    push_long(m, r, ret);
    Sound_PlayCmd(m, cmd);
    r->a[7] += 6u;
}

/* the same through the forced dispatcher 0x3B66 */
static void play_forced(const f3mem_t *m, f3regs_t *r, uint32_t ret, uint16_t cmd)
{
    push_word(m, r, cmd);
    push_long(m, r, ret);
    Sound_PlayCmdForced(m, (int16_t)cmd);
    r->a[7] += 6u;
}

/* 0x1A05A8/0x1A0672/0x1A0740/0x1A0870: while bit 4 of the P2 switch byte is
 * clear the sound last played is retriggered and D3 is played; the new "last
 * played" value is stored either way. */
static void confirm_sound(const f3mem_t *m, f3regs_t *r, uint32_t ret_forced,
                          uint32_t ret_cmd)
{
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
    if (r->d[0] & 0xFFu)
        return;
    r->d[0] = r->d[7];
    r->d[0] += 1u;                      /* move.l D7,D0 ; addq.l #1,D0 ; beq */
    if (r->d[0] != 0u)
        play_forced(m, r, ret_forced, (uint16_t)r->d[7]);    /* jsr 0x3B66 */
    play_cmd(m, r, ret_cmd, (int16_t)(uint16_t)r->d[3]);     /* jsr 0x39D8 */
    r->d[7] = r->d[3];
}

/* 0x1A0564 row 0 (0x1A057C): the sound number. A new sound refills the eight
 * track volumes from its record (0x7F each while the record's type byte is
 * above 1, otherwise 0). */
static void row_sound_no(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[2] == r->d[4]) {               /* 0x1A057C cmp.l D4,D2 */
        r->d[0] = rd8(m, P1_SWITCH);        /* 0x1A0592 moveq #0/move.b */
        edit_value(m, r, 0x001A05A0u, (uint16_t)r->d[0], r->d[3], 1u,
                   rd16(m, 0x00006750u), 1u);
        r->a[7] += 0xAu;                    /* 0x1A05A0 */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.l */
        r->d[3] = r->d[0];                  /* 0x1A05A6 move.l D0,D3 */
        confirm_sound(m, r, 0x001A05C2u, 0x001A05CCu);  /* 0x1A05B2 */
        /* 0x1A05D0: the compare runs on both arms of the branch above */
        if (r->d[3] != rd32(m, L_LASTSND)) {
            wr32(m, L_LASTSND, r->d[3]);    /* 0x1A05D6 */
            r->a[3] = rd32(m, DIRECTORY + 4u * r->d[3]);    /* 0x1A05DA */
            if ((int8_t)rd8(m, r->a[3]) > 1) {          /* 0x1A05E6 bgt */
                r->d[6] = 0u;                           /* 0x1A05FA */
                for (;;) {
                    wr16(m, L_TRACK(r->d[6]), 0u);      /* 0x1A05FC clr.w */
                    r->d[6] += 1u;
                    r->d[0] = r->d[6];
                    r->d[0] -= 8u;
                    if ((int32_t)r->d[0] >= 0)          /* 0x1A0606 blt */
                        break;
                }
            } else {
                r->d[6] = 0u;                           /* 0x1A05E8 */
                for (;;) {
                    wr16(m, L_TRACK(r->d[6]), 0x7Fu);   /* 0x1A05EA */
                    r->d[6] += 1u;
                    r->d[0] = r->d[6];
                    r->d[0] -= 8u;
                    if ((int32_t)r->d[0] >= 0)          /* 0x1A05F6 blt */
                        break;
                }
            }
        }
    }
    /* 0x1A0608: "SOUND NO    :" and the value as %2.2X */
    push_long(m, r, r->d[3]);
    push_long(m, r, 0x001A209Du);
    push_word(m, r, (uint16_t)r->d[5]);
    push_long(m, r, 0x0061C000u + (r->d[2] * 2u) * 0x80u + 0x208u);
    draw_format(m, r, 0x001A0620u);
    r->a[7] += 0xEu;                        /* 0x1A0620 lea 0xE(SP) */
}

/* 0x1A0564 row 1 (0x1A0628): the code volume and its 0x10-slot bar. */
static void row_code_volume(const f3mem_t *m, f3regs_t *r)
{
    r->a[3] = rd32(m, DIRECTORY + 4u * r->d[3]);    /* 0x1A0628 */
    if ((int8_t)rd8(m, r->a[3]) > 2)                /* 0x1A0630 bgt */
        wr32(m, L_CODEVOL, 0u);                     /* 0x1A0642 clr.l */
    else                                            /* 0x1A0636 move.b 2(A3) */
        wr32(m, L_CODEVOL, (uint32_t)(int32_t)(int8_t)rd8(m, r->a[3] + 2u));

    if (r->d[2] == r->d[4]) {                       /* 0x1A0646 cmp.l D4,D2 */
        r->d[0] = rd8(m, P1_SWITCH);
        edit_value(m, r, 0x001A0668u, (uint16_t)r->d[0],
                   rd32(m, L_CODEVOL), 0u, 0x7Fu, 1u);
        r->a[7] += 0xAu;                            /* 0x1A0668 */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
        wr32(m, L_CODEVOL, r->d[0]);                /* 0x1A066E */
        confirm_sound(m, r, 0x001A068Cu, 0x001A0696u);  /* 0x1A067C */
    }
    {
        const uint32_t dst = (r->d[2] * 2u) * 0x80u;    /* row * 0x100 */
        push_long(m, r, 0x001A2101u);                   /* 0x1A069A "MIN  MAX" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x1A2u);
        draw_format(m, r, 0x001A06B0u);
        push_long(m, r, 0x001A2112u);                   /* 0x1A06B0 "CODE VOLUME :" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x208u);
        draw_format(m, r, 0x001A06C6u);
        push_long(m, r, rd32(m, L_CODEVOL));            /* 0x1A06C6 the value */
        push_long(m, r, 0x001A2120u);                   /* "%2.2X" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x242u);
        draw_format(m, r, 0x001A06E0u);
        draw_bar(m, r, 0x001A06FAu, 0x0061C000u + dst + 0x222u,
                 rd32(m, L_CODEVOL), 0x10u, (uint16_t)r->d[5]);
        r->a[7] += 0x30u;                               /* 0x1A06FA */
    }
    if ((int8_t)rd8(m, r->a[3]) > 2)                    /* 0x1A06FE bgt */
        wr32(m, L_CODEVOL, 0u);                         /* 0x1A0710 clr.l */
    else
        wr8(m, r->a[3] + 2u, (uint8_t)rd32(m, L_CODEVOL));  /* 0x1A0708 */
}

/* 0x1A0564 rows 2..9 (0x1A0718): one track volume and its 0x10-slot bar. */
static void row_track(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[2] == r->d[4]) {                       /* 0x1A0718 cmp.l D4,D2 */
        r->d[0] = rd8(m, P1_SWITCH);
        edit_value(m, r, 0x001A0726u, (uint16_t)r->d[0],
                   rd16(m, L_TRACKROW(r->d[2])), 0u, 0x7Fu, 1u);
        r->a[7] += 0xAu;                            /* 0x1A0738 */
        wr16(m, L_TRACKROW(r->d[2]), (uint16_t)r->d[0]);    /* 0x1A073C */
        confirm_sound(m, r, 0x001A075Au, 0x001A0764u);  /* 0x1A074A */
    }
    {
        const uint32_t dst = (r->d[2] * 2u) * 0x80u;
        push_long(m, r, 0x001A2101u);                   /* 0x1A0768 "MIN  MAX" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x1A2u);
        draw_format(m, r, 0x001A077Eu);
        push_long(m, r, r->d[2] - 1u);                  /* 0x1A0780 pea -1(A0) */
        push_long(m, r, 0x001A2126u);                   /* "TRACK %1d     :" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x208u);
        draw_format(m, r, 0x001A079Au);
        push_long(m, r, (uint32_t)(int32_t)(int16_t)rd16(m, L_TRACKROW(r->d[2])));
        push_long(m, r, 0x001A2120u);                   /* "%2.2X" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x242u);
        draw_format(m, r, 0x001A07B6u);
        draw_bar(m, r, 0x001A07D2u, 0x0061C000u + dst + 0x222u,
                 (uint32_t)(int32_t)(int16_t)rd16(m, L_TRACKROW(r->d[2])),
                 0x10u, (uint16_t)r->d[5]);
        r->a[7] += 0x34u;                               /* 0x1A07D2 */
    }
}

/* 0x1A0564 row 10 (0x1A07DA): the master volume and its 8-slot bar. */
static void row_master_volume(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[2] == r->d[4]) {                       /* 0x1A07DA cmp.l D4,D2 */
        r->d[0] = rd8(m, P1_SWITCH);
        edit_value(m, r, 0x001A07FCu, (uint16_t)r->d[0],
                   rd32(m, L_MASTERV), 0u, 0x3Fu, 1u);
        r->a[7] += 0xAu;                            /* 0x1A07FC */
        r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.l */
        wr32(m, L_MASTERV, r->d[0]);                /* 0x1A0802 */
    }
    {
        const uint32_t dst = (r->d[2] * 2u) * 0x80u;
        push_long(m, r, 0x001A20B0u);                   /* 0x1A0806 "MIN  MAX" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x1A2u);
        draw_format(m, r, 0x001A081Cu);
        push_long(m, r, 0x001A20B9u);                   /* 0x1A081C "SOUND VOLUME:" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x208u);
        draw_format(m, r, 0x001A0832u);
        push_long(m, r, rd32(m, L_MASTERV));            /* 0x1A0832 the value */
        push_long(m, r, 0x001A2120u);                   /* "%2.2X" */
        push_word(m, r, (uint16_t)r->d[5]);
        push_long(m, r, 0x0061C000u + dst + 0x232u);
        draw_format(m, r, 0x001A084Cu);
        draw_bar(m, r, 0x001A0866u, 0x0061C000u + dst + 0x222u,
                 rd32(m, L_MASTERV), 8u, (uint16_t)r->d[5]);
        r->a[7] += 0x30u;                               /* 0x1A0866 */
    }
}

/* 0x1A0564 row 11 (0x1A086C): EXIT. Bit 4 of the P2 switch byte on the
 * selected row arms the exit flag word -0x6(A6). */
static void row_exit(const f3mem_t *m, f3regs_t *r)
{
    if (r->d[2] == r->d[4]) {                       /* 0x1A086C cmp.l D4,D2 */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x10u);
        if (!(r->d[0] & 0xFFu))                     /* 0x1A087A bne */
            wr16(m, L_EXIT, 1u);                    /* 0x1A087C move.w #1 */
    }
    push_long(m, r, 0x001A20C7u);                   /* 0x1A0882 "EXIT" */
    push_word(m, r, (uint16_t)r->d[5]);
    push_long(m, r, 0x0061C000u + (r->d[2] * 2u) * 0x80u + 0x208u);
    draw_format(m, r, 0x001A0898u);
    r->a[7] += 0xAu;                                /* 0x1A0898 */
}

/* 0x1A04DA..0x1A0902: one frame of the editor. Returns SEG_YIELD1 unless the
 * EXIT row armed -0x6(A6), which returns SEG_YIELD2. */
static uint32_t frame_step(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1A04DA: bit 1 of the P2 switch byte steps the row down */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x02u);
    if (!(r->d[0] & 0xFFu)) {                       /* 0x1A04E4 bne */
        r->d[4] += 1u;                              /* 0x1A04E6 */
        if ((int32_t)r->d[4] >= (int32_t)rd32(m, L_ROWS))
            r->d[4] = 0u;                           /* 0x1A04EE */
        r->a[3] = rd32(m, DIRECTORY + 4u * r->d[3]);        /* 0x1A04F0 */
        if ((int8_t)rd8(m, r->a[3]) > 1) {                  /* 0x1A04FC ble */
            r->d[0] = r->d[4];
            r->d[0] -= 2u;                      /* 0x1A0500 subq.l #2,D0 */
            if ((int32_t)r->d[0] >= 0) {        /* 0x1A0502 blt 0x1A050C */
                r->d[0] = 9u;                   /* 0x1A0504 moveq #9,D0 */
                if ((int32_t)r->d[0] >= (int32_t)r->d[4])   /* 0x1A0508 blt */
                    r->d[4] = 0xAu;             /* 0x1A050A moveq #0xA,D4 */
            }
        }
    }
    /* 0x1A050C: bit 0 of the P2 switch byte steps the row up */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (rd8(m, P2_SWITCH) & 0x01u);
    if (!(r->d[0] & 0xFFu)) {                       /* 0x1A0516 bne */
        r->d[4] -= 1u;                              /* 0x1A0518 */
        if ((int32_t)r->d[4] < 0) {                 /* 0x1A051A bpl */
            r->d[4] = 0xFFFFFFFFu;                  /* 0x1A051C */
            r->d[4] += rd32(m, L_ROWS);             /* 0x1A051E */
        }
        r->a[3] = rd32(m, DIRECTORY + 4u * r->d[3]);        /* 0x1A0522 */
        if ((int8_t)rd8(m, r->a[3]) > 1) {                  /* 0x1A052E ble */
            r->d[0] = r->d[4];
            r->d[0] -= 2u;                      /* 0x1A0532 */
            if ((int32_t)r->d[0] >= 0) {        /* 0x1A0534 blt */
                r->d[0] = 9u;                   /* 0x1A0536 */
                if ((int32_t)r->d[0] >= (int32_t)r->d[4])   /* 0x1A053A blt */
                    r->d[4] = 1u;               /* 0x1A053C moveq #1,D4 */
            }
        }
    }

    /* 0x1A053E: one row per line, D5 = 8 for the selected row. */
    {
        const uint32_t rows = rd32(m, L_ROWS);
        for (r->d[2] = 0u; (int32_t)r->d[2] < (int32_t)rows; r->d[2] += 1u) {
            r->d[5] = (r->d[2] == r->d[4]) ? 8u : 7u;   /* 0x1A0544..0x1A054C */
            r->d[0] = r->d[2];
            if (r->d[0] > 0xBu)                         /* 0x1A0556 bhi */
                continue;
            {
                int32_t idx = (int32_t)(int16_t)(uint16_t)r->d[0];
                uint16_t off = rd16(m, (uint32_t)(0x001A0564u + 2 * idx));
                seg_setw(&r->d[0], off);                /* 0x1A055A move.w */
                switch (0x001A0564u + (uint32_t)(int16_t)off) {
                case 0x1A057Cu: row_sound_no(m, r); break;
                case 0x1A0628u: row_code_volume(m, r); break;
                case 0x1A0718u: row_track(m, r); break;
                case 0x1A07DAu: row_master_volume(m, r); break;
                case 0x1A086Cu: row_exit(m, r); break;
                }
            }
        }
    }

    /* 0x1A08A6: the 0x64E8 compare is dead (D0 is -1); with a record type
     * byte above 1 the eight track volumes are not queued. */
    r->a[3] = rd32(m, DIRECTORY + 4u * r->d[3]);    /* 0x1A08B0 */
    if ((int8_t)rd8(m, r->a[3]) <= 1) {             /* 0x1A08BC bgt 0x1A08DC */
        r->d[6] = 0u;                                   /* 0x1A08BE */
        for (;;) {
            push_word(m, r, rd16(m, L_TRACK(r->d[6])));     /* 0x1A08C0 */
            push_word(m, r, (uint16_t)(r->d[6] + 1u));      /* 0x1A08C8 */
            push_word(m, r, (uint16_t)r->d[3]);             /* 0x1A08CA */
            push_long(m, r, 0x001A08D2u);
            (void)Fn_003FD8(m, r);                          /* 0x1A08CC */
            r->a[7] += 0xAu;                /* rts + 0x1A08D2 addq.l #6,SP */
            r->d[6] += 1u;                  /* 0x1A08D4 */
            r->d[0] = r->d[6];
            r->d[0] -= 8u;                  /* 0x1A08D8 subq.l #8,D0 */
            if ((int32_t)r->d[0] < 0)       /* 0x1A08DA blt */
                break;
        }
    }

    /* 0x1A08DC: publish the master volume and send it to the chip */
    r->d[0] = rd32(m, L_MASTERV);
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];   /* ext.w D0 */
    wr16(m, VOLUME, (uint16_t)r->d[0]);             /* 0x1A08E2 */
    for (unsigned i = 0; i < 4u; ++i)
        push_word(m, r, rd16(m, VOLUME));
    push_long(m, r, 0x001A0902u);
    {
        uint8_t v = (uint8_t)r->d[0];
        Sound_SetVolume(m, v, v, v, v);             /* 0x1A08FC jsr 0x3CBA */
    }
    r->a[7] += 0xCu;                    /* rts + 0x1A0902 addq.l #8,SP */

    if (rd16(m, L_EXIT) == 0u)                      /* 0x1A0904 tst.w */
        return SEG_YIELD1;                          /* 0x1A0908 beq */

    /* 0x1A090C: leaving - retrigger the last sound and clear the layers */
    r->d[0] = r->d[7];
    r->d[0] += 1u;
    if (r->d[0] != 0u)                              /* 0x1A0910 beq */
        play_forced(m, r, 0x001A091Au, (uint16_t)r->d[7]);
    return SEG_YIELD2;                              /* 0x1A091C jsr 0x001F62 */
}

uint32_t Seg_1A043C(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x1A043Cu:
        save_frame(m, r);
        r->a[2] = 0x0061C000u;      /* 0x1A0444 text RAM base */
        r->a[4] = 0x001A1BA6u;      /* 0x1A044A jsr (A4) formats and draws */
        r->d[0] = 0xCu;
        wr32(m, L_ROWS, r->d[0]);   /* 0x1A0452 12 rows */
        r->d[4] = 0u;                       /* the selected row */
        r->d[3] = 0u;                       /* the selected sound */
        r->d[7] = 0xFFFFFFFFu;              /* the sound last played */
        r->d[0] = 0xFFFFFFFFu;
        wr32(m, L_LASTSND, r->d[0]);        /* 0x1A045E */
        wr32(m, L_CODEVOL, 0u);             /* 0x1A0462 */
        wr32(m, L_MASTERV, 0u);             /* 0x1A0466 */
        wr16(m, L_EXIT, 0u);                /* 0x1A046A */
        Gfx_ClearAllLayers(m);              /* 0x1A046E */
        for (r->d[2] = 0u; ; ) {                    /* 0x1A0476 */
            uint32_t off = 4u * r->d[2];
            wr32(m, 0x0061E200u + off, rd32(m, 0x001A1C78u + off));
            r->d[2] += 1u;                          /* 0x1A0484 addq.l #1,D2 */
            r->d[0] = 0x48u;                        /* 0x1A0486 moveq #0x48,D0 */
            if ((int32_t)r->d[2] >= (int32_t)r->d[0])   /* 0x1A048A blt */
                break;
        }
        play_cmd(m, r, 0x001A0494u, 0);      /* 0x1A048C clr.w -(SP) ; jsr */
        push_long(m, r, 0x001A049Cu);
        settings_apply_4D06(m, r);           /* 0x1A0496 jsr 0x004D06 */
        push_long(m, r, 0x001A04A2u);
        (void)Fn_004088(m, r);               /* 0x1A049C jsr 0x004088 */
        r->a[0] = (uint32_t)(int32_t)(int16_t)rd16(m, VOLUME);  /* 0x1A04A2 */
        wr32(m, L_MASTERV, r->a[0]);         /* 0x1A04A8 */
        r->d[3] = 1u;                        /* 0x1A04AC */
        r->d[2] = 0u;                        /* 0x1A04AE moveq #0,D2 */
        for (;;) {
            wr16(m, L_TRACK(r->d[2]), 0u);   /* 0x1A04B0 clr.w */
            r->d[2] += 1u;                   /* 0x1A04B4 addq.l #1,D2 */
            r->d[0] = r->d[2];
            r->d[0] -= 8u;                   /* 0x1A04B8 subq.l #8,D0 */
            if ((int32_t)r->d[0] >= 0)       /* 0x1A04BA blt */
                break;
        }
        push_long(m, r, 0x001A2092u);        /* 0x1A04BC "SOUND TEST" */
        push_word(m, r, 0xAu);
        push_long(m, r, 0x0061C09Cu);
        draw_format(m, r, 0x001A04CCu);
        r->a[7] += 0xAu;                     /* 0x1A04CC */
        return SEG_YIELD1;                   /* 0x1A04D0 bra 0x1A0904 */

    case 0x1A04DAu:
        return frame_step(m, r);

    case 0x1A0922u:
        /* 0x1A0922: the volume byte, then 0x4DEE, whose write-back enters the
         * reset loop when the serial device does not answer - the harness has
         * no I/O space, so this path never returns (as in seg_1A0A52.c). */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, VOLUME);
        wr8(m, 0x00406C62u, (uint8_t)r->d[0]);
        push_long(m, r, 0x001A0934u);
        if (settings_refresh_4DEE(m, r) < 0)
            return SEG_LEAVE;               /* 0x004E3C jsr 0x01014E */
        Gfx_ClearAllLayers(m);              /* 0x1A0934 */
        return restore_frame(m, r);         /* 0x1A093A..0x1A0940 */
    }
    return SEG_LEAVE;
}
