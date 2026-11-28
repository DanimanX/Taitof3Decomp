/* Shared loader for scripts/capture_fn.sh outputs (<prefix>_pairs.bin,
 * _regs.tsv, optional _dpram.bin / _spr.bin / _gfx.bin / _pal.bin / _eep.bin). Header-only, for tests. */
#ifndef TAITOF3_TESTS_CAPTURE_H
#define TAITOF3_TESTS_CAPTURE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../src/common/f3mem.h"

enum { CR_D0 = 0, CR_A0 = 8, CR_A1 = 9, CR_SP = 15, CR_N = 16, CR_PC = 16, CR_SR = 17 };
enum { CAP_EEP_BYTES = F3_EEP_WORDS * 2, CAP_EEP_RECORD = CAP_EEP_BYTES + 24 };

typedef struct {
    int pairs;
    uint8_t *wram, *dpram, *spr, *gfx, *pal, *eep; /* pre,post interleaved per pair */
    uint32_t (*pre)[CR_N], (*post)[CR_N];
    unsigned *fpre, *fpost;              /* frame of each dump (regs.tsv col 3) */
    uint32_t *pcpre, *pcpost;            /* entry / exit PC (optional 17th column, F3_EXITPC) */
    uint16_t *srpre, *srpost;            /* SR (optional 18th column, F3_INJECT_EXC) */
} capture_t;

static inline uint8_t *cap_slurp(const char *path, long *len)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); *len = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = *len >= 0 ? malloc(*len ? (size_t)*len : 1) : NULL;
    if (!b || fread(b, 1, (size_t)*len, f) != (size_t)*len) {
        free(b);
        fclose(f);
        errno = EIO;
        return NULL;
    }
    fclose(f);
    return b;
}

/* 0 on success, -1 if the capture is missing, -2 for an invalid EEPROM file. */
static inline int cap_load(capture_t *c, const char *pfx)
{
    char p[512];
    long l;
    memset(c, 0, sizeof *c);
    /* capture_fn.sh writes <prefix>.bin; older sets were kept as
     * <prefix>_pairs.bin. Accept either. */
    snprintf(p, sizeof p, "%s_pairs.bin", pfx);
    c->wram = cap_slurp(p, &l);
    if (!c->wram) {
        snprintf(p, sizeof p, "%s.bin", pfx);
        c->wram = cap_slurp(p, &l);
    }
    if (!c->wram) return -1;
    c->pairs = (int)(l / (2 * F3_WRAM_SIZE));
    snprintf(p, sizeof p, "%s_eep.bin", pfx);
    c->eep = cap_slurp(p, &l);
    if (c->eep ? l != (long)c->pairs * 2 * CAP_EEP_RECORD : errno != ENOENT) {
        free(c->eep);
        free(c->wram);
        memset(c, 0, sizeof *c);
        return -2;
    }
    snprintf(p, sizeof p, "%s_dpram.bin", pfx);
    if ((c->dpram = cap_slurp(p, &l)) && l / (2 * F3_DPRAM_SIZE) < c->pairs) c->pairs = (int)(l / (2 * F3_DPRAM_SIZE));
    snprintf(p, sizeof p, "%s_spr.bin", pfx);
    if ((c->spr = cap_slurp(p, &l)) && l / (2 * F3_SPR_SIZE) < c->pairs) c->pairs = (int)(l / (2 * F3_SPR_SIZE));
    snprintf(p, sizeof p, "%s_gfx.bin", pfx);
    if ((c->gfx = cap_slurp(p, &l)) && l / (2 * F3_GFX_SIZE) < c->pairs) c->pairs = (int)(l / (2 * F3_GFX_SIZE));
    snprintf(p, sizeof p, "%s_pal.bin", pfx);
    if ((c->pal = cap_slurp(p, &l)) && l / (2 * F3_PAL_SIZE) < c->pairs) c->pairs = (int)(l / (2 * F3_PAL_SIZE));
    c->pre = calloc((size_t)c->pairs + 1, sizeof *c->pre);
    c->post = calloc((size_t)c->pairs + 1, sizeof *c->post);
    c->fpre = calloc((size_t)c->pairs + 1, sizeof *c->fpre);
    c->fpost = calloc((size_t)c->pairs + 1, sizeof *c->fpost);
    c->pcpost = calloc((size_t)c->pairs + 1, sizeof *c->pcpost);
    c->pcpre = calloc((size_t)c->pairs + 1, sizeof *c->pcpre);
    snprintf(p, sizeof p, "%s_regs.tsv", pfx);
    FILE *f = fopen(p, "r");
    char line[512];
    if (f && fgets(line, sizeof line, f)) {
        if (strstr(line, "\tPC\tSR")) {
            c->srpre = calloc((size_t)c->pairs + 1, sizeof *c->srpre);
            c->srpost = calloc((size_t)c->pairs + 1, sizeof *c->srpost);
        }
        while (fgets(line, sizeof line, f)) {
            int i; char which[8]; unsigned fr;
            if (sscanf(line, "%d\t%7s\t%x", &i, which, &fr) != 3 || i >= c->pairs) continue;
            uint32_t *dst = !strcmp(which, "pre") ? c->pre[i] : !strcmp(which, "post") ? c->post[i] : NULL;
            if (!dst) continue;
            if (dst == c->pre[i]) c->fpre[i] = fr; else c->fpost[i] = fr;
            char *s = line;
            for (int t = 0; t < 3; t++) s = strchr(s, '\t') + 1;
            for (int k = 0; k < CR_N; k++) dst[k] = (uint32_t)strtoul(s, &s, 16);
            int post = dst == c->post[i];
            (post ? c->pcpost : c->pcpre)[i] = (uint32_t)strtoul(s, &s, 16);
            if (c->srpre)
                (post ? c->srpost : c->srpre)[i] = (uint16_t)strtoul(s, NULL, 16);
        }
    }
    if (f) fclose(f);
    return 0;
}

