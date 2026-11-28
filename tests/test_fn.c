/* Generic runner for function captures (scripts/capture_fn.sh <addr> ret ...
 * -> work/ram/fn_<ADDR>_{pairs,regs,gfx,spr,dpram,pal,eep}): one call per pair.
 * Each pair loads pre Work RAM and optional screen memory, sprite RAM,
 * sound DPRAM, palette RAM and EEPROM snapshots. The C translation runs from
 * the captured pre registers through an adapter. The runner compares the post
 * snapshots, skipping the stack window [SP-0x100, SP) below the entry SP
 * (locals, saved registers and the callee's frames live there). Selected
 * output registers are compared under a mask when the table entry asks.
 * Exception captures compare PC, SR and all general registers, including
 * SP, and check the handler's register saves in Work RAM.
 *
 * Adapter contract: int fn(const f3mem_t *m, f3regs_t *r). r holds the pre
 * registers (D0-D7, A0-A6, A7 = SP at entry, so stack arguments are at
 * SP+4...); the adapter writes the registers the routine returns. A return
 * < 0 means the call reached a path that is not decompiled yet: that pair is
 * counted as "undecompiled", not as a mismatch, and fails the routine only if
 * no pair was observable (as test_objects does with REJECT).
 *
 * Env: F3_ONLY=<substring of the entry name> runs only the matching entries.
 * A missing capture is a SKIP. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "capture.h"
#include "../src/enemy/obj_1194C2.h"      /* Text_DrawString (0x005666) */
#include "../src/player/player_task.h"    /* Player_CoinCheckA (0x0C77BE) */
#include "../src/enemy/obj_CBF04.h"       /* Obj_CBEEA (0x0CBEEA) */
#include "../src/video/linefx_176390.h"   /* LineFx_176390 */
#include "../src/common/fn_leaf1.h"        /* Leaf_* (Strata draft) */
#include "../src/kernel/fn_010352.h"      /* Kernel_CreditLine010352 (0x010352) */
#include "../src/kernel/fn_010420.h"      /* Kernel_CoinStart010420 (0x010420) */
#include "../src/enemy/fn_18BF46.h"       /* Obj_18BF46_Step */
#include "../src/enemy/fn_0D5E56.h"       /* Obj_0D5E56_AI */
#include "../src/enemy/fn_0D5B70.h"       /* Obj_0D5B70_Step */
#include "../src/enemy/fn_11BC6E.h"       /* Obj_11BC6E_SpawnWave */

typedef int (*fn_adapter_t)(const f3mem_t *m, f3regs_t *r);

typedef struct { int reg; uint32_t mask; } reg_check_t;   /* reg: CR_* index */

typedef struct {
    const char *name;
    const char *cap;
    fn_adapter_t fn;
    reg_check_t out[CR_SR + 1];  /* mask 0 ends the list */
} fn_entry_t;

/* ---- adapters ---------------------------------------------------------- */

#include "../src/kernel/seg.h"
/* one adapter for every segment capture: resume at the capture's entry PC */
static int ad_seg(const f3mem_t *m, f3regs_t *r)
{
    r->pc = Seg_Run(m, r, r->pc);
    return r->pc == SEG_LEAVE ? -1 : 0;
}
/* General registers, with PC for segments and PC/SR for exceptions. */
#define CPU_REGS { CR_D0, ~0u }, { CR_D0 + 1, ~0u }, { CR_D0 + 2, ~0u }, { CR_D0 + 3, ~0u }, \
    { CR_D0 + 4, ~0u }, { CR_D0 + 5, ~0u }, { CR_D0 + 6, ~0u }, { CR_D0 + 7, ~0u }, \
    { CR_A0, ~0u }, { CR_A0 + 1, ~0u }, { CR_A0 + 2, ~0u }, { CR_A0 + 3, ~0u }, \
    { CR_A0 + 4, ~0u }, { CR_A0 + 5, ~0u }, { CR_A0 + 6, ~0u }, { CR_SP, ~0u }
#define SEGREG_CHECKS CPU_REGS, { CR_PC, ~0u }
#define SEGREGS { SEGREG_CHECKS }
#define EXCREGS { SEGREG_CHECKS, { CR_SR, 0xFFFFu } }

/* ROM 0x005666: (8,A6)=src.l (0xC,A6)=dst.l (0x10,A6)=col.w; D0 = end dst */
static int ad_005666(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    r->d[0] = Text_DrawString(m, rd32(m, sp + 4u), rd32(m, sp + 8u), rd16(m, sp + 12u));
    return 0;
}

/* ROM 0x0C77BE: A0 = player state; D0.b = result (upper bytes not modelled) */
static int ad_0C77BE(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | Player_CoinCheckA(m, r->a[0]);
    return 0;
}

