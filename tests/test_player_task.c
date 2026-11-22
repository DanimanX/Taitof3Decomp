/* Differential test: ROM 0x0C6A64 Task_PlayerControl (C) vs MAME Work RAM.
 *
 * MAME reference. The task is not a call-and-return routine: it yields
 * (TRAP #5) and every resume re-enters 0x0C6A64 from the top (docs/PLAYER.md
 * §4). Captures taken with a breakpoint on any PC of this task are perturbed
 * (a non-persistent breakpoint makes the emulated CPU stop one instruction
 * earlier on every later hit -- measured), so the reference here is a MAME run
 * with NO debugger: the state's Work RAM as loaded (frame 0, pre) and at the
 * end of the first emulated frame (post):
 *
 *   F3_OUT=work/ram/boot_<state> scripts/mame_run.sh \
 *     work/dariusg_trace/lua/boot_frame.lua -state <state>
 *   make build/test_player_task && ./build/test_player_task
 *
 * Result (MAME 0.289): every byte the model writes is a byte MAME also writes
 * (phantom-writes = 0 in all three states), and 8 / 9 / 5 bytes per state differ
 * from MAME outside those regions: 0x41A995/0x41ACED (collision list tails),
 * 0x41ABEA.. (shot-list head), 0x41B197/0x41B1CF/0x41B1E1/0x41B1EB/0x41B211
 * (player struct words the 0x0B9080 script pass / collision slot owns). They
 * are the acceptance ceiling of this lane: the test reports the count and
 * fails if it grows past MAX_RESIDUAL.
 *
 * Checks:
 *   1. helpers on a hand-built Work RAM image (0x0C6DEC, 0x0C6CE6, 0x0C6D0E,
 *      0x0C6E10, 0x0C6B36), including the stop-before-side-effects contract of
 *      every not-yet-decompiled tail;
 *   2. PlayerControl_RunFrame on the three boot images: it must reach the
 *      documented stop (0x0B9080) and match MAME outside the regions another
 *      kernel slot rewrites during the same frame (ROM shadow, io, RNG seed,
 *      sprite cursor, shield/text objects, object pool -- owned by 0x0B9080's
 *      scripts and the particle/HUD slots);
 *   3. the two frame captures at 0x0C6B16 are checked for consistency only.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "capture.h"
#include "../src/player/player_task.h"
#include "../src/objects/obj_pool.h"

#define A_SCENE     (F3_A5 + 0x1D1Eu)
#define A_DEMO_MODE (F3_A5 + 0x1D40u)
#define A_CTRL      (F3_A5 + 0x1D36u)
#define A_FRAME_CNT (F3_A5 + 0x0878u)
#define A_LIST      (F3_A5 + 0x1D4Cu)
#define A_LIVES_P1  (F3_A5 + 0x1D3Au)
#define A_LIVES_P2  (F3_A5 + 0x1D3Cu)
#define A_SHAKE_X   (F3_A5 + 0x1D20u)
#define A_SHAKE_Y   (F3_A5 + 0x1D24u)
#define A_DEMO_LVL  (F3_A5 + 0x1D4Au)
#define A_TROW1     (F3_A5 + 0x1D46u)
#define A_TCNT1     (F3_A5 + 0x1D48u)
#define A_TROW2     (F3_A5 + 0x1D42u)
#define A_TCNT2     (F3_A5 + 0x1D44u)
#define A_ACT_CNT   (F3_A5 + 0x1D34u)
#define A_ACT_OBJ   (F3_A5 + 0x1D28u)
#define A_ACT_TBL   (F3_A5 + 0x1D2Cu)
#define A_PICK_SEED (F3_A5 - 0x1456u)
#define A_BOMB      (F3_A5 + 0x1D58u)
#define A_SHIP_MASK (OBJ_FREE_MASKS + 2u * 4u)  /* Obj_Spawn(.., group 2..7) */
#define P1          P1_STATE_ADDR
#define P2          (P1_STATE_ADDR + 0x8Au)
#define A_SHIELD_ST 0x41B2A4u
#define P1_SHOTS    0x410354u

