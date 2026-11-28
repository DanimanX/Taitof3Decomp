/* Elevator Action Returns (elvactr) register-level routines. Same convention
 * as dariusg's src/auto (fn_auto.h): each Ea_<ADDR> takes the 68k register
 * file with A7 at the return address and leaves it as the ROM does after its
 * rts; a return < 0 means the call reached a path not decompiled yet.
 * Accepted only bit-exact against a MAME capture (elvactr/tests/fn_auto.inc,
 * make test-elvactr). */
#ifndef EA_AUTO_H
#define EA_AUTO_H
#include "../../src/common/f3mem.h"
#include "../../src/common/f3regs.h"
#include "../../src/auto/fn_call_frames.h"   /* push_long, push_word, set_word */
/* BEGIN generated */
int Ea_001338(const f3mem_t *m, f3regs_t *r); /* ROM 0x001338 */
int Ea_002E8C(const f3mem_t *m, f3regs_t *r); /* ROM 0x002E8C */
int Ea_000AD6(const f3mem_t *m, f3regs_t *r); /* ROM 0x000AD6 */
/* END generated */
#endif
