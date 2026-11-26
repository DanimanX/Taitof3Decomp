#ifndef TAITOF3_ENEMY_FN_11BC6E_H
#define TAITOF3_ENEMY_FN_11BC6E_H
#include "../common/f3mem.h"
/* ROM 0x11BC6E: (D0 preserved) probability gate 0x0CC2BC(0xFA); when it hits,
 * spawn one 0x11BCB2 object per entry of the A5+0x1D2C list (group 0, count 1,
 * the list entry as the argument). */
void Obj_11BC6E_SpawnWave(const f3mem_t *m, uint32_t a0);
#endif