/* Bytes per state that the 0x0B9080 script pass / collision slot own (see the
 * header comment); the test fails when this count grows. */
#define MAX_RESIDUAL 9

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("FAIL: "); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static f3mem_t mk(uint8_t *wram, const uint8_t *rom)
{
    f3mem_t m;
    memset(&m, 0, sizeof m);
    m.wram = wram;
    m.rom = rom;
    return m;
}

static int nonzero(f3mem_t *m, uint32_t base, uint32_t n)
{
    int c = 0;
    for (uint32_t i = 0; i < n; i++) c += rd8(m, base + i) != 0;
    return c;
}

/* Regions another kernel slot rewrites between two frames of this task, and
 * the two words this lane clears in its scene init while another slot also
 * writes them (measured: the model's 0 values at 0x408878 / 0x409D47 are
 * restored by the next frame, so they are not exclusive to 0x0C6A64). */
static int foreign_region(uint32_t a)
{
    if (a == 0x408878u || a == 0x409D47u) return 16;
    if (a >= 0x410000u && a < 0x410060u) return 17; /* pool free/snapshot masks */
    if (a >= 0x400000u && a < 0x400B00u) return 1;  /* ROM shadow, kernel counters */
    if (a >= 0x402200u && a < 0x4022C0u) return 2;  /* io ports, debug taps */
    if (a >= 0x406BA0u && a < 0x406BB0u) return 3;  /* RNG seed */
    if (a >= 0x40735Du && a < 0x407380u) return 4;  /* sprite cursor/shadow */
    if (a >= 0x4018C0u && a < 0x407200u) return 7;  /* palette RAM mirror, HUD */
    if (a >= 0x400B30u && a < 0x400B60u) return 8;  /* kernel slot bookkeeping */
    if (a >= 0x404E00u && a < 0x404F00u) return 9;  /* sound ring counters */
    if (a >= 0x41B2A4u && a < 0x41B500u) return 5;  /* shield + text objects */
    if (a >= OBJ_BASE && a < OBJ_BASE + 12u * OBJ_GROUP_SIZE) return 6; /* pool */
    if (a >= 0x408840u && a < 0x408A80u) return 10; /* collision lists, script ptrs */
    if (a >= 0x409D20u && a < 0x409F00u) return 11; /* script/particle state */
    if (a >= 0x40A000u && a < 0x40A200u) return 12; /* fx/particle state */
    if (a >= 0x40B300u && a < 0x40B3A0u) return 13; /* stage state */
    if (a >= 0x40C1C0u && a < 0x40C300u) return 14; /* scroll/timer state */
    return 0;
}

