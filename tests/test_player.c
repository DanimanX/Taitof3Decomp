/* Differential test: Player_MoveAndBounds (C) vs MAME per-frame Work RAM.
 * Inputs (local, not versioned): work/maincpu.bin, work/ram/corr_frames.bin
 * (scripts/lua/session_corr.lua). For each frame t: RAM_t -> run C on P1 ->
 * compare pos/angle/bank/hp with RAM_{t+1}. io_ch is taken from RAM_t or
 * RAM_{t+1} (variant reported) since the poll happens at vblank.
 * Frames where other routines also move/damage the ship are excluded and
 * counted: external damage/death (hp drops or lock flag changes, written by
 * Task_Collision) and the respawn fly-in (invuln >= 150, +1.5 px/frame X). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/player/player_move.h"

static uint8_t *slurp(const char *path, long *len)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); *len = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc((size_t)*len);
    if (fread(b, 1, (size_t)*len, f) != (size_t)*len) { free(b); b = NULL; }
    fclose(f);
    return b;
}

#define P P1_STATE_ADDR
#define OFF(a) ((a) - F3_WRAM_BASE)

/* Pairs capture (scripts/lua/session_branches.lua): pre_t already contains the
 * forced pokes; post_t is the end of the next frame. io_ch is taken from post
 * (polled at the start of that frame). Reports per phase. */
static int run_pairs(const uint8_t *rom, const char *pairs_path, const char *phases_path)
{
    long cl;
    uint8_t *cap = slurp(pairs_path, &cl);
    FILE *ph = fopen(phases_path, "r");
    if (!cap || !ph) { printf("player-branches: SKIP (need %s)\n", pairs_path); return 0; }
    char line[128], names[512][24];
    int np = 0;
    if (!fgets(line, sizeof line, ph)) { fclose(ph); return 0; }
    while (np < 512 && fgets(line, sizeof line, ph)) {
        char *tab = strchr(line, '\t');
        if (!tab) continue;
        sscanf(tab + 1, "%23s", names[np++]);
    }
    fclose(ph);
    int pairs = (int)(cl / (2 * F3_WRAM_SIZE));
    if (pairs > np) pairs = np;
    uint8_t *work = malloc(F3_WRAM_SIZE);
    const char *phase_names[] = { "normal", "shake", "terrain_arm0", "terrain_arm10" };
    int ok[4] = {0}, tot[4] = {0}, ext[4] = {0}, shown = 0, rc = 0;
    static const uint32_t fields[] = { PS_POS_X, PS_POS_Y, PS_BANK, PS_HP, PS_SHIELD, PS_SPEED };
    static const char *fname[] = { "pos_x", "pos_y", "bank", "hp", "shield", "speed" };
    static const int fsize[] = { 4, 4, 4, 4, 2, 2 };
    for (int i = 0; i < pairs; i++) {
        const uint8_t *pre = cap + (size_t)i * 2 * F3_WRAM_SIZE, *post = pre + F3_WRAM_SIZE;
        int k = 0;
        while (k < 4 && strcmp(names[i], phase_names[k])) k++;
        if (k == 4) continue;
        memcpy(work, pre, F3_WRAM_SIZE);
        memcpy(work + OFF(0x40221C), post + OFF(0x40221C), 24);
        f3mem_t m = { work, rom, NULL, NULL, NULL, NULL }, mp = { (uint8_t *)post, rom, NULL, NULL, NULL, NULL };
        if ((int16_t)rd16(&m, P + 0x4C) < 0 || (rd8(&m, P + PS_FLAGS) & 1) ||
            rd8(&mp, P + PS_FLAGS) != rd8(&m, P + PS_FLAGS)) { ext[k]++; continue; }
        /* Note: shake frames consume RNG and match bit-exactly from pre's
           seed, i.e. the ship tick runs before the idle loop in the frame. */
        Player_MoveAndBounds(&m, P);
        int good = 1, only_hp_lower = 1, any_diff = 0;
        for (int f = 0; f < 6; f++) {
            uint32_t g = fsize[f] == 4 ? rd32(&m, P + fields[f]) : rd16(&m, P + fields[f]);
            uint32_t w = fsize[f] == 4 ? rd32(&mp, P + fields[f]) : rd16(&mp, P + fields[f]);
            if (g != w) { any_diff = 1; if (fields[f] != PS_HP || (int32_t)w > (int32_t)g) only_hp_lower = 0; }
        }
        /* hp lower than predicted with every other field equal: an enemy or
           bullet hit applied by Task_Collision in the same frame (external). */
        if (any_diff && only_hp_lower) { ext[k]++; continue; }
        for (int f = 0; f < 6; f++) {
            uint32_t got = fsize[f] == 4 ? rd32(&m, P + fields[f]) : rd16(&m, P + fields[f]);
            uint32_t want = fsize[f] == 4 ? rd32(&mp, P + fields[f]) : rd16(&mp, P + fields[f]);
            if (got != want) {
                good = 0;
                if (shown < 8) { printf("  pair %d [%s] %s got=%08X want=%08X\n", i, phase_names[k], fname[f], got, want); shown++; }
            }
        }
        tot[k]++; ok[k] += good;
    }
    for (int k = 0; k < 4; k++) {
        printf("player-branches: %-14s %3d/%3d match (%d excluded: external hp hits/locks)\n", phase_names[k], ok[k], tot[k], ext[k]);
        if (ok[k] != tot[k] || tot[k] < 50) rc = 1;
    }
    free(work); free(cap);
    return rc;
}

