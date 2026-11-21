#ifndef TAITOF3_OBJSCRIPT_CF1E4_H
#define TAITOF3_OBJSCRIPT_CF1E4_H
#include "../common/f3mem.h"
/* Object script interpreter (A0 = obj, script ptr at obj+0x44, 6-byte
 * opcodes). Returns 0, or -1 at a path not decompiled yet. */
int ObjScript_CF1E4(const f3mem_t *m, uint32_t obj); /* ROM 0xCF1E4 */
#endif