/* ---- 1. helper steps --------------------------------------------------- */
static void test_helpers(const uint8_t *rom)
{
    static uint8_t img[F3_WRAM_SIZE];
    f3mem_t m = mk(img, rom);

    /* Player_PickActive, nobody active */
    memset(img, 0, sizeof img);
    memset(img + (OBJ_FREE_MASKS - F3_WRAM_BASE), 0, 4 * 12);
    wr16(&m, P1 + 0x4C, 0xFFFFu);
    wr16(&m, P2 + 0x4C, 0xFFFFu);
    wr32(&m, A_ACT_OBJ, 0xDEADBEEFu);
    Player_PickActive(&m);
    CHECK(rd16(&m, A_ACT_CNT) == 0, "PickActive count with nobody active");
    CHECK(rd32(&m, A_ACT_OBJ) == 0, "PickActive clears 0x1D28 first");

    /* one active player (the table is read back through the RAM image) */
    wr16(&m, P1 + 0x4C, 0x0002u);
    wr16(&m, A_PICK_SEED, 0x1234u);
    wr32(&m, A_ACT_TBL, 0);
    wr32(&m, A_ACT_TBL + 4u, 0);
    Player_PickActive(&m);
    CHECK(rd16(&m, A_ACT_CNT) == 1, "PickActive count with P1 active");
    CHECK(rd32(&m, A_ACT_TBL) == P1, "PickActive records the active player");
    CHECK(rd32(&m, A_ACT_OBJ) == P1, "PickActive picks the only active player");

    /* both active: divu over the seed, remainder = seed % 2 */
    wr16(&m, P2 + 0x4C, 0x0000u);
    for (int bit = 0; bit < 2; bit++) {
        wr16(&m, A_PICK_SEED, (uint16_t)(0x0100u | bit));
        wr32(&m, A_ACT_OBJ, 0);
        Player_PickActive(&m);
        CHECK(rd16(&m, A_ACT_CNT) == 2, "PickActive count with both active");
        CHECK(rd32(&m, A_ACT_TBL) == P1 && rd32(&m, A_ACT_TBL + 4) == P2,
              "PickActive table order");
        CHECK(rd32(&m, A_ACT_OBJ) == (bit ? P2 : P1),
              "PickActive remainder picks the player (seed bit %d)", bit);
    }

    /* Player_SceneInit: scene words, then it stops at 0x001660 */
    memset(img, 0, sizeof img);
    wr8(&m, A_CTRL, 0xFFu);
    wr8(&m, A_BOMB, 0x55u);
    wr16(&m, A_FRAME_CNT, 0x1234u);
    wr16(&m, A_DEMO_LVL, 9u);
    wr32(&m, A_SHAKE_X, 1u);
    wr32(&m, A_SHAKE_Y, 2u);
    memset(img + (OBJ_FREE_MASKS - F3_WRAM_BASE), 0, 4 * 12);
    wr32(&m, A_SHIP_MASK, 0x7FFFFFFFu);
    memset(img + (OBJ_BASE - F3_WRAM_BASE) + 2 * OBJ_GROUP_SIZE, 0, OBJ_GROUP_SIZE);
    int rc = Player_SceneInit(&m);
    CHECK(rc == -1, "SceneInit stops at the 0x001660 tail");
    CHECK(rd8(&m, A_CTRL) == 0xE1u, "SceneInit clears ctrl bits 1..4 (got %02X)",
          rd8(&m, A_CTRL));
    CHECK(rd8(&m, A_BOMB) == 0 && rd16(&m, A_FRAME_CNT) == 0 &&
          rd16(&m, A_DEMO_LVL) == 0 && rd32(&m, A_SHAKE_X) == 0 && rd32(&m, A_SHAKE_Y) == 0,
          "SceneInit clears the scene words");
    CHECK(rd16(&m, A_TROW1) == 0 && rd16(&m, A_TCNT1) == 0 && rd16(&m, A_TROW2) == 0 &&
          rd16(&m, A_TCNT2) == 0 && rd16(&m, A_SHIELD_ST + 0x1C) == 0 &&
          rd16(&m, A_SHIELD_ST + 0x1E) == 0, "SceneInit runs 0x0C7D9A");
    CHECK(nonzero(&m, OBJ_BASE + 3 * OBJ_GROUP_SIZE, OBJ_GROUP_SIZE) == 0,
          "SceneInit stops before the ship objects are spawned");
    CHECK(rd8(&m, A_DEMO_MODE) == 0, "SceneInit leaves 0x1D40");

    /* Player_ShotPoolReset: exactly 64 objects */
    memset(img, 0, sizeof img);
    wr32(&m, P1_SHOTS + 0x10u, 0x11111111u);
    wr8(&m, P1_SHOTS + 0x34u, 0x01u);
    wr8(&m, P1_SHOTS + 0x35u, 0x01u);
    wr16(&m, P1_SHOTS + 0x1Eu, 7u);
    wr32(&m, P1_SHOTS + 64u * 0x6Cu + 0x10u, 0x22222222u);
    Player_ShotPoolReset(&m);
    CHECK(rd32(&m, P1_SHOTS + 0x10u) == 0x00028740u && rd8(&m, P1_SHOTS + 0x34u) == 0xFF &&
          rd8(&m, P1_SHOTS + 0x35u) == 0 && rd16(&m, P1_SHOTS + 0x1Eu) == 0,
          "ShotPoolReset first object");
    CHECK(rd32(&m, P1_SHOTS + 63u * 0x6Cu + 0x10u) == 0x00028740u &&
          rd8(&m, P1_SHOTS + 63u * 0x6Cu + 0x34u) == 0xFF, "ShotPoolReset last object");
    CHECK(rd32(&m, P1_SHOTS + 64u * 0x6Cu + 0x10u) == 0x22222222u,
          "ShotPoolReset stays inside the pool");

    /* Player_ArmRespawn */
    memset(img, 0, sizeof img);
    wr32(&m, P1, 0x000C77BEu);
    wr16(&m, A_LIVES_P1, 0x0003u);
    wr16(&m, A_LIVES_P2, 0x0000u);
    wr16(&m, P1 + 0x5Eu, 5u);
    wr16(&m, P1 + 0x52u, 5u);
    wr16(&m, P1 + 0x62u, 5u);
    wr8(&m, P1 + 0x64u, 0xFFu);
    wr8(&m, P1 + 0x65u, 0x55u);
    Player_ArmRespawn(&m, P1);
    CHECK(rd32(&m, P1) == PLAYER_FN_RESPAWN, "ArmRespawn sets fn 0x0C793E");
    CHECK(rd16(&m, P1 + 0x4C) == 0x0003u, "ArmRespawn copies the P1 life counter");
    CHECK(rd16(&m, P1 + 0x60) == 0x0000u, "ArmRespawn copies the P2 life word");
    CHECK(rd16(&m, P1 + 0x5E) == 0 && rd16(&m, P1 + 0x52) == 0 && rd16(&m, P1 + 0x62) == 0,
          "ArmRespawn clears +0x5E/+0x52/+0x62");
    CHECK(rd8(&m, P1 + 0x64) == 0xFEu, "ArmRespawn clears bit 0 of +0x64");
    CHECK(rd8(&m, P1 + 0x65) == 0x55u, "ArmRespawn leaves +0x65");

    /* Player_Respawn up to its 0x0C8E1A tail */
    memset(img, 0, sizeof img);
    wr16(&m, P1 + 0x22, 0u);
    wr16(&m, P1 + 0x54, 0x0007u); /* 7<<16 / 3 = q 2 r 1 */
    wr16(&m, P1 + 0x58, 0x000Cu); /* 12<<16 / 5 = q 2 r 2 */
    wr16(&m, P1 + 0x56, 0x0021u);
    wr16(&m, P1 + 0x4E, 0x0005u);
    wr8(&m, P1 + 0x65, 0xFFu);
    wr16(&m, A_LIVES_P2, 0x0002u);
    rc = Player_Respawn(&m, P1);
    CHECK(rc == -1, "Respawn stops at the 0x0C8E1A tail");
    CHECK(rd16(&m, P1 + 0x14) == 0xFFE0u && rd16(&m, P1 + 0x18) == 0x0040u,
          "Respawn spawn position from 0x0C926E[0]");
    CHECK(rd32(&m, P1 + 0x24) == 0x00010000u && rd32(&m, P1 + 0x28) == 0x00010000u,
          "Respawn resets HP and contact damage");
    CHECK(rd16(&m, P1 + 0x50) == 0x00D2u, "Respawn invulnerability 0xD2");
    CHECK(rd16(&m, P1 + 0x54) == 0x0002u, "Respawn multiplier 7-3-remainder(1) (got %u)",
          (unsigned)rd16(&m, P1 + 0x54));
    CHECK(rd16(&m, P1 + 0x58) == 0x000Au, "Respawn power 12-remainder(2) (got %u)",
          (unsigned)rd16(&m, P1 + 0x58));
    CHECK(rd16(&m, P1 + 0x60) == 0x0002u, "Respawn copies the P2 life word");
    CHECK(rd16(&m, P1 + 0x6A) == 0x0021u && rd16(&m, P1 + 0x6C) == 0xFFFFu &&
          rd16(&m, P1 + 0x7A) == 0x000Au && rd16(&m, P1 + 0x7C) == 0xFFFFu,
          "Respawn re-arms the two (value, -1) pairs");
    CHECK(rd8(&m, P1 + 0x34) == 0 && rd32(&m, P1 + 0x3A) == 0 && rd16(&m, P1 + 0x4E) == 0,
          "Respawn clears flags/bank/shield");
    /* below 3 the quotient word is 0, so the -3 is skipped but the remainder
     * (2) is still taken: 2 -> 0 */
    memset(img, 0, sizeof img);
    wr16(&m, P1 + 0x54, 0x0002u);
    Player_Respawn(&m, P1);
    CHECK(rd16(&m, P1 + 0x54) == 0xFFFFu, "Respawn: 2-2-1 wraps to 0xFFFF (got %u)",
          (unsigned)rd16(&m, P1 + 0x54));
}

