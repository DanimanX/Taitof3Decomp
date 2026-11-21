/* Differential test: Sound_PlayCmd (C) vs exact MAME pre/post pairs captured
 * with the debugger at entry 0x39D8 / rts 0x3A20, including the sound DPRAM
 * (F3_DPRAM=1 scripts/capture_fn.sh 39D8 3A20 100 work/ram/sndcmd).
 * The command word is the caller's pushed argument at SP+4. Compares all of
 * Work RAM (except the live stack frame) and all of the DPRAM. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/audio/sound_cmd.h"

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

enum { R_PLAY, R_FORCED, R_STOP, R_STOP2 };
static const char *rname[] = { "Sound_PlayCmd", "Sound_PlayCmdForced", "Sound_StopAll", "Sound_StopAllPart2" };

/* returns -1 if capture missing, else number of mismatching pairs; *n = pairs */
static int run(const char *pfx, int routine, const uint8_t *rom, int *n)
{
    char p1[256], p2[256], p3[256];
    snprintf(p1, sizeof p1, "%s_pairs.bin", pfx);
    snprintf(p2, sizeof p2, "%s_dpram.bin", pfx);
    snprintf(p3, sizeof p3, "%s_regs.tsv", pfx);
    long cl, dl, rl;
    uint8_t *cap = slurp(p1, &cl), *dp = slurp(p2, &dl);
    FILE *rf = fopen(p3, "r");
    (void)rl;
    if (!cap || !dp || !rf) { if (rf) fclose(rf); free(cap); free(dp); return -1; }
    int pairs = (int)(cl / (2 * F3_WRAM_SIZE));
    if ((long)pairs > dl / (2 * F3_DPRAM_SIZE)) pairs = (int)(dl / (2 * F3_DPRAM_SIZE));
    uint32_t *sp = calloc((size_t)pairs, sizeof *sp);
    char line[512];
    if (fgets(line, sizeof line, rf)) {
        while (fgets(line, sizeof line, rf)) {
            int i; char which[8];
            if (sscanf(line, "%d\t%7s", &i, which) != 2 || strcmp(which, "pre") || i >= pairs) continue;
            char *c = line;
            for (int t = 0; t < 18 && c; t++) c = strchr(c + 1, '\t'); /* 19th col = SP */
            if (c) sp[i] = (uint32_t)strtoul(c + 1, NULL, 16);
        }
    }
    fclose(rf);
    uint8_t *work = malloc(F3_WRAM_SIZE), dwork[F3_DPRAM_SIZE];
    int ok = 0, shown = 0, types[8] = {0}, skipped = 0;
    for (int i = 0; i < pairs; i++) {
        const uint8_t *pre = cap + (size_t)i * 2 * F3_WRAM_SIZE, *post = pre + F3_WRAM_SIZE;
        const uint8_t *dpre = dp + (size_t)i * 2 * F3_DPRAM_SIZE, *dpost = dpre + F3_DPRAM_SIZE;
        memcpy(work, pre, F3_WRAM_SIZE);
        memcpy(dwork, dpre, F3_DPRAM_SIZE);
        f3mem_t m = { work, (uint8_t *)rom, dwork, NULL, NULL };
        int16_t cmd = (int16_t)rd16(&m, sp[i] + 4);
        if (routine == R_PLAY && cmd >= 0 && cmd <= (int16_t)rd16(&m, 0x6750) && !rd8(&m, F3_A5 - 0x1444)) {
            uint8_t t = rd8(&m, rd32(&m, rd32(&m, F3_A5 - 0x1448) + 4u * (uint32_t)cmd));
            types[t < 7 ? t : 7]++;
        } else if (routine == R_PLAY) skipped++;
        switch (routine) {
        case R_PLAY:   Sound_PlayCmd(&m, cmd); break;
        case R_FORCED: Sound_PlayCmdForced(&m, cmd); break;
        case R_STOP:   Sound_StopAll(&m); break;
        default:       Sound_StopAllPart2(&m); break;
        }
        int diffs = 0;
        for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
            uint32_t a = F3_WRAM_BASE + o;
            if (a >= sp[i] - 0x40 && a < sp[i]) continue; /* movem save area */
            if (work[o] != post[o]) {
                if (shown < 10) { printf("  pair %d cmd %d wram @%06X got=%02X want=%02X\n", i, cmd, a, work[o], post[o]); shown++; }
                diffs++;
            }
        }
        for (uint32_t o = 0; o < F3_DPRAM_SIZE; o++)
            if (o != 0x482 && o != 0x483 && dwork[o] != dpost[o]) { /* 0x482: sound-CPU read index, moves concurrently */
                if (shown < 10) { printf("  pair %d cmd %d dpram +%03X got=%02X want=%02X (pre %02X)\n", i, cmd, o, dwork[o], dpost[o], dpre[o]); shown++; }
                diffs++;
            }
        ok += diffs == 0;
    }
    printf("sound: %-20s %-22s %3d/%3d calls bit-exact (Work RAM + DPRAM)", rname[routine], pfx, ok, pairs);
    if (routine == R_PLAY)
        printf("; handler types 0/2/3: %d/%d/%d, rejected %d", types[0], types[2], types[3], skipped);
    printf("\n");
    free(work); free(cap); free(dp); free(sp);
    *n = pairs;
    return pairs - ok;
}

int main(void)
{
    long rl;
    uint8_t *rom = slurp("work/maincpu.bin", &rl);
    if (!rom) { printf("sound: SKIP (need work/maincpu.bin)\n"); return 0; }
    static const struct { const char *pfx; int routine; } caps[] = {
        { "work/ram/sndcmd", R_PLAY },       /* natural gameplay calls */
        { "work/ram/sndsweep", R_PLAY },     /* F3_SWEEP=154: ids 0..123 */
        { "work/ram/sndsweep2", R_PLAY },    /* F3_SWEEP_BASE=124: ids 124..153 */
        { "work/ram/sndforced", R_FORCED },
        { "work/ram/sndstop", R_STOP },
        { "work/ram/sndstop2", R_STOP2 },
    };
    int bad = 0, total = 0;
    for (size_t k = 0; k < sizeof caps / sizeof caps[0]; k++) {
        int n = 0, r = run(caps[k].pfx, caps[k].routine, rom, &n);
        if (r < 0) { printf("sound: %-20s SKIP (no %s capture)\n", rname[caps[k].routine], caps[k].pfx); continue; }
        bad += r; total += n;
    }
    free(rom);
    return bad == 0 && total > 0 ? 0 : 1;
}
