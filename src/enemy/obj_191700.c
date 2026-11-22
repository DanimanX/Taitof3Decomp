#include "obj_191700.h"
#include "../video/sprite.h"
/* ROM 0x191700: not decompiled yet (stub)
 * Translation attempted (slide left 5; +0x1F -2; +0x1E -0x10; then
 * y += (word)@+0x1E & 0x7F00 per bfextu {0:7}; draw; back to 0x1916F8 while
 * +0x1E != 0, else +0x60 = 0x400 and continue at 0x191742) and it matched
 * only 2/60. scripts/capture_obj.sh runs with F3_EXIT=104F0 (the tick-list
 * loop head), so its "post" is taken at the next caller hit, one tick after
 * this routine returned, not at its rts. The capture must be redone with a
 * return-address exit before this can be verified; the stub stays. */
int Obj_191700_Tick(const f3mem_t *m, uint32_t obj) { (void)m; (void)obj; return -1; }