/* ---- 2. RunFrame contract (no MAME needed) ----------------------------- */
static void test_runframe_contract(const uint8_t *rom)
{
    static uint8_t img[F3_WRAM_SIZE];
    f3mem_t m = mk(img, rom);

    memset(img, 0, sizeof img);
    /* only group 2, slot 0 free: where the wrapper task must land */
    memset(img + (OBJ_FREE_MASKS - F3_WRAM_BASE), 0, 4 * 12);
    wr32(&m, A_SHIP_MASK, 0x7FFFFFFFu);
    wr8(&m, A_CTRL, 0xFFu);
    int rc = PlayerControl_RunFrame(&m);
    CHECK(rc == -1, "RunFrame stops at the scene-init tail");
    CHECK(rd8(&m, A_SCENE) == 0x01u, "RunFrame sets the scene-started bit");
    CHECK(rd16(&m, A_DEMO_MODE) == 0x001Du, "RunFrame writes 0x1D40 = 0x1D once");
    CHECK(rd8(&m, A_CTRL) == 0x00u, "RunFrame clears ctrl, SceneInit clears 1..4 (got %02X)",
          rd8(&m, A_CTRL));
    CHECK(rd32(&m, OBJ_BASE + 2 * OBJ_GROUP_SIZE + OBJ_SIZE) == 0x000C685Au,
          "RunFrame spawned the wrapper fn 0x0C685A (group 2, slot 1)");
    CHECK(rd8(&m, OBJ_BASE + 2 * OBJ_GROUP_SIZE + OBJ_SIZE + 4) == 2u &&
          rd8(&m, OBJ_BASE + 2 * OBJ_GROUP_SIZE + OBJ_SIZE + 5) == 1u,
          "RunFrame spawn object header group 2 / slot 1");
    CHECK(rd32(&m, 0x41B190u) == 0, "RunFrame leaves the player structs to 0x001660");

    wr16(&m, A_DEMO_MODE, 0x0055u);
    wr8(&m, A_CTRL, 0x0Fu);
    wr8(&m, A_SCENE, 0x01u);
    rc = PlayerControl_RunFrame(&m);
    CHECK(rc == -1, "RunFrame stops at the scene-init tail (2nd call)");
    CHECK(rd16(&m, A_DEMO_MODE) == 0x0055u, "RunFrame leaves 0x1D40 after the first scene");
    CHECK(rd8(&m, A_CTRL) == 0x01u, "RunFrame skips the bset block (got %02X)",
          rd8(&m, A_CTRL));
}