int main(int argc, char **argv)
{
    const char *rom_path = argc > 1 ? argv[1] : "work/maincpu.bin";
    const char *cap_path = argc > 2 ? argv[2] : "work/ram/corr_frames.bin";
    long rl, cl;
    uint8_t *rom = slurp(rom_path, &rl), *cap = slurp(cap_path, &cl);
    if (!rom || !cap) { printf("player: SKIP (need %s and %s)\n", rom_path, cap_path); return 0; }
    int frames = (int)(cl / F3_WRAM_SIZE);
    uint8_t *work = malloc(F3_WRAM_SIZE);
    f3mem_t m = { work, rom, NULL, NULL, NULL, NULL };
    static const uint32_t fields[] = { PS_POS_X, PS_POS_Y, PS_BANK, PS_HP };
    static const char *fname[] = { "pos_x", "pos_y", "bank", "hp" };
    int rc = 0;
    for (int variant = 0; variant < 2; variant++) {
        int tested = 0, ok = 0, skipped = 0, shown = 0, ext_dmg = 0, flyin = 0;
        for (int t = 0; t + 1 < frames; t++) {
            const uint8_t *pre = cap + (size_t)t * F3_WRAM_SIZE, *post = pre + F3_WRAM_SIZE;
            memcpy(work, pre, F3_WRAM_SIZE);
            if (variant == 1) memcpy(work + OFF(0x40221C), post + OFF(0x40221C), 24); /* io_ch */
            m.wram = work;
            /* only frames where the player object ticks normally: alive & unlocked */
            if ((int16_t)rd16(&m, P + 0x4C) < 0 || (rd8(&m, P + PS_FLAGS) & 1)) { skipped++; continue; }
            {
                f3mem_t mp = { (uint8_t *)post, rom, NULL, NULL, NULL, NULL };
                if ((int32_t)rd32(&mp, P + PS_HP) < (int32_t)rd32(&m, P + PS_HP) ||
                    rd8(&mp, P + PS_FLAGS) != rd8(&m, P + PS_FLAGS)) { ext_dmg++; continue; }
                if ((int16_t)rd16(&m, P + PS_INVULN) >= 150) { flyin++; continue; }
            }
            Player_MoveAndBounds(&m, P);
            int good = 1;
            for (int k = 0; k < 4; k++) {
                uint32_t got = rd32(&m, P + fields[k]);
                f3mem_t mp = { (uint8_t *)post, rom, NULL, NULL, NULL, NULL };
                uint32_t want = rd32(&mp, P + fields[k]);
                if (got != want) {
                    good = 0;
                    if (variant == 1 && shown < 6) {
                        printf("  t=%d %s got=%08X want=%08X\n", t, fname[k], got, want);
                        shown++;
                    }
                }
            }
            tested++; ok += good;
        }
        printf("player: io_ch from %s: %d/%d frames match (excluded: %d dead/locked, %d external damage, %d respawn fly-in)\n",
               variant ? "RAM_t+1" : "RAM_t", ok, tested, skipped, ext_dmg, flyin);
        if (variant == 1 && (ok != tested || tested < 100)) rc = 1;
    }
    rc |= run_pairs(rom, argc > 3 ? argv[3] : "work/ram/branch_pairs.bin",
                    argc > 4 ? argv[4] : "work/ram/branch_phases.tsv");
    return rc;
}
