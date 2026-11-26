#ifndef TAITOF3_OBJ_18765E_H
#define TAITOF3_OBJ_18765E_H
#include "../common/f3mem.h"

/* ROM 0x17B3AE: upload the 8 boss palettes (alternate set when A5+0x32DC == 1). */
void Obj_17B3AE(const f3mem_t *m);

/* ROM 0x1847E8: reset the boss globals at A5+0x4B02.. and the object flags. */
void Obj_1847E8(const f3mem_t *m, uint32_t obj);

/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_18765E_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x18765E */

/* ROM 0x18495E: scale boss HP (+0x24 high word) by difficulty nibble A5+0x87F.
 * (Also declared as Obj_18495E in ../objects/obj_shared_ext.h.) */
void Obj_18495E(const f3mem_t *m, uint32_t obj);
#endif
