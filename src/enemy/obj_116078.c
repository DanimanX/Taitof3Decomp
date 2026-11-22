#include "obj_116078.h"
/* ROM 0x116078: not decompiled yet (stub)
 * Translation attempted from bundle_116078.s (parent flag 0x10/0x01 ->
 * 0x105EC Obj_Free; else 0xCBD52 Obj_Animate with the PC table at 0x116200,
 * 0x113400 position before 0x1282 Obj_DrawSprite, timer +0x6, then 0x1160C4).
 * Against the golden capture the harness aborted with
 * "f3mem: access outside RAM/ROM at 2C082D" (the object's parent pointer
 * +0x8 taken from work RAM points into the 0x2C0000 I/O window, which
 * f3mem does not model), and the capture posts one tick late
 * (F3_EXIT=104F0, see obj_191700.c). Needs a recapture / an I/O-window
 * decision before it can be verified. */
int Obj_116078_Tick(const f3mem_t *m, uint32_t obj) { (void)m; (void)obj; return -1; }
