#ifndef TAITOF3_ENEMY_FN_0D5E56_H
#define TAITOF3_ENEMY_FN_0D5E56_H
#include "../common/f3mem.h"
/* ROM 0x0D5E56: enemy AI preamble. Runs three spawn/proximity gates
 * (0x0CC1E0, 0x0CC24A, 0x0CC306); when all pass, aims 0x0CC33A(0x10,0x10) and
 * spawns one 0x0D901A object (group 9, count 2) through 0x01057C. */
int Obj_0D5E56_AI(const f3mem_t *m, uint32_t obj);
/* ROM 0x0CC1E0 / 0x0CC24A / 0x0CC306: the three AI gates of 0x0D5E56.
 * 0x0CC1E0 true if any object of the A5+0x1D2C list is within +-0x50 of this
 * object's centre in both axes; 0x0CC24A true while the centre is inside the
 * screen box; 0x0CC306 true when (0x0D0C9E() - (D0w+D1w)/2) >= 0. */
int Enemy_0CC1E0(const f3mem_t *m, uint32_t a0);
int Enemy_0CC24A(const f3mem_t *m, uint32_t a0);
int Enemy_0CC306(const f3mem_t *m, uint16_t d0w, uint16_t d1w);
#endif
