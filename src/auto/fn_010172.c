/* ROM 0x010172: Task0_Main, the boot task.
 * Verified against power-on captures in work/ram/seg0_* from resume PC to
 * the next TRAP, including SR, SP, the stack and all captured RAM regions.
 *
 *   Resume   Stop     Pairs   Path
 *   010174   0101A6     1/1   Font, settings check, wait message and sleep
 *   0101A8   0101F8     1/1   Sound cues, service switches and second sleep
 *   0101FA   01026E     1/1   Display and sound rebuild, with IRQ3 replay
 *   010270   01027E     1/1   Prepare the second init task
 *   010280   010282     1/1   Drop spawn arguments and yield
 *   010284   010296     1/1   Set up the attract task
 *   010298   01029A     1/1   Drop spawn arguments and yield
 *   01029C   01029A   30/30   Attract/credit loop while waiting for start
 *
 * The start/reset continuations at 0x010308, 0x010318, 0x010322 and
 * 0x010336 and the separate test-mode branch at 0x010432 are uncaptured.
 * Fn_010172 models the initial TRAP #5 yield. Seg_Task0_Main stops before
 * each TRAP. Task0_Main_Frame executes synchronous TRAP #1/#2 handlers
 * and returns the resume PC for the scheduler's next TRAP #4/#5 yield.
 */
#include "../kernel/seg.h"
#include "fn_text_frames.h"
#include "../audio/sound_cmd.h"
#include "../kernel/kernel.h"
#include "../player/player_task.h"

/* MOVE/TST update NZVC and preserve X. Narrow operands are sign-extended
 * by their caller for the N test. */
static void move_flags(f3regs_t *r, int32_t value)
{
    r->sr = (uint16_t)((r->sr & ~0x0Fu) | (value < 0 ? 8u : value == 0 ? 4u : 0u));
}

static void save_call(const f3mem_t *m, f3regs_t *r, uint32_t ret, uint16_t mask)
{
    push_long(m, r, ret);
    push_movem(m, r, mask);
}

/* 0x13F238 can rewrite the pushed argument before 0x0039D8 queues it. */
static void play_sound(const f3mem_t *m, f3regs_t *r, int16_t cmd, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, (uint16_t)cmd);
    wr32(m, r->a[7] - 4u, ret - 6u);            /* 0x13F238 return PC */
    if (rd8(m, r->a[5] + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40) {
        cmd = (int16_t)(cmd + 0x53);
        wr16(m, r->a[7], (uint16_t)cmd);
    }
    save_call(m, r, ret, 0x0707u);               /* 0x0039D8 saves D0-D2/A0-A2 */
    if (rd8(m, r->a[5] - 0x1444u) == 0u && cmd >= 0 &&
        cmd <= (int16_t)rd16(m, 0x6750u)) {
        push_long(m, r, 0x003A16u);             /* command handler return PC */
        r->sr &= (uint16_t)~0x10u;              /* 0x003A16 add.w D1,D1 */
    }
    Sound_PlayCmd(m, cmd);
    r->a[7] = sp;
}

/* Execute a synchronous trap when running a whole frame. Its arguments
 * remain on the stack until the instruction at the resume PC removes them. */
static void run_trap(const f3mem_t *m, f3regs_t *r, uint32_t pc, unsigned vector)
{
    const uint16_t sr = r->sr;
    push_word(m, r, (uint16_t)(0x80u + 4u * vector));
    push_long(m, r, pc + 2u);
    push_word(m, r, sr);
    uint32_t frame = r->a[7];
    push_movem(m, r, vector == 1u ? 0x03C0u : 0x0003u);
    r->a[7] = frame;
    if (vector == 1u)
        (void)Kern_001E76(m, r);
    else
        (void)Kern_001E0E(m, r);
    r->a[7] = frame + 8u;
}

static void stop_sound(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    save_call(m, r, ret, 0x071Fu);               /* 0x003C24 saves D0-D4/A0-A2 */
    Sound_StopAll(m);
    r->a[7] = sp;
    r->sr &= (uint16_t)~0x10u;                  /* 0x003CAE add.w D1,D1 */
}

static void clear_sprites(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    save_call(m, r, ret, 0x0303u);               /* 0x00117E saves D0-D1/A0-A1 */
    Sprite_ClearAll(m);
    r->a[7] = sp;
}

