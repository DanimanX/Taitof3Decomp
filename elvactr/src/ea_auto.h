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
int Ea_0069F4(const f3mem_t *m, f3regs_t *r); /* ROM 0x0069F4 */
int Ea_003B96(const f3mem_t *m, f3regs_t *r); /* ROM 0x003B96 */
int Ea_003FF0(const f3mem_t *m, f3regs_t *r); /* ROM 0x003FF0 */
int Ea_001044(const f3mem_t *m, f3regs_t *r); /* ROM 0x001044 */
int Ea_0A485C(const f3mem_t *m, f3regs_t *r); /* ROM 0x0A485C */
int Ea_0BEB9A(const f3mem_t *m, f3regs_t *r); /* ROM 0x0BEB9A */
int Ea_0BF622(const f3mem_t *m, f3regs_t *r); /* ROM 0x0BF622 */
int Ea_0C019E(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C019E */
int Ea_1316B4(const f3mem_t *m, f3regs_t *r); /* ROM 0x1316B4 */
int Ea_0A4B32(const f3mem_t *m, f3regs_t *r); /* ROM 0x0A4B32 */
int Ea_0A8BA4(const f3mem_t *m, f3regs_t *r); /* ROM 0x0A8BA4 */
int Ea_0BDB38(const f3mem_t *m, f3regs_t *r); /* ROM 0x0BDB38 */
int Ea_0BDD0E(const f3mem_t *m, f3regs_t *r); /* ROM 0x0BDD0E */
int Ea_0BFC56(const f3mem_t *m, f3regs_t *r); /* ROM 0x0BFC56 */
int Ea_0C05A2(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C05A2 */
int Ea_0C07D6(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C07D6 */
int Ea_0C0E92(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C0E92 */
int Ea_0C18F2(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C18F2 */
int Ea_0C25A6(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C25A6 */
int Ea_0C3F44(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C3F44 */
int Ea_0C42A0(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C42A0 */
int Ea_0C44CA(const f3mem_t *m, f3regs_t *r); /* ROM 0x0C44CA */
int Ea_0D0EC2(const f3mem_t *m, f3regs_t *r); /* ROM 0x0D0EC2 */
int Ea_12FF6C(const f3mem_t *m, f3regs_t *r); /* ROM 0x12FF6C */
int Ea_1321DE(const f3mem_t *m, f3regs_t *r); /* ROM 0x1321DE */
int Ea_137584(const f3mem_t *m, f3regs_t *r); /* ROM 0x137584 */
int Ea_15655C(const f3mem_t *m, f3regs_t *r); /* ROM 0x15655C */
/* END generated */
#endif
