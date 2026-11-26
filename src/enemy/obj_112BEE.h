#ifndef TAITOF3_OBJ_112BEE_H
#define TAITOF3_OBJ_112BEE_H
#include "../common/f3mem.h"
/* ROM 0x112BEE: per-stage boss set-up, jmp through the table 0x112BFE by
 * A5+0x32DE (1-based). Returns -1 for stages not translated yet. */
int Obj_112BEE_Tick(const f3mem_t *m, uint32_t obj);
int Obj_113DA8(const f3mem_t *m, uint32_t obj);     /* ROM 0x113DA8 */
#endif