/* 0x005880: the text clear preserves D0-D1/A0. Its loop can be
 * interrupted after the store, with A0 already advanced to the next long. */
static int clear_text(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      task0_checkpoint_t checkpoint)
{
    const f3regs_t saved = *r;
    save_call(m, r, ret, 0x0103u);
    r->a[0] = 0x61C000u;

    if (checkpoint) {
        r->d[0] = set_word(r->d[0], 0x07FFu);
        r->d[1] = 0;
        r->pc = 0x005892u;
        move_flags(r, 0);
        do {
            wr32(m, r->a[0], r->d[1]);
            r->a[0] += 4u;
            if (checkpoint(m, r) < 0)
                return -1;
            r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] - 1u));
        } while ((uint16_t)r->d[0] != 0xFFFFu);
    }
    Gfx_ClearTextLayerFrom(m, r->a[0]);
    *r = saved;
    move_flags(r, 0);
    return 0;
}

static int clear_layers(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                        task0_checkpoint_t checkpoint)
{
    uint32_t sp = r->a[7];
    push_long(m, r, ret);                       /* 0x005A12 */
    if (clear_text(m, r, 0x005A16u, checkpoint) < 0)
        return -1;
    /* The last subcall, 0x0059B4, saves a superset of the earlier frames. */
    save_call(m, r, 0x005A26u, 0x0307u);
    Gfx_ClearPlayfields(m);
    r->a[7] = sp;
    return 0;
}

static void load_font(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    save_call(m, r, ret, 0x0303u);               /* 0x005A3A saves D0-D1/A0-A1 */
    (void)Fn_005A3A(m, r);
    r->a[7] = sp;
}

static void load_sound_bank(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    wr32(m, r->a[7] - 4u, ret);                 /* 0x0014DA */
    wr32(m, r->a[7] - 8u, 0x0014E6u);           /* last BSR in 0x0014DA */
    SoundBank0_0014DA_Regs(m, r);
}

static void push_trap_argument(const f3mem_t *m, f3regs_t *r, uint32_t value)
{
    push_long(m, r, value);
    move_flags(r, (int32_t)value);               /* MOVE.L to stack */
}

static void push_spawn(const f3mem_t *m, f3regs_t *r, uint32_t slot,
                       uint32_t entry)
{
    push_trap_argument(m, r, slot);
    push_long(m, r, entry);                     /* PEA preserves CCR */
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

/* 0x010432: test-mode entry (reached from 0x010182 or from 0x01033C's -1). */
static uint32_t pass_10432(const f3mem_t *m, f3regs_t *r)
{
    push_trap_argument(m, r, 0xFFFEu);
    return 0x010438u;                           /* TRAP #2 */
}

static uint32_t pass_1044E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    r->a[7] += 12u;
    wr8(m, a5 - 0x5DC7u, 0);
    wr8(m, a5 - 0x5DCCu, 0);
    wr8(m, a5 - 0x1444u, 0);
    clear_sprites(m, r, 0x010462u);
    (void)clear_layers(m, r, 0x010466u, 0);
    load_sound_bank(m, r, 0x01046Eu);
    stop_sound(m, r, 0x010474u);
    return 0x010474u;                           /* TRAP #5 */
}

/* 0x010236..0x01026E: rebuild the machine before spawning the init tasks. */
static uint32_t pass_10236(const f3mem_t *m, f3regs_t *r, task0_checkpoint_t checkpoint)
{
    uint32_t a5 = r->a[5];

    load_font(m, r, 0x01023Cu);
    wr8(m, a5 - 0x144Cu, 0);
    wr8(m, a5 - 0x144Bu, 0);
    wr8(m, a5 - 0x1452u, 0);
    stop_sound(m, r, 0x01024Eu);
    wr32(m, r->a[7] - 4u, 0x010252u);
    (void)Fn_001070(m, r);
    clear_sprites(m, r, 0x010256u);
    if (clear_layers(m, r, 0x01025Au, checkpoint) < 0)
        return SEG_LEAVE;
    wr32(m, r->a[7] - 4u, 0x01025Eu);
    wr32(m, r->a[7] - 8u, 0x005B40u);           /* 0x005B0C's final fill call */
    Gfx_InitLineRam_Regs(m, r);
    r->sr &= (uint16_t)~0x10u;                  /* 0x005B72 addi.w #0x400,D0 */
    load_sound_bank(m, r, 0x010262u);
    push_spawn(m, r, 4u, 0x010488u);
    return 0x01026Eu;                           /* TRAP #1 */
}

