#ifndef TAITOF3_STAGE_TIMELINE_H
#define TAITOF3_STAGE_TIMELINE_H
/* Stage timeline (Task_StageTimeline, slot 6): the stage scroll position
 * advances by a 16.16 speed each frame; whenever its integer part changes,
 * script entries {pos.w, data[6]} whose pos equals it spawn enemy-wave
 * objects (fn 0xCC11C, groups 5..6, arg = &data). An entry whose word at
 * data+4 is negative ends the script (boss / stage-end sequence). */
#include "../common/f3mem.h"

#define TL_SCRIPT_PTR  (F3_A5 + 0x3308)  /* -> next script entry */
#define TL_SCROLL_POS  (F3_A5 + 0x330C)  /* 16.16 */
#define TL_SCROLL_SPD  (F3_A5 + 0x3310)  /* 16.16 */
#define TL_LAST_POS    (F3_A5 + 0x3314)  /* integer part seen last frame */
#define TL_FX_STARTED  (F3_A5 + 0x331A)
#define FX_PTR         (F3_A5 - 0x5DEC)
#define FX_TIMER       (F3_A5 - 0x5DE8)
#define FX_MODE        (F3_A5 - 0x5DE6)
#define WAVE_OBJ_FN    0x0CC11Cu

void Fx_StartA(const f3mem_t *m, uint8_t id);        /* ROM 0x0015D0 (table 0x16890, id % 6) */
void Fx_StartB(const f3mem_t *m, uint8_t id);        /* ROM 0x0015EC (table 0x168C0, id % 8) */
/* One frame of the timeline loop (ROM 0x13F2D4..0x13F340, after the pause
 * check). Returns 1 when the end marker is reached (continues at 0x13F342),
 * else 0. *spawned counts wave objects spawned. */
int StageTimeline_Step(const f3mem_t *m, int *spawned);

#endif
