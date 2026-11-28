/* ROM 0x010172: UNVERIFIED (no capture, and none is possible for the entry):
 * kernel task 0 (Task0_Main) -- the boot task that clears the machine, arms
 * the two display/sound init routines, then runs the attract/credit state
 * machine, and switches to test mode when the service switches or a bad
 * settings block ask for it.
 *
 * UNVERIFIABLE at 0x010172 (the only instruction an entry capture could show):
 * the body starts with `trap #5` (0x010172), which pushes the 6-byte frame
 * {PC = 0x010174, SR} and hands the CPU to the TRAP #5 body at 0x001F78, whose
 * Kern_Schedule never returns to that push -- it saves the context and switches
 * task. So F3_EXIT=010174 never fires inside this activation: 0x010174 is
 * reached only when the scheduler later resumes that saved context -- a later
 * frame, after a scheduler round trip -- so test_fn skips the pair (fpre !=
 * fpost) and the post state is not this entry's anyway. F3_INJECT_EXC does not
 * help either: it
 * does not synthesise a TRAP -- it plants an 8-byte format-0 frame and starts
 * the routine at 0x010172, so the real `trap #5` still executes on top of it
 * and still yields (and the frame it leaves is the trap's 6-byte one, not the
 * synthetic one; the "ret" mode never sees its RTE). The closest reachable
 * stop is 0x002274 (F3_INJECT=010172
 * F3_EXIT=002274 F3_INJECT_ABORT=1), but that post is the scheduler entry after
 * Kern_001F78's context save -- it verifies the TRAP #5 body, which already has
 * its own capture (work/ram/fn_001F78, "Kernel 0x001F78 TRAP 5"), not this
 * task. Fn_010172 therefore only models the yield, and the first frame of the
 * task is verified separately through Task0_Main_Frame/seg captures.
 *
 * The body starts with TRAP #5 (0x010172), so Fn_010172 only yields. One pass
 * of the loop is Task0_Main_Frame, which runs the code after the yield up to
 * the next scheduling point:
 *
 *   010174  jsr 0x005A3A (font) ; jsr 0x0050DC (EEPROM settings check)
 *   010182  D0 != 0 -> test mode (0x010432)
 *   010186  "WAIT A MOMENT" at 0x61C79C, TRAP #4 sleep 0xB4 frames
 *   0101A8  sound 0, sound 0x94, latch two service-switch bits from A5-0x5DE4
 *           into A5-0x144A (inverted), TRAP #4 sleep 0xB4 frames
 *   0101FA  sound 0x95, CACR = 1 (cache, not modelled), volume word
 *           A5-0x5DC0 to 0xC007F8..FB, clear the text layer
 *   010236  font, clear A5-0x144C/0x144B/0x1452, stop the sound, rebuild the
 *           sprite buffers, sprites, layers, line RAM and sound bank, then
 *           TRAP #1 spawn <slot 4, 0x010488> and <slot 15, 0x0038AC>
 *   010282  TRAP #5 -> resume 0x010284
 *   010284  A5-0x1444 = A5-0x5DC5, TRAP #1 spawn <slot 1, 0x0B8404>
 *   01029A  TRAP #5 -> resume 0x01029C
 *   01029C  per-frame select (0x01033C): while the coin/credit state asks to
 *           keep waiting it yields again at 0x01029A; otherwise the
 *           0x0102EE reset runs (stop sound, sprites, layers, TRAP #2 kill
 *           0x7FFE, TRAP #1 spawn <slot 1, 0x004146>, A5-0x1444 = 0) and the
 *           task yields at 0x010320
 *   010322  per-frame select again; A5-0x144B bit 0 set -> TRAP #2 kill
 *           0xFFFE and the 0x010236 init runs again; else yield at 0x010320
 *   010432  test-mode entry: TRAP #2 kill 0xFFFE, TRAP #1 spawn
 *           <slot 1, 0x1A1A26, param = long at ROM 0>, clear A5-0x5DC7,
 *           0x5DCC and 0x1444, sprites/layers/line RAM/sound bank, stop the
 *           sound, then TRAP #5 and bra 0x010474 forever.
 *
 * TRAP #1 and TRAP #2 are modelled with the kernel handlers (Kern_001E76 /
 * Kern_001E0E, 0x001E76 / 0x001E0E), including the 68020 format-0 frame the
 * handler reads its arguments from (SR 0x2000, the task SR of 0x1EF2). TRAP #4
 * sleep is the scheduler's job (the handler 0x001F30 saves the task context and
 * sets its timer): a pass that reaches one has already pushed the 0xB4 frame
 * count, ends there and reports the wake-up PC, whose first instruction is the
 * `addq.l #4,A7` that pops the argument.
 *
 * The jsr return addresses of Fn_005A3A (0x010174, 0x010236) are not written:
 * that helper leaves SP at its entry value rather than popping like its rts;
 * the frame is below SP and nothing reads it. Every other call, trap and jmp
 * writes its frame as the ROM does.
 *
 * 0x0014DA is translated register-level in fn_000402.c
 * (SoundBank0_0014DA_Regs) because its sign-extended destination index writes
 * through the 0x438000 work-RAM mirror; the shared void helper does not. */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString 0x005666 */