/* 0x01008A-style startup, first half: font, settings check, "WAIT A MOMENT"
 * and the first sleep. */
static uint32_t pass_10174(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];

    load_font(m, r, 0x01017Au);
    push_long(m, r, 0x010180u);                 /* 0x01017A jsr 0x000050DC */
    int rc = Fn_0050DC(m, r);
    r->a[7] = sp;
    if (rc < 0)
        return SEG_LEAVE;                       /* settings recovery: untranslated */
    if (r->d[0] != 0u)                          /* 0x010180 tst.l D0 / bne */
        return pass_10432(m, r);

    draw_text(m, r, 0x01019Cu, 0x01047Au, 0x61C79Cu, 7u);
    r->sr &= (uint16_t)~0x10u;                  /* 0x00567A add.w D7,D7 */
    /* 0x0101A0 move.l #0xB4,-(A7) ; 0x0101A6 trap #4: sleep 180 frames. The
     * CPU frame and the handler (0x001F30) belong to the scheduler, as with
     * every other yield; 0x0101A8 addq.l #4,A7 pops the argument on wake. */
    push_trap_argument(m, r, 0xB4u);
    return 0x0101A6u;
}

/* 0x0101A8: the two sound cues, the service-switch latch and the second
 * sleep. */
static uint32_t pass_101A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    r->a[7] += 4u;                              /* 0x0101A8 addq.l #4,A7 */
    play_sound(m, r, 0x0000u, 0x0101BAu);                     /* 0x0101AA..0x0101BA */
    play_sound(m, r, 0x0094u, 0x0101CEu);                     /* 0x0101BE..0x0101CE */

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
    push_trap_argument(m, r, 0xB4u);
    return 0x0101F8u;
}

/* 0x0101FA: sound cue, cache enable, volume, text clear and rebuild. */
static uint32_t pass_101FA(const f3mem_t *m, f3regs_t *r, task0_checkpoint_t checkpoint)
{
    uint32_t a5 = r->a[5];
    r->a[7] += 4u;
    play_sound(m, r, 0x0095u, 0x01020Cu);
    r->d[0] = 1u;                               /* 0x010210 moveq; 0x010212 movec to CACR */
    uint16_t volume = rd16(m, a5 - 0x5DC0u);
    for (unsigned i = 0; i < 4u; ++i)
        push_word(m, r, volume);
    push_long(m, r, 0x01022Cu);
    wr32(m, r->a[7] - 4u, r->a[0]);             /* 0x003CBA saves A0 */
    (void)Fn_003CBA(m, r);
    r->a[7] += 12u;
    (void)clear_text(m, r, 0x010236u, 0);
    return pass_10236(m, r, checkpoint);
}

/* 0x010284: latch the start-ready flag and prepare to spawn the object runner. */
static uint32_t pass_10284(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    wr8(m, a5 - 0x1444u, rd8(m, a5 - 0x5DC5u));
    push_spawn(m, r, 1u, 0x0B8404u);
    return 0x010296u;                           /* TRAP #1 */
}

/* 0x0102EE: clear the screen, then stop at the task kill. */
static uint32_t pass_scene_reset(const f3mem_t *m, f3regs_t *r)
{
    stop_sound(m, r, 0x0102F4u);
    clear_sprites(m, r, 0x0102FAu);
    (void)clear_layers(m, r, 0x010300u, 0);
    push_trap_argument(m, r, 0x7FFEu);
    return 0x010306u;                           /* TRAP #2 */
}