static inline const uint8_t *cap_wram(const capture_t *c, int i, int post)
{ return c->wram + ((size_t)i * 2 + (size_t)post) * F3_WRAM_SIZE; }
static inline const uint8_t *cap_spr(const capture_t *c, int i, int post)
{ return c->spr ? c->spr + ((size_t)i * 2 + (size_t)post) * F3_SPR_SIZE : NULL; }

static inline const uint8_t *cap_dpram(const capture_t *c, int i, int post)
{ return c->dpram ? c->dpram + ((size_t)i * 2 + (size_t)post) * F3_DPRAM_SIZE : NULL; }

static inline const uint8_t *cap_gfx(const capture_t *c, int i, int post)
{ return c->gfx ? c->gfx + ((size_t)i * 2 + (size_t)post) * F3_GFX_SIZE : NULL; }

static inline const uint8_t *cap_pal(const capture_t *c, int i, int post)
{ return c->pal ? c->pal + ((size_t)i * 2 + (size_t)post) * F3_PAL_SIZE : NULL; }

static inline uint32_t cap_be(const uint8_t *p, unsigned n)
{
    uint32_t result = 0;
    for (unsigned i = 0; i < n; ++i) result = (result << 8) | p[i];
    return result;
}

/* Big-endian device words, SR, six serial bytes (phase, pins, lock, command,
 * address, DO), then four u32s (bit count, command accumulator, shift and
 * nanoseconds until ready). See scripts/lua/capture_fn.lua. */