#include "../kernel/kernel.h"      /* Kern_001E76 / Kern_001E0E */
#include "../player/player_task.h" /* Coin_CheckStartReady (0x000FEC) */
#include "../video/sprite.h"

/* 0x005666 with the caller's arguments and its link/move frame. */
static void draw_string(const f3mem_t *m, f3regs_t *r, uint32_t src,
                        uint32_t dst, uint16_t col, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, col);                       /* 0x010186 move.w #7,-(A7) */
    push_long(m, r, dst);                       /* 0x01018A pea.l 0x61C79C */
    push_long(m, r, src);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    r->d[0] = Text_DrawString(m, src, dst, col);
    r->a[7] = sp;                               /* 0x01019C lea 0xA(A7),A7 */
}

/* ROM 0x0013F238 + 0x0039D8: the caller pushes the command word, 0x0013F238
 * rewrites it in place (+0x53 when A5+0x3306 is set and 1 <= cmd <= 0x40), and
 * 0x0039D8 queues it into the sound DPRAM ring. */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp[F3_DPRAM_SIZE] = {0};
        f3mem_t tm = *m;
        tm.dpram = tmp;
        Sound_PlayCmd(&tm, cmd);
    }
}

/* TRAP #1 with the two arguments the ROM pushes: entry PC (SP+8) and slot
 * (SP+12). The task parameter at SP+16 is whatever the caller's stack holds,
 * as in the ROM. */
static void trap01(const f3mem_t *m, f3regs_t *r, uint32_t slot,
                   uint32_t entry, uint32_t retpc)
{
    uint32_t sp = r->a[7];
    push_long(m, r, slot);                      /* move.l #slot,-(A7) */
    push_long(m, r, entry);                     /* pea.l entry */
    r->a[7] -= 8u;                              /* 68020 format-0 trap frame */
    wr16(m, r->a[7], 0x2000u);                  /* SR of the task context (0x1EF2) */
    wr32(m, r->a[7] + 2u, retpc);
    wr16(m, r->a[7] + 6u, 0x0084u);             /* format 0, vector 33 (trap #1) */
    (void)Kern_001E76(m, r);                    /* 0x001E76 */
    r->a[7] = sp;                               /* addq.l #8,A7 */
}

/* TRAP #1 with the task parameter pushed too (0x01043C..0x01044C). */
static void trap01_param(const f3mem_t *m, f3regs_t *r, uint32_t slot,
                         uint32_t entry, uint32_t param, uint32_t retpc)
{
    uint32_t sp = r->a[7];
    push_long(m, r, param);                     /* 0x01043C move.l (0).w,-(A7) */
    push_long(m, r, slot);                      /* 0x010440 move.l #1 */
    push_long(m, r, entry);                     /* 0x010446 pea.l */
    r->a[7] -= 8u;                              /* 68020 format-0 trap frame */
    wr16(m, r->a[7], 0x2000u);
    wr32(m, r->a[7] + 2u, retpc);
    wr16(m, r->a[7] + 6u, 0x0084u);             /* format 0, vector 33 (trap #1) */
    (void)Kern_001E76(m, r);
    r->a[7] = sp;                               /* 0x01044E lea 0xC(A7),A7 */
}