/* 0x000FEC returns through three different operand sizes in D0. */
static void check_start_ready(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    wr32(m, r->a[7] - 4u, 0x0102EAu);
    if (!rd16(m, a5 - 0x7F5Eu)) {
        r->d[0] = (r->d[0] & ~0xFFu) | Coin_CheckStartReady(m);
    } else if (rd16(m, a5 - 0x7F70u)) {
        r->d[0] = 0;
    } else {
        uint16_t paid = rd16(m, a5 - 0x7F6Eu);
        if (paid == 0u) {
            r->d[0] = 0xFFFFFF00u | Coin_CheckStartReady(m);
        } else {
            uint16_t change = rd16(m, a5 - 0x7F5Cu);
            r->d[0] = set_word(r->d[0], (uint16_t)(change - paid));
            r->sr = (uint16_t)((r->sr & ~0x10u) | (change >= paid ? 0x10u : 0u));
        }
    }
    move_flags(r, (int8_t)r->d[0]);              /* 0x0102EA tst.b D0 */
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
        check_start_ready(m, r);                   /* 0x0102E4 jsr */
        if ((int8_t)r->d[0] < 0)                /* 0x0102EA tst.b / blt 0x01029A */
            return 0x01029Au;
        return pass_scene_reset(m, r);
    }

    wr16(m, a5 + 0x1D38u, 0u);                  /* 0x0102BC move.w #0 */
    if ((rd8(m, a5 - 0x5DDBu) & 0x80u) == 0u)   /* 0x0102C2 btst #7 / beq */
        return pass_scene_reset(m, r);
    if (rd8(m, a5 - 0x5DC6u) != 0u)             /* 0x0102CC tst.b / bne 0x01029A */
        return 0x01029Au;
    wr16(m, a5 + 0x1D38u, 1u);                  /* 0x0102D2 move.w #1 */
    if ((rd8(m, a5 - 0x5DD7u) & 0x80u) == 0u)   /* 0x0102D8 btst #7 / beq */
        return pass_scene_reset(m, r);
    return 0x01029Au;                           /* 0x0102E2 bra 0x01029A */
}

/* 0x010322: after the reset pass, wait for A5-0x144B bit 0, then kill the
 * tasks and run the 0x010236 init again. */
static uint32_t pass_10322(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];

    if (call_01033C(m, r, 0x010326u) < 0)
        return pass_10432(m, r);
    if ((rd8(m, a5 - 0x144Bu) & 0x01u) == 0u)   /* 0x010326 btst #0 / beq 0x010320 */
        return 0x010320u;
    push_trap_argument(m, r, 0xFFFEu);
    return 0x010334u;
}

int Fn_010172(const f3mem_t *m, f3regs_t *r)
{
    /* 0x010172 trap #5: the task yields before its first instruction, so the
     * body starts at 0x010174 (Task0_Main_Frame). */
    (void)m;
    (void)r;
    return 0;
}

/* ROM 0x010172: code between Task0_Main's traps. */
uint32_t Seg_Task0_Main(const f3mem_t *m, f3regs_t *r, uint32_t pc,
                        task0_checkpoint_t checkpoint)
{
    switch (pc) {
    case 0x010174u: return pass_10174(m, r);
    case 0x0101A8u: return pass_101A8(m, r);
    case 0x0101FAu: return pass_101FA(m, r, checkpoint);
    case 0x010236u: return pass_10236(m, r, checkpoint);
    case 0x010270u:
        r->a[7] += 8u;
        push_spawn(m, r, 0x0Fu, 0x0038ACu);
        return 0x01027Eu;
    case 0x010280u:
        r->a[7] += 8u;
        return 0x010282u;
    case 0x010284u: return pass_10284(m, r);
    case 0x010298u:
        r->a[7] += 8u;
        return 0x01029Au;
    case 0x01029Cu: return pass_1029C(m, r);
    case 0x010308u:
        r->a[7] += 4u;
        push_spawn(m, r, 1u, 0x004146u);
        return 0x010316u;
    case 0x010318u:
        r->a[7] += 8u;
        wr8(m, r->a[5] - 0x1444u, 0);
        move_flags(r, 0);
        return 0x010320u;
    case 0x010322u: return pass_10322(m, r);
    case 0x010336u:
        r->a[7] += 4u;
        return pass_10236(m, r, checkpoint);
    case 0x010432u: return pass_10432(m, r);
    case 0x01043Au:
        r->a[7] += 4u;
        push_trap_argument(m, r, rd32(m, 0));
        push_spawn(m, r, 1u, 0x1A1A26u);
        return 0x01044Cu;
    case 0x01044Eu: return pass_1044E(m, r);
    case 0x010476u: return 0x010474u;
    default: return SEG_LEAVE;
    }
}

uint32_t Task0_Main_Frame(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    for (;;) {
        uint32_t trap = Seg_Task0_Main(m, r, pc, 0);
        if (trap == SEG_LEAVE)
            return 0;
        pc = trap + 2u;
        unsigned vector = rd16(m, trap) & 0x0Fu;
        if (vector != 1u && vector != 2u)
            return pc;
        run_trap(m, r, trap, vector);
    }
}