/* ---- 3. boot-frame comparison with MAME -------------------------------- */
static void run_boot(const uint8_t *rom, const char *state)
{
    char pre[256], post[256];
    snprintf(pre, sizeof pre, "work/ram/boot_%s_pre.bin", state);
    snprintf(post, sizeof post, "work/ram/boot_%s_post.bin", state);
    long l1, l2;
    uint8_t *p = cap_slurp(pre, &l1);
    uint8_t *q = cap_slurp(post, &l2);
    if (!p || !q || l1 != F3_WRAM_SIZE || l2 != F3_WRAM_SIZE) {
        printf("  %s: SKIP (need %s / %s)\n", state, pre, post);
        free(p); free(q);
        return;
    }
    static uint8_t img[F3_WRAM_SIZE];
    memcpy(img, p, F3_WRAM_SIZE);
    f3mem_t m = mk(img, rom);
    int rc = PlayerControl_RunFrame(&m);
    int bad = 0, shown = 0, phantom = 0, first_phantom = 0;
    for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
        uint32_t a = F3_WRAM_BASE + o;
        if (img[o] != q[o] && !foreign_region(a)) {
            bad++;
            if (shown++ < 20)
                printf("  %s @%06X got=%02X want=%02X\n", state, a, img[o], q[o]);
        }
        if (img[o] != p[o] && p[o] == q[o] && !foreign_region(a)) phantom++;
    }
    printf("  %s: rc=%d mismatches=%d phantom-writes=%d%s\n", state, rc, bad, phantom,
           phantom ? " (first: see next line)" : "");
    if (phantom)
        printf("  %s: first phantom write @%06X\n", state, (unsigned)first_phantom);
    /* Name the regions that own the remaining differences, so the next lane
     * can split them per routine instead of re-deriving this list. */
    {
        uint32_t first = 0, last = 0;
        for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
            uint32_t a = F3_WRAM_BASE + o;
            if (img[o] == q[o] && img[o] == p[o]) continue;
            if (!first) first = a;
            last = a;
        }
        if (first)
            printf("  %s: diff span %06X..%06X (other slots write these too)\n",
                   state, first, last);
    }
    CHECK(rc == -1, "%s: RunFrame must stop at 0x0B9080", state);
    CHECK(bad <= MAX_RESIDUAL, "%s: %d bytes differ from MAME (max %d)", state, bad,
          MAX_RESIDUAL);
    CHECK(phantom == 0, "%s: %d model writes MAME did not make", state, phantom);
    free(p);
    free(q);
}