static inline int cap_eeprom(const capture_t *c, int i, int post,
                             f3_eeprom_t *e, uint16_t *sr, uint8_t *dout)
{
    const uint8_t *p = c->eep + ((size_t)i * 2 + (size_t)post) * CAP_EEP_RECORD;
    for (unsigned k = 0; k < F3_EEP_WORDS; ++k)
        e->words[k] = (uint16_t)cap_be(p + 2 * k, 2);
    p += CAP_EEP_BYTES;
    *sr = (uint16_t)cap_be(p, 2);
    e->phase = p[2];
    e->pins = p[3];
    e->write_enabled = !p[4];
    e->command = p[5];
    e->address = p[6];
    *dout = p[7];
    e->bits = cap_be(p + 8, 4);
    e->command_bits = cap_be(p + 12, 4);
    e->shift = cap_be(p + 16, 4);
    e->busy_ns = cap_be(p + 20, 4);
    return e->phase <= F3_EEP_COMPLETE && !(e->pins & ~(F3_EEP_CS | F3_EEP_CLK | F3_EEP_DI))
        && p[4] <= 1 && e->command <= F3_EEP_ERASE_ALL && e->address < F3_EEP_WORDS
        && *dout <= 1 && e->command_bits <= 0xFFu
        && (e->phase != F3_EEP_COMMAND || (e->bits < 8 && e->command_bits < (1u << e->bits)))
        && (e->phase != F3_EEP_DATA || e->bits < 16);
}

static inline void cap_free(capture_t *c)
{
    free(c->wram);
    free(c->dpram);
    free(c->spr);
    free(c->gfx);
    free(c->pal);
    free(c->eep);
    free(c->pre);
    free(c->post);
    free(c->fpre);
    free(c->fpost);
    free(c->pcpre);
    free(c->pcpost);
    free(c->srpre);
    free(c->srpost);
}

/* byte diff of a region, printing up to *budget mismatches; skip [lo,hi) */
/* Engine-side capture artifact: P1's PS_INVULN high byte (0x41B1E1) flips
 * 0xD1 -> 0xD2 between the pre and post snapshot of 541 pairs across 47
 * captures. It is not a game write: measured over all 50,597 captured pairs
 * the transition is ALWAYS exactly D1->D2, the capture frame counter does not
 * advance across those pairs, and no translation in src/ writes 0x41B1xx.
 * The invulnerability counter lives in 0x41B19x and is only armed by the
 * player respawn (0xD2), so a player tick ran inside those posts.
 * Excluding it keeps a routine from being failed for work it never did; the
 * exclusion is one byte and is reported here rather than hidden. */
#define CAP_ARTIFACT_LO 0x41B1E1u
#define CAP_ARTIFACT_HI 0x41B1E2u
/* Sound DPRAM 0xC00482 (word): the 68000 sound CPU's ring READ index, i.e. the
 * other side of the MB8421 (docs/PROGRESS.md: C00480 = write idx*2, C00482 =
 * sound-CPU read idx). The peer CPU consumes commands while the main-CPU call
 * is being captured, so the byte moves between the pre and post dump with no
 * main-CPU code behind it: fnk_0039B8 pair 19 has 0x5E -> 0x6C there right
 * after 0x0039B8 queued "ABC" into the ring (the only pair of 30 that does,
 * although all 30 calls are byte-identical). No translation in src/ writes
 * 0xC00482, and the main CPU's own index at 0xC00480 is still compared. */
#define CAP_ARTIFACT_DPRAM_LO 0xC00482u
#define CAP_ARTIFACT_DPRAM_HI 0xC00484u

static inline int cap_diff_region(const char *tag, int pair, uint32_t base, const uint8_t *got,
                                  const uint8_t *want, uint32_t n, uint32_t lo, uint32_t hi,
                                  int allow_artifacts, int *budget)
{
    int d = 0;
    for (uint32_t o = 0; o < n; o++) {
        uint32_t a = base + o;
        if (a >= lo && a < hi) continue;
        if (allow_artifacts &&
            ((a >= CAP_ARTIFACT_LO && a < CAP_ARTIFACT_HI) ||
             (a >= CAP_ARTIFACT_DPRAM_LO && a < CAP_ARTIFACT_DPRAM_HI))) continue;
        if (got[o] != want[o]) {
            if ((*budget)-- > 0) printf("  %s pair %d @%06X got=%02X want=%02X\n", tag, pair, a, got[o], want[o]);
            d++;
        }
    }
    return d;
}

static inline int cap_diff(const char *tag, int pair, uint32_t base, const uint8_t *got,
                           const uint8_t *want, uint32_t n, uint32_t lo, uint32_t hi, int *budget)
{
    return cap_diff_region(tag, pair, base, got, want, n, lo, hi, 1, budget);
}
#endif