/* ROM 0x0CBEEA: jmp target, A0 = obj, D1.w = child count, A1 = child fn */
static int ad_0CBEEA(const f3mem_t *m, f3regs_t *r)
{
    return Obj_CBEEA(m, r->a[0], (uint16_t)r->d[1], r->a[1]);
}

/* ROM 0x176390: D0 height, D1 depth, D7 first row, A0..A3 line tables */
static int ad_176390(const f3mem_t *m, f3regs_t *r)
{
    LineFx_176390(m, r->d[0], r->d[1], r->d[7], r->a[0], r->a[1], r->a[2], r->a[3]);
    return 0;
}

/* ROM 0x13D638: A0 = player, returns D1.w = slot (or -1), A1 = record */
static int ad_13D638(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1;
    int16_t d1 = Hiscore_Insert(m, r->a[0], &a1);
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)d1;
    r->a[1] = a1;
    return 0;
}

static int ad_L16AB58(const f3mem_t *m, f3regs_t *r) { Leaf_16AB58(m, r); return 0; }
static int ad_L16AB5C(const f3mem_t *m, f3regs_t *r) { Leaf_16AB5C(m, r); return 0; }
static int ad_L16AB6A(const f3mem_t *m, f3regs_t *r) { Leaf_16AB6A(m, r); return 0; }
static int ad_L16AB62(const f3mem_t *m, f3regs_t *r) { Leaf_16AB62(m, r); return 0; }
static int ad_L1766B6(const f3mem_t *m, f3regs_t *r) { Leaf_1766B6(m, r); return 0; }
static int ad_L0CEC40(const f3mem_t *m, f3regs_t *r) { Leaf_0CEC40(m, r); return 0; }
static int ad_L1766DA(const f3mem_t *m, f3regs_t *r) { Leaf_1766DA(m, r); return 0; }
static int ad_L1766EE(const f3mem_t *m, f3regs_t *r) { Leaf_1766EE(m, r); return 0; }
static int ad_L13E9B0(const f3mem_t *m, f3regs_t *r) { Leaf_13E9B0(m, r); return 0; }
static int ad_L0C660A(const f3mem_t *m, f3regs_t *r) { Leaf_0C660A(m, r); return 0; }
static int ad_L16A2F6(const f3mem_t *m, f3regs_t *r) { Leaf_16A2F6(m, r); return 0; }
static int ad_L13E994(const f3mem_t *m, f3regs_t *r) { Leaf_13E994(m, r); return 0; }
static int ad_L1766BE(const f3mem_t *m, f3regs_t *r) { Leaf_1766BE(m, r); return 0; }
static int ad_L13CAAE(const f3mem_t *m, f3regs_t *r) { Leaf_13CAAE(m, r); return 0; }

/* ROM 0x010352: no arguments; A5-relative state + ROM strings decide it */
static int ad_010352(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Kernel_CreditLine010352(m);
    return 0;
}

/* ROM 0x010420: no arguments */
static int ad_010420(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Kernel_CoinStart010420(m);
    return 0;
}

/* ROM 0x18BF46: A0 = object; D0 = 0 on the paths it returns from */
static int ad_18BF46(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = (uint32_t)Obj_18BF46_Step(m, r->a[0]);
    return 0;
}

/* ROM 0x0D5E56: A0 = object. No register output is checked: the
 * 0x0D5E56-0x0D5EAC prologue's callees leave D0 = 0xFFFF in the capture on
 * every pair, which none of 0x0CC1E0/0x0CC24A/0x0CC306 produces when they
 * pass. */
static int ad_0D5E56(const f3mem_t *m, f3regs_t *r)
{
    (void)Obj_0D5E56_AI(m, r->a[0]);
    return 0;
}

/* ROM 0x0D5B70: A0 = object; no result register, Work RAM only */
static int ad_0D5B70(const f3mem_t *m, f3regs_t *r)
{
    Obj_0D5B70_Step(m, r->a[0]);
    return 0;
}

/* ROM 0x11BC6E: A0 = parent of the spawned children; D0 is pushed and
 * popped unchanged */
static int ad_11BC6E(const f3mem_t *m, f3regs_t *r)
{
    Obj_11BC6E_SpawnWave(m, r->a[0]);
    return 0;
}

/* Kernel task loop bodies (src/kernel/tasks.c, Task*_Frame in src/auto):
 * captured from the instruction after the task's TRAP #5 to the next TRAP #5
 * (scripts/capture_fn.sh <resume> <yield>, F3_NOIRQ=1), so one pair is one
 * pass of the loop. Registers are scratch across a yield: RAM only. */