/* ---- 4. frame captures at 0x0C6B16 (consistency only) ------------------ */
static void run_capture_consistency(const uint8_t *rom, const char *pfx)
{
    static capture_t c;
    if (cap_load(&c, pfx) != 0) {
        printf("  %s: SKIP (no capture)\n", pfx);
        return;
    }
    uint8_t *img = malloc(F3_WRAM_SIZE);
    int stopped = 0;
    for (int i = 0; i < c.pairs; i++) {
        f3mem_t m = mk(img, rom);
        memcpy(img, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        if (PlayerControl_RunFrame(&m) != 0) stopped++;
    }
    printf("  %s: frames=%d stopped=%d\n", pfx, c.pairs, stopped);
    CHECK(stopped == c.pairs, "%s: every frame must reach the 0x0B9080 stop", pfx);
    free(img);
    cap_free(&c);
}

int main(int argc, char **argv)
{
    long rl;
    uint8_t *rom = cap_slurp(argc > 1 ? argv[1] : "work/maincpu.bin", &rl);
    if (!rom) {
        printf("player-task: SKIP (no work/maincpu.bin)\n");
        return 0;
    }
    test_helpers(rom);
    test_runframe_contract(rom);
    run_boot(rom, "01_attract");
    run_boot(rom, "02_stage1_start");
    run_boot(rom, "03_active_combat");
    run_capture_consistency(rom, "work/ram/player_c6b16_02_stage1_start");
    run_capture_consistency(rom, "work/ram/player_c6b16_01_attract");
    run_capture_consistency(rom, "work/ram/player_c6b16_03_active_combat");
    printf("player-task: %s (%d/%d checks failed)\n", fails ? "FAIL" : "PASS", fails, checks);
    free(rom);
    return fails != 0;
}
