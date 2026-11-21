/* Darius Gaiden (Taito F3) — object pool allocator and per-frame tick loops.
 * Decompiled from ROM 0x010488..0x0106A6 (Task_ObjectPoolRunner, slot 4, and
 * Task_ObjGroup11Runner, slot 14). Bitfields use 68020 numbering: bit offset 0
 * is the MSB of the byte at the base address. */
#include "obj_pool.h"

static void bf_set1(const f3mem_t *m, uint32_t base, uint32_t off, int v)
{
    uint32_t a = base + (off >> 3);
    uint8_t bit = (uint8_t)(0x80u >> (off & 7));
    uint8_t b = rd8(m, a);
    wr8(m, a, v ? (uint8_t)(b | bit) : (uint8_t)(b & ~bit));
}

uint32_t Obj_Spawn(const f3mem_t *m, uint32_t parent_a0, uint32_t fn,
                   uint16_t group, uint16_t ngroups, uint32_t arg)
{
    uint32_t a1 = OBJ_FREE_MASKS + 4u * group;
    uint32_t a2 = OBJ_BASE + (uint32_t)group * OBJ_GROUP_SIZE;   /* mulu.w */
    uint16_t g = group, end = (uint16_t)(group + ngroups);
    for (;;) {
        uint32_t field = rd32(m, a1);
        if (field) {                                  /* bfffo (A1){0:32} */
            uint32_t idx = 0;
            while (!(field & (0x80000000u >> idx))) idx++;
            bf_set1(m, a1, idx, 0);                   /* bfclr: allocate */
            a2 += (uint32_t)(int32_t)(int16_t)(idx * OBJ_SIZE);
            wr32(m, a2 + 0x0, fn);
            wr8(m, a2 + 0x4, (uint8_t)g);
            wr8(m, a2 + 0x5, (uint8_t)idx);
            wr32(m, a2 + 0xC, arg);
            wr32(m, a2 + 0x8, parent_a0);
            return a2;
        }
        a1 += 4; a2 += OBJ_GROUP_SIZE; g++;
        if (!((int16_t)g < (int16_t)end))
            return 0xFFFFFFFFu;                      /* pool full */
    }
}

void Obj_Free(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd8(m, obj + 4) << 5);
    d0 = (uint16_t)((d0 & 0xFF00u) | ((d0 + rd8(m, obj + 5)) & 0xFFu)); /* add.b */
    bf_set1(m, OBJ_FREE_MASKS, d0, 1);
}

static void reset_masks(const f3mem_t *m, uint16_t g, uint16_t n, int both)
{
    uint16_t end = (uint16_t)(g + n);
    do {
        wr32(m, OBJ_FREE_MASKS + 4u * g, 0xFFFFFFFFu);
        if (both) wr32(m, OBJ_SNAP_MASKS + 4u * g, 0xFFFFFFFFu);
        g++;
    } while ((int16_t)g < (int16_t)end);
}

void Obj_ResetGroups(const f3mem_t *m, uint16_t group, uint16_t n) { reset_masks(m, group, n, 0); }
void Obj_ResetPools(const f3mem_t *m) { reset_masks(m, 0, 11, 1); }
void Obj_ResetGroup11(const f3mem_t *m) { reset_masks(m, 11, 1, 1); }

static void run(const f3mem_t *m, uint16_t g0, uint16_t n, obj_tick_cb cb, void *ctx)
{
    for (uint16_t g = 0; g < n; g++)
        wr32(m, OBJ_SNAP_MASKS + 4u * (g0 + g), rd32(m, OBJ_FREE_MASKS + 4u * (g0 + g)));
    uint32_t obj = OBJ_BASE + (uint32_t)g0 * OBJ_GROUP_SIZE;
    for (uint16_t g = 0; g < n; g++) {
        uint32_t snap = rd32(m, OBJ_SNAP_MASKS + 4u * (g0 + g));
        for (int bit = 31; bit >= 0; bit--, obj += OBJ_SIZE)
            if (!(snap & (1u << bit)))
                cb(m, rd32(m, obj), obj, ctx);
    }
}

void Obj_RunGroups(const f3mem_t *m, obj_tick_cb cb, void *ctx) { run(m, 0, 11, cb, ctx); }
void Obj_RunGroup11(const f3mem_t *m, obj_tick_cb cb, void *ctx) { run(m, 11, 1, cb, ctx); }