/* TRAP #2: kill the tasks selected by the mask (0x001E0E). */
static void trap02(const f3mem_t *m, f3regs_t *r, uint32_t mask, uint32_t retpc)
{
    uint32_t sp = r->a[7];
    push_long(m, r, mask);                      /* move.l #mask,-(A7) */
    r->a[7] -= 8u;                              /* 68020 format-0 trap frame */
    wr16(m, r->a[7], 0x2000u);
    wr32(m, r->a[7] + 2u, retpc);
    wr16(m, r->a[7] + 6u, 0x0088u);             /* format 0, vector 34 (trap #2) */
    (void)Kern_001E0E(m, r);                    /* 0x001E0E */
    r->a[7] = sp;                               /* addq.l #4,A7 */
}

/* bsr.w 0x01033C: the routine pops its own return address; on its -1
 * (test-mode) return nothing was written yet, so the frame is dropped here. */
static int call_01033C(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_long(m, r, ret);
    int rc = Fn_01033C(m, r);
    r->a[7] = sp;
    return rc;
}

static uint32_t pass_10174(const f3mem_t *m, f3regs_t *r);
static uint32_t pass_101A8(const f3mem_t *m, f3regs_t *r);
static uint32_t pass_101FA(const f3mem_t *m, f3regs_t *r);
static uint32_t pass_10236(const f3mem_t *m, f3regs_t *r);
static uint32_t pass_1029C(const f3mem_t *m, f3regs_t *r);
static uint32_t pass_10322(const f3mem_t *m, f3regs_t *r);
static uint32_t pass_10432(const f3mem_t *m, f3regs_t *r);

/* 0x010432: test-mode entry (reached from 0x010182 or from 0x01033C's -1). */
static uint32_t pass_10432(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    trap02(m, r, 0x0000FFFEu, 0x01043Au);       /* 0x010432..0x01043A */
    trap01_param(m, r, 1u, 0x001A1A26u, rd32(m, 0x00000000u), 0x01044Eu); /* 0x01043C */
    wr8(m, a5 - 0x5DC7u, 0x00u);                /* 0x010452 */
    wr8(m, a5 - 0x5DCCu, 0x00u);                /* 0x010456 */
    wr8(m, a5 - 0x1444u, 0x00u);                /* 0x01045A */
    Sprite_ClearAll(m);                         /* 0x01045E jsr 0x0000117E */
    Gfx_ClearAllLayers(m);                      /* 0x010462 jsr 0x00005A12 */
    Gfx_InitLineRam(m);                         /* 0x010466 jsr 0x00005B0C */
    SoundBank0_0014DA_Regs(m, r);               /* 0x01046A jsr 0x000014DA */
    Sound_StopAll(m);                           /* 0x01046E jsr 0x00003C24 */
    /* 0x010474 trap #5 ; 0x010476 bra.s 0x010474: yield forever. */
    return 0x010476u;
}

/* 0x010236..0x010282: rebuild the machine and spawn the two init tasks. */
static uint32_t pass_10236(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    (void)Fn_005A3A(m, r);                      /* 0x010236 jsr 0x00005A3A */
    wr8(m, a5 - 0x144Cu, 0x00u);                /* 0x01023C sf.b */
    wr8(m, a5 - 0x144Bu, 0x00u);                /* 0x010240 */
    wr8(m, a5 - 0x1452u, 0x00u);                /* 0x010244 */
    Sound_StopAll(m);                           /* 0x010248 jsr 0x00003C24 */
    Sprite_InitBuffers(m);                      /* 0x01024E jsr 0x00001070 */
    Sprite_ClearAll(m);                         /* 0x010252 jsr 0x0000117E */
    Gfx_ClearAllLayers(m);                      /* 0x010256 jsr 0x00005A12 */
    Gfx_InitLineRam(m);                         /* 0x01025A jsr 0x00005B0C */
    SoundBank0_0014DA_Regs(m, r);               /* 0x01025E jsr 0x000014DA */
    trap01(m, r, 4u, 0x00010488u, 0x010270u);   /* 0x010262..0x01026E */
    trap01(m, r, 0x0Fu, 0x000038ACu, 0x010280u);/* 0x010272..0x01027E */
    return 0x010284u;                           /* 0x010282 trap #5 */
}

/* 0x01008A-style startup, first half: font, settings check, "WAIT A MOMENT"
 * and the first sleep. */
