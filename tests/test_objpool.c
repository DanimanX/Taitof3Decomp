/* Differential tests for the object pool (src/objects/obj_pool.c) against
 * MAME debugger captures:
 *   scripts/capture_fn.sh 1057C 105EA 300 work/ram/objspawn   (Obj_Spawn)
 *   scripts/capture_fn.sh 105EC 1060C 300 work/ram/objfree    (Obj_Free)
 *   F3_LOG=104EE scripts/capture_fn.sh 104B8 10500 60 work/ram/objrun
 *     (Obj_RunGroups: every object tick call A0=obj, A1=fn is logged) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/objects/obj_pool.h"

enum { R_D0 = 0, R_A0 = 8, R_A1 = 9, R_SP = 15, NREG = 16 };
typedef struct { uint32_t pre[NREG], post[NREG]; uint32_t *log; int nlog; } regs_t;

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

static regs_t *load_regs(const char *path, int pairs)
{
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    regs_t *r = calloc((size_t)pairs, sizeof *r);
    char line[512];
    if (!fgets(line, sizeof line, f)) { fclose(f); return r; }
    while (fgets(line, sizeof line, f)) {
        int i; char which[8]; unsigned fr; uint32_t v[NREG];
        char *c = line;
        if (sscanf(c, "%d\t%7s\t%x", &i, which, &fr) != 3 || i >= pairs) continue;
        for (int t = 0; t < 3; t++) c = strchr(c, '\t') + 1;
        for (int k = 0; k < NREG; k++) { v[k] = (uint32_t)strtoul(c, &c, 16); }
        if (!strcmp(which, "pre")) memcpy(r[i].pre, v, sizeof v);
        else if (!strcmp(which, "post")) memcpy(r[i].post, v, sizeof v);
        else {
            r[i].log = realloc(r[i].log, (size_t)(r[i].nlog + 1) * 2 * sizeof(uint32_t));
            r[i].log[2 * r[i].nlog] = v[R_A1];
            r[i].log[2 * r[i].nlog + 1] = v[R_A0];
            r[i].nlog++;
        }
    }
    fclose(f);
    return r;
}

static int ram_diff(const uint8_t *got, const uint8_t *want, uint32_t sp, int pair, const char *tag, int *shown)
{
    int d = 0;
    for (uint32_t o = 0; o < F3_WRAM_SIZE; o++) {
        uint32_t a = F3_WRAM_BASE + o;
        if (a >= sp - 0x40 && a < sp) continue;   /* movem save area */
        if (got[o] != want[o]) {
            if ((*shown)++ < 8) printf("  %s pair %d @%06X got=%02X want=%02X\n", tag, pair, a, got[o], want[o]);
            d++;
        }
    }
    return d;
}

typedef struct { const uint32_t *log; int n, pos, bad; } cmp_ctx;
static void cmp_cb(const f3mem_t *m, uint32_t fn, uint32_t obj, void *vctx)
{
    (void)m;
    cmp_ctx *c = vctx;
    if (c->pos >= c->n || c->log[2 * c->pos] != fn || c->log[2 * c->pos + 1] != obj) c->bad++;
    c->pos++;
}

/* returns mismatches, or -1 if the capture is missing */
static int test(const char *pfx, int kind, int *npairs, int *extra)
{
    char p1[256], p2[256];
    snprintf(p1, sizeof p1, "%s_pairs.bin", pfx);
    snprintf(p2, sizeof p2, "%s_regs.tsv", pfx);
    long cl;
    uint8_t *cap = slurp(p1, &cl);
    if (!cap) return -1;
    int pairs = (int)(cl / (2 * F3_WRAM_SIZE)), bad = 0, shown = 0;
    regs_t *r = load_regs(p2, pairs);
    uint8_t *work = malloc(F3_WRAM_SIZE);
    *extra = 0;
    for (int i = 0; i < pairs; i++) {
        const uint8_t *pre = cap + (size_t)i * 2 * F3_WRAM_SIZE, *post = pre + F3_WRAM_SIZE;
        memcpy(work, pre, F3_WRAM_SIZE);
        f3mem_t m = { work, NULL, NULL };
        uint32_t sp = r[i].pre[R_SP];
        int d = 0;
        if (kind == 0) {
            uint32_t got = Obj_Spawn(&m, r[i].pre[R_A0], rd32(&m, sp + 4), rd16(&m, sp + 8),
                                     rd16(&m, sp + 10), rd32(&m, sp + 12));
            if (got != r[i].post[R_D0]) { d++; if (shown++ < 8) printf("  spawn pair %d D0 got=%08X want=%08X\n", i, got, r[i].post[R_D0]); }
            if (got == 0xFFFFFFFFu) (*extra)++;
            d += ram_diff(work, post, sp, i, "spawn", &shown);
        } else if (kind == 1) {
            Obj_Free(&m, rd32(&m, sp + 4));
            d += ram_diff(work, post, sp, i, "free", &shown);
        } else {
            cmp_ctx c = { r[i].log, r[i].nlog, 0, 0 };
            Obj_RunGroups(&m, cmp_cb, &c);
            if (c.pos != c.n) c.bad++;
            *extra += c.n;
            if (c.bad && shown++ < 8) printf("  run pair %d: %d calls in C vs %d in MAME, %d mismatches\n", i, c.pos, c.n, c.bad);
            d += c.bad;
            /* the loop itself only writes the snapshot masks */
            if (memcmp(work + (OBJ_SNAP_MASKS - F3_WRAM_BASE), post + (OBJ_SNAP_MASKS - F3_WRAM_BASE), 44)) {
                d++;
                if (shown++ < 8) printf("  run pair %d: snapshot masks differ\n", i);
            }
        }
        bad += d != 0;
    }
    for (int i = 0; i < pairs; i++) free(r[i].log);
    free(r); free(work); free(cap);
    *npairs = pairs;
    return bad;
}

int main(void)
{
    static const struct { const char *pfx, *name; int kind; } t[] = {
        { "work/ram/objspawn", "Obj_Spawn", 0 },
        { "work/ram/objspawn_full", "Obj_Spawn", 0 }, /* F3_POKE32: groups 0..10 full */
        { "work/ram/objfree", "Obj_Free", 1 },
        { "work/ram/objrun", "Obj_RunGroups", 2 },
    };
    int rc = 0, any = 0;
    for (int k = 0; k < 4; k++) {
        int n = 0, extra = 0, bad = test(t[k].pfx, t[k].kind, &n, &extra);
        if (bad < 0) { printf("objpool: %-14s SKIP (no %s capture)\n", t[k].name, t[k].pfx); continue; }
        any = 1;
        printf("objpool: %-14s %3d/%3d calls bit-exact", t[k].name, n - bad, n);
        if (t[k].kind == 0) printf(" (%d pool-full)", extra);
        if (t[k].kind == 2) printf(" (%d object ticks, same fn/obj order)", extra);
        printf("\n");
        if (bad || n == 0) rc = 1;
    }
    return any ? rc : 0;
}
