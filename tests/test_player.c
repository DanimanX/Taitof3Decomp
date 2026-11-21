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

int main(int argc, char **argv)
{
    const char *rom_path = argc > 1 ? argv[1] : "work/maincpu.bin";
    const char *cap_path = argc > 2 ? argv[2] : "work/ram/corr_frames.bin";
    long rl, cl;
    uint8_t *rom = slurp(rom_path, &rl), *cap = slurp(cap_path, &cl);
    if (!rom || !cap) { printf("player: SKIP (need %s and %s)\n", rom_path, cap_path); return 0; }
    int frames = (int)(cl / F3_WRAM_SIZE);
    uint8_t *work = malloc(F3_WRAM_SIZE);
    f3mem_t m = { work, rom };
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
                f3mem_t mp = { (uint8_t *)post, rom };
                if ((int32_t)rd32(&mp, P + PS_HP) < (int32_t)rd32(&m, P + PS_HP) ||
                    rd8(&mp, P + PS_FLAGS) != rd8(&m, P + PS_FLAGS)) { ext_dmg++; continue; }
                if ((int16_t)rd16(&m, P + PS_INVULN) >= 150) { flyin++; continue; }
            }
            Player_MoveAndBounds(&m, P);
            int good = 1;
            for (int k = 0; k < 4; k++) {
                uint32_t got = rd32(&m, P + fields[k]);
                f3mem_t mp = { (uint8_t *)post, rom };
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
    return rc;
}