#include "../src/kernel/tasks.h"
static int ad_task_coll(const f3mem_t *m, f3regs_t *r) { (void)r; TaskCollision_Frame(m); return 0; }
void TaskStageTimeline_Frame(const f3mem_t *m);
static int ad_task_timeline(const f3mem_t *m, f3regs_t *r) { (void)r; TaskStageTimeline_Frame(m); return 0; }
/* task setup: from the task entry to its first TRAP #5 */
static int ad_task_coll_init(const f3mem_t *m, f3regs_t *r) { (void)r; TaskCollision_Init(m); return 0; }
/* 0x010516 (Group11 setup), injected with F3_INJECT=010516 from Y14_002500 */
static int ad_task_g11_init(const f3mem_t *m, f3regs_t *r) { (void)r; TaskGroup11_Init(m); return 0; }
#include "../src/kernel/kernel.h"
/* kernel calls (src/kernel/kernel.h): stop PC = the RTE reached */
#define KERN_AD(A) static int ad_k_##A(const f3mem_t *m, f3regs_t *r) { r->pc = Kern_##A(m, r); return 0; }
static uint32_t Kern_002274(const f3mem_t *m, f3regs_t *r) { return Kern_Schedule(m, r); }
KERN_AD(002274)
KERN_AD(001E76)
KERN_AD(001E0E)
KERN_AD(001E48)
KERN_AD(001F30)
KERN_AD(001F78)
/* VBlank path: per-pair input ports from <cap>_ports.tsv (F3_PORTS=1); the
 * frame tick's dead stack frame below *tick_sp is skipped (skip_lo/hi). */
static const char *cur_cap; static int cur_pair; static uint32_t skip_lo, skip_hi;
static const uint8_t *pair_ports(void)
{
    static char path[256]; static uint8_t tab[512][32]; static int n = -1;
    char want[256]; snprintf(want, sizeof want, "%s_ports.tsv", cur_cap);
    if (n < 0 || strcmp(path, want)) {
        snprintf(path, sizeof path, "%s", want); n = 0;
        FILE *f = fopen(path, "r"); char ln[128];
        while (f && fgets(ln, sizeof ln, f) && n < 512) {
            int k; char hx[65];
            if (sscanf(ln, "%d\t%64s", &k, hx) == 2 && k == n) {
                for (int b = 0; b < 32; b++) { unsigned v; sscanf(hx + 2 * b, "%2x", &v); tab[n][b] = (uint8_t)v; }
                n++;
            }
        }
        if (f) fclose(f);
    }
    return cur_pair < n ? tab[cur_pair] : NULL;
}
#define KERN_VB(A) static int ad_k_##A(const f3mem_t *m, f3regs_t *r) { \
    const uint8_t *p = pair_ports(); uint32_t tsp = 0; \
    r->pc = Kern_##A(m, r, p, &tsp); if (!r->pc) return -1; \
    skip_lo = tsp - 0x400u; skip_hi = tsp; return 0; }
KERN_VB(00222E)
KERN_VB(001A34)
KERN_VB(001A54)
static int ad_task_d15_init(const f3mem_t *m, f3regs_t *r) { (void)r; TaskSlot15_Init(m); return 0; }
static int ad_task_d15(const f3mem_t *m, f3regs_t *r) { r->pc = TaskSlot15_Frame(m, r); return r->pc ? 0 : -1; }
/* END kernel adapters */
static int ad_task_timeline_init(const f3mem_t *m, f3regs_t *r) { return Fn_13F2C6(m, r) < 0 ? -1 : 0; }

#define ALLREGS { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } }
#ifdef FN1
/* One-routine build for drafting (scripts/strata_fn.py):
 * cc -DFN1=Fn_ADDR -DFN1_ADDR='"ADDR"' tests/test_fn.c draft.c build/libf3.a */
