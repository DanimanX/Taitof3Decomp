#ifndef TAITOF3_OBJ_POOL_H
#define TAITOF3_OBJ_POOL_H
/* Object pool: 12 groups x 32 objects x 0x6C bytes at 0x410060 + g*0xD80.
 * free mask (1 = free, MSB = index 0) per group at 0x410000[g];
 * per-frame snapshot of the masks at 0x410030[g]. Object header:
 * +0 fn (tick routine), +4 group.b, +5 index.b, +8 parent (caller A0), +C arg. */
#include "../common/f3mem.h"

#define OBJ_FREE_MASKS 0x410000u
#define OBJ_SNAP_MASKS 0x410030u
#define OBJ_BASE       0x410060u
#define OBJ_SIZE       0x6Cu
#define OBJ_GROUP_SIZE 0xD80u

/* Called for each live object in tick order; fn = ROM address of its routine. */
typedef void (*obj_tick_cb)(const f3mem_t *m, uint32_t fn, uint32_t obj, void *ctx);

uint32_t Obj_Spawn(const f3mem_t *m, uint32_t parent_a0, uint32_t fn,
                   uint16_t group, uint16_t ngroups, uint32_t arg); /* ROM 0x01057C */
void Obj_Free(const f3mem_t *m, uint32_t obj);                      /* ROM 0x0105EC */
void Obj_ResetGroups(const f3mem_t *m, uint16_t group, uint16_t n); /* ROM 0x01060E */
void Obj_ResetPools(const f3mem_t *m);                              /* ROM 0x010638 */
void Obj_ResetGroup11(const f3mem_t *m);                            /* ROM 0x010672 */
void Obj_RunGroups(const f3mem_t *m, obj_tick_cb cb, void *ctx);    /* ROM 0x0104B8 */
void Obj_RunGroup11(const f3mem_t *m, obj_tick_cb cb, void *ctx);   /* ROM 0x010532 */

#endif