static uint32_t pass_10174(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];

    (void)Fn_005A3A(m, r);                      /* 0x010174 jsr 0x00005A3A */
    push_long(m, r, 0x010180u);                 /* 0x01017A jsr 0x000050DC */
    int rc = Fn_0050DC(m, r);
    r->a[7] = sp;
    if (rc < 0)
        return 0u;                              /* settings recovery: untranslated */
    if (r->d[0] != 0u)                          /* 0x010180 tst.l D0 / bne */
        return pass_10432(m, r);

    draw_string(m, r, 0x001047Au, 0x0061C79Cu, 7u, 0x01019Cu); /* 0x010186 */
    /* 0x0101A0 move.l #0xB4,-(A7) ; 0x0101A6 trap #4: sleep 180 frames. The
     * CPU frame and the handler (0x001F30) belong to the scheduler, as with
     * every other yield; 0x0101A8 addq.l #4,A7 pops the argument on wake. */
    push_long(m, r, 0x000000B4u);
    return 0x0101A8u;
}

/* 0x0101A8: the two sound cues, the service-switch latch and the second
 * sleep. */
static uint32_t pass_101A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    r->a[7] += 4u;                              /* 0x0101A8 addq.l #4,A7 */
    play_sound(m, 0x0000u);                     /* 0x0101AA..0x0101BA */
    play_sound(m, 0x0094u);                     /* 0x0101BE..0x0101CE */

    /* 0x0101D2 sf.b ; 0x0101D6 btst #4 / seq / bfins {0:1} ;
     * 0x0101E4 btst #3 / seq / bfins {1:1}: the two switch bits, inverted
     * into the top two bits of A5-0x144A. */
    wr8(m, a5 - 0x144Au, 0x00u);
    uint8_t flags = rd8(m, a5 - 0x5DE4u);
    wr8(m, a5 - 0x144Au, (uint8_t)((flags & 0x10u) ? 0x00u : 0x80u));
    wr8(m, a5 - 0x144Au,
        (uint8_t)(((flags & 0x10u) ? 0x00u : 0x80u) |
                  ((flags & 0x08u) ? 0x00u : 0x40u)));
    /* 0x0101DC/0x0101EA seq.b D0: 0xFF when the tested bit was clear. */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)((flags & 0x08u) ? 0x00u : 0xFFu);

    /* 0x0101F2 move.l #0xB4,-(A7) ; 0x0101F8 trap #4: sleep 180 frames. */
    push_long(m, r, 0x000000B4u);
    return 0x0101FAu;
}

/* 0x0101FA: third sound cue, cache enable, volume, text clear, then the
 * 0x010236 rebuild. */
static uint32_t pass_101FA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint8_t vol;

    r->a[7] += 4u;                              /* 0x0101FA addq.l #4,A7 */
    play_sound(m, 0x0095u);                     /* 0x0101FC..0x01020C */
    r->d[0] = 1u;                               /* 0x010210 moveq #1,D0 ;
                                                 * 0x010212 movec D0,CACR */
    /* 0x010216..0x010222: the word at A5-0x5DC0 pushed four times; 0x003CBA
     * copies the low byte of each copy to 0xC007F8..FB. */
    for (unsigned i = 0; i < 4u; ++i)
        push_word(m, r, rd16(m, a5 - 0x5DC0u));
    vol = rd8(m, a5 - 0x5DBFu);                 /* = the low byte pushed above */
    Sound_SetVolume(m, vol, vol, vol, vol);     /* 0x010226 jsr 0x003CBA */
    r->a[7] += 8u;                              /* 0x01022C lea (8,A7),A7 */
    Gfx_ClearTextLayer(m);                      /* 0x010230 jsr 0x00005880 */
    return pass_10236(m, r);                    /* 0x010236 */
}

/* 0x010284: latch the start-ready flag and spawn the object runner. */
static uint32_t pass_10284(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    wr8(m, a5 - 0x1444u, rd8(m, a5 - 0x5DC5u)); /* 0x010284 move.b (A5-0x5DC5) */
    trap01(m, r, 1u, 0x000B8404u, 0x010298u);   /* 0x01028A..0x010296 */
    return 0x01029Cu;                           /* 0x01029A trap #5 */
}