int FN1(const f3mem_t *m, f3regs_t *r);
static const fn_entry_t fns[] = {
    { "Fn 0x" FN1_ADDR " draft", "work/ram/fn_" FN1_ADDR, FN1, ALLREGS },
};
#else
#include "../src/auto/fn_auto.h"
static const fn_entry_t fns[] = {
#include "fn_auto.inc"
    { "Fn 0x00466E EEPROM read",       "work/ram/fne2_00466E", Fn_00466E, { CPU_REGS } },
    { "Fn 0x0046C0 EEPROM write",      "work/ram/fne2_0046C0", Fn_0046C0, { CPU_REGS } },
    { "Fn 0x004C42 high-score restore", "work/ram/fne2_004C42", Fn_004C42, { CPU_REGS } },
    { "Fn 0x004D06 settings reload",   "work/ram/fne2_004D06", Fn_004D06, { CPU_REGS } },
    { "Fn 0x005080 settings defaults", "work/ram/fne2_005080", Fn_005080, { CPU_REGS } },
    { "Fn 0x00509A high-score defaults", "work/ram/fne2_00509A", Fn_00509A, { CPU_REGS } },
    { "Fn 0x005666 Text_DrawString",   "work/ram/fn_005666", ad_005666, { { CR_D0, 0xFFFFFFFFu } } },
    { "Fn 0x0C77BE Player_CoinCheckA", "work/ram/fn_0C77BE", ad_0C77BE, { { CR_D0, 0x000000FFu } } },
    { "Fn 0x0CBEEA Obj_CBEEA",         "work/ram/fn_0CBEEA", ad_0CBEEA, { { 0, 0u } } },
    { "Fn 0x13D638 Hiscore_Insert",    "work/ram/fn_13D638", ad_13D638, { { CR_D0 + 1, 0x0000FFFFu }, { CR_A0 + 1, 0xFFFFFFFFu }, { 0, 0u }, { 0, 0u } } },
    { "Fn 0x16AB58 Leaf_16AB58", "work/ram/fn_16AB58", ad_L16AB58, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x16AB5C Leaf_16AB5C", "work/ram/fn_16AB5C", ad_L16AB5C, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x16AB6A Leaf_16AB6A", "work/ram/fn_16AB6A", ad_L16AB6A, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x16AB62 Leaf_16AB62", "work/ram/fn_16AB62", ad_L16AB62, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x1766B6 Leaf_1766B6", "work/ram/fn_1766B6", ad_L1766B6, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x0CEC40 Leaf_0CEC40", "work/ram/fn_0CEC40", ad_L0CEC40, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x1766DA Leaf_1766DA", "work/ram/fn_1766DA", ad_L1766DA, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x1766EE Leaf_1766EE", "work/ram/fn_1766EE", ad_L1766EE, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x13E9B0 Leaf_13E9B0", "work/ram/fn_13E9B0", ad_L13E9B0, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x0C660A Leaf_0C660A", "work/ram/fn_0C660A", ad_L0C660A, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x16A2F6 Leaf_16A2F6", "work/ram/fn_16A2F6", ad_L16A2F6, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x13E994 Leaf_13E994", "work/ram/fn_13E994", ad_L13E994, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x1766BE Leaf_1766BE", "work/ram/fn_1766BE", ad_L1766BE, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x13CAAE Leaf_13CAAE", "work/ram/fn_13CAAE", ad_L13CAAE, { { CR_D0, 0xFFFFFFFFu }, { CR_D0 + 1, 0xFFFFFFFFu }, { CR_A0, 0xFFFFFFFFu }, { CR_A0 + 1, 0xFFFFFFFFu } } },
    { "Fn 0x010352 Kernel_CreditLine", "work/ram/fn_010352", ad_010352, { { 0, 0u } } },
    { "Fn 0x010420 Kernel_CoinStart",  "work/ram/fn_010420", ad_010420, { { 0, 0u } } },
    { "Fn 0x18BF46 Obj_18BF46_Step",   "work/ram/fn_18BF46", ad_18BF46, { { 0, 0u } } },
    { "Fn 0x0D5E56 Obj_0D5E56_AI",      "work/ram/fn_0D5E56", ad_0D5E56, { { 0, 0u } } },
    { "Fn 0x0D5B70 Obj_0D5B70_Step",  "work/ram/fn_0D5B70", ad_0D5B70, { { 0, 0u } } },
    { "Fn 0x11BC6E Obj_11BC6E_Wave",   "work/ram/fn_11BC6E", ad_11BC6E, { { CR_D0, 0xFFFFFFFFu } } },
    { "Fn 0x176390 LineFx_176390",     "work/ram/fn_176390", ad_176390, { { 0, 0u } } },
    { "Task 0x0C63F2 Collision loop",  "work/ram/task_coll", ad_task_coll, { { 0, 0u } } },
    { "Seg 0x13F5B8 13F5B8 group",  "work/ram/seg_13F5B8", ad_seg, SEGREGS },
    { "Seg 0x13F600 13F5B8 group",  "work/ram/seg_13F600", ad_seg, SEGREGS },
    { "Seg 0x13F65C 13F5B8 group",  "work/ram/seg_13F65C", ad_seg, SEGREGS },
    { "Seg 0x13DAD4 13DAD4 group",  "work/ram/seg_13DAD4", ad_seg, SEGREGS },
    { "Seg 0x13DBBA 13DAD4 group",  "work/ram/seg_13DBBA", ad_seg, SEGREGS },
    { "Seg 0x13DBFC 13DAD4 group",  "work/ram/seg_13DBFC", ad_seg, SEGREGS },
    { "Seg 0x13DC3A 13DAD4 group",  "work/ram/seg_13DC3A", ad_seg, SEGREGS },
    { "Seg 0x13DC9C 13DAD4 group",  "work/ram/seg_13DC9C", ad_seg, SEGREGS },
    { "Seg 0x13DCFE 13DAD4 group",  "work/ram/seg_13DCFE", ad_seg, SEGREGS },
    { "Seg 0x13DD58 13DAD4 group",  "work/ram/seg_13DD58", ad_seg, SEGREGS },
    { "Seg 0x13DDA6 13DAD4 group",  "work/ram/seg_13DDA6", ad_seg, SEGREGS },
    { "Seg 0x13DDB2 13DAD4 group",  "work/ram/seg_13DDB2", ad_seg, SEGREGS },
    { "Seg 0x13DDF0 13DAD4 group",  "work/ram/seg_13DDF0", ad_seg, SEGREGS },
    { "Seg 0x13D69A 13D69A group",  "work/ram/seg_13D69A", ad_seg, SEGREGS },
    { "Seg 0x13D74C 13D69A group",  "work/ram/seg_13D74C", ad_seg, SEGREGS },
    { "Seg 0x0B8404 0B8404 group",  "work/ram/seg_0B8404", ad_seg, SEGREGS },
    { "Seg 0x0B840A 0B8404 group",  "work/ram/seg_0B840A", ad_seg, SEGREGS },
    { "Seg 0x0B8986 0B8404 group",  "work/ram/seg_0B8986", ad_seg, SEGREGS },
    { "Seg 0x0B89AA 0B8404 group",  "work/ram/seg_0B89AA", ad_seg, SEGREGS },
    { "Seg 0x0B89D6 0B8404 group",  "work/ram/seg_0B89D6", ad_seg, SEGREGS },
    { "Seg 0x0B89E6 0B8404 group",  "work/ram/seg_0B89E6", ad_seg, SEGREGS },
    { "Seg 0x0B8A44 0B8404 group",  "work/ram/seg_0B8A44", ad_seg, SEGREGS },
    { "Seg 0x0B8AD6 0B8404 group",  "work/ram/seg_0B8AD6", ad_seg, SEGREGS },
    { "Seg 0x0B8E8C 0B8404 group",  "work/ram/seg_0B8E8C", ad_seg, SEGREGS },
    { "Seg 0x0B8EB0 0B8404 group",  "work/ram/seg_0B8EB0", ad_seg, SEGREGS },
    { "Seg 0x0B8ED4 0B8404 group",  "work/ram/seg_0B8ED4", ad_seg, SEGREGS },
    { "Seg 0x0B8F1E 0B8404 group",  "work/ram/seg_0B8F1E", ad_seg, SEGREGS },
    { "Seg 0x0B8F8A 0B8404 group",  "work/ram/seg_0B8F8A", ad_seg, SEGREGS },
    { "Seg 0x0B9062 0B8404 group",  "work/ram/seg_0B9062", ad_seg, SEGREGS },
    { "Seg 0x0B935A 0B935A group",  "work/ram/seg_0B935A", ad_seg, SEGREGS },
    { "Seg 0x0B938A 0B935A group",  "work/ram/seg_0B938A", ad_seg, SEGREGS },
    { "Seg 0x0B93E6 0B935A group",  "work/ram/seg_0B93E6", ad_seg, SEGREGS },
    { "Seg 0x0B9498 0B935A group",  "work/ram/seg_0B9498", ad_seg, SEGREGS },
    { "Seg 0x0B956C 0B935A group",  "work/ram/seg_0B956C", ad_seg, SEGREGS },
    { "Seg 0x0B958C 0B935A group",  "work/ram/seg_0B958C", ad_seg, SEGREGS },
    { "Seg 0x0B9632 0B935A group",  "work/ram/seg_0B9632", ad_seg, SEGREGS },
    { "Seg 0x19F504 19F504 group",  "work/ram/seg_19F504", ad_seg, SEGREGS },
    { "Seg 0x19F628 19F504 group",  "work/ram/seg_19F628", ad_seg, SEGREGS },
    { "Seg 0x19F630 19F504 group",  "work/ram/seg_19F630", ad_seg, SEGREGS },
    { "Seg 0x19F708 19F708 group",  "work/ram/seg_19F708", ad_seg, SEGREGS },
    { "Seg 0x19F748 19F708 group",  "work/ram/seg_19F748", ad_seg, SEGREGS },
    { "Seg 0x1A00E6 1A00E6 group",  "work/ram/seg_1A00E6", ad_seg, SEGREGS },
    { "Seg 0x1A0168 1A00E6 group",  "work/ram/seg_1A0168", ad_seg, SEGREGS },
    { "Seg 0x1A0A52 1A0A52 group",  "work/ram/seg_1A0A52", ad_seg, SEGREGS },
    { "Seg 0x1A0AC6 1A0A52 group",  "work/ram/seg_1A0AC6", ad_seg, SEGREGS },
    { "Seg 0x1A1282 1A1282 group",  "work/ram/seg_1A1282", ad_seg, SEGREGS },
    { "Seg 0x1A12E2 1A1282 group",  "work/ram/seg_1A12E2", ad_seg, SEGREGS },
    { "Seg 0x1A1A26 1A1A26 group",  "work/ram/seg_1A1A26", ad_seg, SEGREGS },
    { "Seg 0x1A1A6E 1A1A26 group",  "work/ram/seg_1A1A6E", ad_seg, SEGREGS },
    { "Seg 0x1A1856 1A1856 group",  "work/ram/seg_1A1856", ad_seg, SEGREGS },
    { "Seg 0x1A189E 1A1856 group",  "work/ram/seg_1A189E", ad_seg, SEGREGS },
    { "Seg 0x1A1944 1A1944 group",  "work/ram/seg_1A1944", ad_seg, SEGREGS },
    { "Seg 0x1A198C 1A1944 group",  "work/ram/seg_1A198C", ad_seg, SEGREGS },
    { "Task 0x0C63E8 Collision init",  "work/ram/task_init_coll", ad_task_coll_init, { { 0, 0u } } },
    { "Kernel 0x00222E preempt Z01",    "work/ram/fn_00222E", ad_k_00222E, SEGREGS },
    { "Kernel 0x00222E preempt Z04",    "work/ram/fn_00222E_z04", ad_k_00222E, SEGREGS },
    { "Kernel 0x001A34 IRQ2 VBlank",    "work/ram/fn_001A34", ad_k_001A34, SEGREGS },
    { "Kernel 0x001A54 IRQ3",           "work/ram/fn_001A54", ad_k_001A54, SEGREGS },
    { "Kernel 0x002274 scheduler",     "work/ram/seg_002274", ad_k_002274, SEGREGS },
    { "Kernel 0x001E76 TRAP 1",        "work/ram/fn_001E76", ad_k_001E76, SEGREGS },
    { "Kernel 0x001E0E TRAP 2",        "work/ram/fn_001E0E", ad_k_001E0E, SEGREGS },
    { "Kernel 0x001E48 TRAP 3",        "work/ram/fn_001E48", ad_k_001E48, SEGREGS },
    { "Kernel 0x001F30 TRAP 4",        "work/ram/fn_001F30", ad_k_001F30, SEGREGS },
    { "Kernel 0x001F78 TRAP 5",        "work/ram/fn_001F78", ad_k_001F78, SEGREGS },
    { "Kernel 0x001FE8 TRAP 6 RTE",    "work/ram/fnx_001FE8", Fn_001FE8, EXCREGS },
    { "Kernel 0x001FE8 TRAP 6 wait",   "work/ram/fnx_001FE8_sched", Fn_001FE8, EXCREGS },
    { "Kernel 0x001FAC TRAP 7",        "work/ram/fnx_001FAC", Fn_001FAC, EXCREGS },
    { "Kernel 0x00205C TRAP 8",        "work/ram/fnx_00205C", Fn_00205C, EXCREGS },
    { "Kernel 0x0020C8 TRAP 9",        "work/ram/fnx_0020C8", Fn_0020C8, EXCREGS },
    { "Kernel 0x002156 TRAP 10",       "work/ram/fnx_002156", Fn_002156, EXCREGS },
    { "Kernel 0x0021BE TRAP 11",       "work/ram/fnx_0021BE", Fn_0021BE, EXCREGS },
    { "Kernel 0x00220E TRAP 12",       "work/ram/fnx_00220E", Fn_00220E, EXCREGS },
    { "Kernel 0x001A6C IRQ5",          "work/ram/fnx_001A6C", Fn_001A6C, EXCREGS },
    { "Kernel 0x000400 RTE",           "work/ram/fnx_000400", Fn_000400, EXCREGS },
    { "Task 0x0038B2 Slot15 daemon loop", "work/ram/task_d15", ad_task_d15, SEGREGS },
    { "Task 0x0038B2 Slot15 paused",   "work/ram/task_d15p", ad_task_d15, SEGREGS },   /* F3_POKE8=406BB5=02 */
    { "Task 0x003912 Slot15 init",     "work/ram/task_init_d15", ad_task_d15_init, { { 0, 0u } } }, /* F3_INJECT */
    { "Task 0x010516 Group11 init",    "work/ram/task_init_g11", ad_task_g11_init, { { 0, 0u } } },
    { "Task 0x13F2C6 Timeline init",   "work/ram/task_init_tl", ad_task_timeline_init, { { 0, 0u } } },
    { "Task 0x13F2CC Timeline loop",   "work/ram/task_timeline", ad_task_timeline, { { 0, 0u } } },
};
#endif

/* ---- runner ------------------------------------------------------------ */

static uint8_t *slurp(const char *path, long *len) { return cap_slurp(path, len); }

static int diff_eeprom(const char *name, int pair, const f3_eeprom_t *got,
                       const f3_eeprom_t *want, uint8_t dout, int *budget)
{
    int diff = 0;
    for (unsigned i = 0; i < F3_EEP_WORDS; ++i) {
        if (got->words[i] == want->words[i]) continue;
        if ((*budget)-- > 0)
            printf("  %s pair %d EEPROM[%02X] got=%04X want=%04X\n",
                   name, pair, i, got->words[i], want->words[i]);
        ++diff;
    }
    if (got->phase != want->phase || got->pins != want->pins ||
        got->write_enabled != want->write_enabled || f3_eeprom_do(got) != dout) {
        if ((*budget)-- > 0)
            printf("  %s pair %d EEPROM phase/pins/EWEN/DO got=%u/%02X/%u/%u want=%u/%02X/%u/%u\n",
                   name, pair, got->phase, got->pins, got->write_enabled, f3_eeprom_do(got),
                   want->phase, want->pins, want->write_enabled, dout);
        ++diff;
    }
    return diff;
}

static int run(const fn_entry_t *e, const uint8_t *rom)
{
    capture_t c;
    int loaded = cap_load(&c, e->cap);
    if (loaded == -2 || (!loaded && strstr(e->cap, "/fne2_") && !c.eep)) {
        printf("fn: %-34s FAIL (missing or invalid %s_eep.bin)\n", e->name, e->cap);
        cap_free(&c);
        return 1;
    }
    if (loaded || c.pairs == 0) {
        printf("fn: %-34s SKIP (no %s)\n", e->name, e->cap);
        if (c.wram) cap_free(&c);
        return 0;
    }
    int exception = e->out[CR_SR].mask != 0;
    if (exception && !c.srpre) {
        printf("fn: %-34s FAIL (need PC/SR columns in %s_regs.tsv)\n", e->name, e->cap);
        cap_free(&c);
        return 1;
    }
    uint8_t *w = malloc(F3_WRAM_SIZE);
    uint8_t *g = c.gfx ? malloc(F3_GFX_SIZE) : NULL;
    uint8_t *s = c.spr ? malloc(F3_SPR_SIZE) : NULL;
    uint8_t *dp = c.dpram ? malloc(F3_DPRAM_SIZE) : NULL;
    uint8_t *pal = c.pal ? malloc(F3_PAL_SIZE) : NULL;
    if (!w || (c.gfx && !g) || (c.spr && !s) || (c.dpram && !dp) || (c.pal && !pal)) {
        fprintf(stderr, "fn: %s REJECT (cannot allocate capture buffers)\n", e->name);
        free(w); free(g); free(s); free(dp); free(pal); cap_free(&c);
        return 1;
    }
    int ok = 0, undec = 0, irq = 0, budget = 12;
    for (int i = 0; i < c.pairs; i++) {
        /* a call whose post dump is in a later frame than its pre dump was
         * interrupted by VBlank: the IRQ handler's writes (frame counter,
         * task mask, ...) are in the post state, so no routine can match it */
        /* the VBlank path itself advances the frame counter by one */
        int vb = e->fn == ad_k_00222E || e->fn == ad_k_001A34 || e->fn == ad_k_001A54;
        if (c.fpre && c.fpost[i] != c.fpre[i] + (vb && c.fpost[i] != c.fpre[i] ? 1u : 0u)) { irq++; continue; }
        memcpy(w, cap_wram(&c, i, 0), F3_WRAM_SIZE);
        if (g) memcpy(g, cap_gfx(&c, i, 0), F3_GFX_SIZE);
        if (s) memcpy(s, cap_spr(&c, i, 0), F3_SPR_SIZE);
        if (dp) memcpy(dp, cap_dpram(&c, i, 0), F3_DPRAM_SIZE);
        if (pal) memcpy(pal, cap_pal(&c, i, 0), F3_PAL_SIZE);
        f3_eeprom_t eeprom, expected;
        f3mem_t m = { w, rom, dp, s, g, pal, c.eep ? &eeprom : NULL };
        f3regs_t r = { 0 };
        uint8_t do_post = 0;
        if (c.eep) {
            uint16_t sr_post;
            uint8_t do_pre;
            if (!cap_eeprom(&c, i, 0, &eeprom, &r.sr, &do_pre) ||
                !cap_eeprom(&c, i, 1, &expected, &sr_post, &do_post) ||
                f3_eeprom_do(&eeprom) != do_pre) {
                if (budget-- > 0) printf("  %s pair %d invalid EEPROM snapshot\n", e->name, i);
                continue;
            }
        }
        for (int k = 0; k < 8; k++) { r.d[k] = c.pre[i][CR_D0 + k]; r.a[k] = c.pre[i][CR_A0 + k]; }
        r.pc = c.pcpre[i];
        if (exception) r.sr = c.srpre[i];
        cur_cap = e->cap; cur_pair = i; skip_lo = skip_hi = 0;
        if (e->fn(&m, &r) < 0) { undec++; continue; }
        if (skip_hi > skip_lo && skip_lo >= F3_WRAM_BASE && skip_hi <= F3_WRAM_BASE + F3_WRAM_SIZE)
            memcpy(w + (skip_lo - F3_WRAM_BASE), cap_wram(&c, i, 1) + (skip_lo - F3_WRAM_BASE), skip_hi - skip_lo);
        uint32_t sp = c.pre[i][CR_SP];
        int d = cap_diff(e->name, i, F3_WRAM_BASE, w, cap_wram(&c, i, 1), F3_WRAM_SIZE,
                         exception ? sp : sp - 0x100u, sp, &budget);
        if (g) d += cap_diff(e->name, i, F3_GFX_BASE, g, cap_gfx(&c, i, 1), F3_GFX_SIZE, 0, 0, &budget);
        if (s) d += cap_diff(e->name, i, F3_SPR_BASE, s, cap_spr(&c, i, 1), F3_SPR_SIZE, 0, 0, &budget);
        /* sound DPRAM 0xC00000-0xC007FF, for captures taken with F3_DPRAM=1 */
        if (dp) d += cap_diff(e->name, i, F3_DPRAM_BASE, dp, cap_dpram(&c, i, 1), F3_DPRAM_SIZE, 0, 0, &budget);
        if (pal) d += cap_diff(e->name, i, F3_PAL_BASE, pal, cap_pal(&c, i, 1), F3_PAL_SIZE, 0, 0, &budget);
        if (c.eep) d += diff_eeprom(e->name, i, &eeprom, &expected, do_post, &budget);
        for (int k = 0; k <= CR_SR && e->out[k].mask; k++) {
            int x = e->out[k].reg;
            uint32_t got, want;
            if (x == CR_PC) {
                got = r.pc;
                want = c.pcpost[i];
            } else if (x == CR_SR) {
                got = r.sr;
                want = c.srpost[i];
            } else {
                got = x < CR_A0 ? r.d[x] : r.a[x - CR_A0];
                want = c.post[i][x];
            }
            got &= e->out[k].mask;
            want &= e->out[k].mask;
            if (got != want) {
                if (budget-- > 0) {
                    char gpr[3] = { x < CR_A0 ? 'D' : 'A', '0' + (x & 7), '\0' };
                    const char *name = x == CR_PC ? "PC" : x == CR_SR ? "SR" : x == CR_SP ? "SP" : gpr;
                    printf("  %s pair %d %s got=%08X want=%08X\n", e->name, i, name, got, want);
                }
                d++;
            }
        }
        if (d == 0) ok++;
    }
    int obs = c.pairs - undec - irq;
    printf("fn: %-34s %3d/%3d calls bit-exact (%d undecompiled, gfx %s, spr %s, pal %s)%s",
           e->name, ok, obs, undec, g ? "yes" : "no", s ? "yes" : "no", pal ? "yes" : "no",
           obs == 0 ? " REJECT no observable call" : "");
    if (irq) printf(" [%d interrupted by VBlank, skipped]", irq);
    printf("\n");
    free(w); free(g); free(s); free(dp); free(pal); cap_free(&c);
    return (obs - ok) + (obs == 0 ? 1 : 0);
}

int main(void)
{
    long rl = 0;
    uint8_t *rom = slurp("work/maincpu.bin", &rl);
    if (!rom || rl < (long)F3_ROM_SIZE) { printf("fn: SKIP (need work/maincpu.bin)\n"); free(rom); return 0; }
    const char *only = getenv("F3_ONLY");
    int bad = 0;
    for (size_t k = 0; k < sizeof fns / sizeof fns[0]; k++) {
        if (only && !strstr(fns[k].name, only)) continue;
        bad += run(&fns[k], rom);
    }
    printf("fn: %s (%d failing calls)\n", bad ? "FAIL" : "PASS", bad);
    free(rom);
    return bad ? 1 : 0;
}
