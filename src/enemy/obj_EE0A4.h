#ifndef TAITOF3_OBJ_EE0A4_H
#define TAITOF3_OBJ_EE0A4_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_EE0A4_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0EE0A4 */
int Enemy_FormationShooter(const f3mem_t *m, uint32_t obj,
                           uint32_t cf4b8_tbl, uint32_t anim_tbl); /* ROM 0x0EDC38 body (copies at 0x0EE0A4, 0x0EE468, 0x0EE7D8) */
#endif