/* 0x0102EE: full reset of the screen state, then spawn the next routine. */
static uint32_t pass_scene_reset(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    Sound_StopAll(m);                           /* 0x0102EE jsr 0x00003C24 */
    Sprite_ClearAll(m);                         /* 0x0102F4 jsr 0x0000117E */
    Gfx_ClearAllLayers(m);                      /* 0x0102FA jsr 0x00005A12 */
    trap02(m, r, 0x00007FFEu, 0x010308u);       /* 0x010300..0x010308 */
    trap01(m, r, 1u, 0x00004146u, 0x010318u);   /* 0x01030A..0x010316 */
    wr8(m, a5 - 0x1444u, 0x00u);                /* 0x01031A move.b #0 */
    return 0x010322u;                           /* 0x010320 trap #5 */
}

/* 0x01029C: the per-frame coin/credit select. Every "not yet" branch lands on
 * the TRAP #5 at 0x01029A and comes back at 0x01029C. */
static uint32_t pass_1029C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    if (call_01033C(m, r, 0x0102A0u) < 0)
        return pass_10432(m, r);                /* 0x010420 -> 0x010432 */

    if (rd8(m, a5 - 0x5DC3u) != 0u) {           /* 0x0102A0 tst.b / beq 0x0102B4 */
        r->d[0] = set_word(r->d[0], rd16(m, a5 - 0x1456u));          /* 0x0102A8 */
        r->d[0] = set_word(r->d[0], (uint16_t)((uint16_t)r->d[0] & 0x07FFu));
        if ((uint16_t)r->d[0] == 0u)            /* 0x0102B0 beq 0x0102EE */
            return pass_scene_reset(m, r);
    }

    if (rd8(m, a5 - 0x5DC1u) == 0u) {           /* 0x0102B4 tst.b / beq 0x0102E4 */
        r->d[0] = set_word(r->d[0], Coin_CheckStartReady(m)); /* 0x0102E4 jsr */
        if ((int8_t)r->d[0] < 0)                /* 0x0102EA tst.b / blt 0x01029A */
            return 0x01029Cu;
        return pass_scene_reset(m, r);
    }

    wr16(m, a5 + 0x1D38u, 0u);                  /* 0x0102BC move.w #0 */
    if ((rd8(m, a5 - 0x5DDBu) & 0x80u) == 0u)   /* 0x0102C2 btst #7 / beq */
        return pass_scene_reset(m, r);
    if (rd8(m, a5 - 0x5DC6u) != 0u)             /* 0x0102CC tst.b / bne 0x01029A */
        return 0x01029Cu;
    wr16(m, a5 + 0x1D38u, 1u);                  /* 0x0102D2 move.w #1 */
    if ((rd8(m, a5 - 0x5DD7u) & 0x80u) == 0u)   /* 0x0102D8 btst #7 / beq */
        return pass_scene_reset(m, r);
    return 0x01029Cu;                           /* 0x0102E2 bra 0x01029A */
}

/* 0x010322: after the reset pass, wait for A5-0x144B bit 0, then kill the
 * tasks and run the 0x010236 init again. */
static uint32_t pass_10322(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    if (call_01033C(m, r, 0x010326u) < 0)
        return pass_10432(m, r);
    if ((rd8(m, a5 - 0x144Bu) & 0x01u) == 0u)   /* 0x010326 btst #0 / beq 0x010320 */
        return 0x010322u;
    trap02(m, r, 0x0000FFFEu, 0x010336u);       /* 0x01032E..0x010336 */
    return pass_10236(m, r);                    /* 0x010338 bra.w 0x010236 */
}

int Fn_010172(const f3mem_t *m, f3regs_t *r)
{
    /* 0x010172 trap #5: the task yields before its first instruction, so the
     * body starts at 0x010174 (Task0_Main_Frame). */
    (void)m;
    (void)r;
    return 0;
}

uint32_t Task0_Main_Frame(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x010174u: return pass_10174(m, r);
    case 0x0101A8u: return pass_101A8(m, r);
    case 0x0101FAu: return pass_101FA(m, r);
    case 0x010236u: return pass_10236(m, r);
    case 0x010284u: return pass_10284(m, r);
    case 0x01029Cu: return pass_1029C(m, r);
    case 0x010322u: return pass_10322(m, r);
    case 0x010432u: return pass_10432(m, r);
    case 0x010476u: return 0x010476u;   /* 0x010476 bra.s 0x010474: yields forever */
    default: return 0u;
    }
}
