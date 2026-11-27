#ifndef TAITOF3_KERNEL_TASKS_H
#define TAITOF3_KERNEL_TASKS_H
#include "../common/f3mem.h"
#include "../objects/obj_pool.h"

/* Kernel task bodies that are only a setup call and a TRAP #5 frame loop.
 * As in src/player/player_task.c, a task is split into the code before its
 * first yield (_Init) and one pass of the loop after the yield (_Frame).
 * No MAME capture covers them (UNVERIFIED); every callee is decompiled. */

void TaskCollision_Init(const f3mem_t *m);   /* ROM 0x0C63E8 Task_Collision */
void TaskCollision_Frame(const f3mem_t *m);  /* ROM 0x0C63F2 loop body */

void TaskGroup11_Init(const f3mem_t *m);     /* ROM 0x010502 Task_ObjGroup11Runner (+ 0x010516) */
/* ROM 0x010508 loop body: returns 1 when the pause bit kept it to the yield. */
int  TaskGroup11_Frame(const f3mem_t *m, obj_tick_cb cb, void *ctx);

#endif
