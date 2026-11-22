#ifndef TAITOF3_OBJ_E8B72_H
#define TAITOF3_OBJ_E8B72_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_E8B72_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x0E8B72 */
/* Shared helpers of the 0xE-range script enemies (copied from obj_EEBB0.c,
 * verified there and in this batch) */
uint16_t Polar_Angle256(const f3mem_t *m, int16_t d0_in, int16_t d1_in); /* ROM 0x006422 */
uint16_t Polar_Distance(const f3mem_t *m, int16_t d0_in, int16_t d1_in); /* ROM 0x00649A */
void Obj_OrbitDebug(const f3mem_t *m, uint32_t obj); /* ROM 0x0CBB2C */
void Obj_HitFlash(const f3mem_t *m, uint32_t obj); /* ROM 0x0CBE34 */
void Obj_DieWithBonus(const f3mem_t *m, uint32_t obj, uint32_t d0); /* ROM 0x0CEE52 */
void Obj_DieWithBonusChk(const f3mem_t *m, uint32_t obj, uint32_t d0); /* ROM 0x0CEE48 */
void Obj_DieWithBonusDbg(const f3mem_t *m, uint32_t obj, uint32_t d0); /* ROM 0x0CEEA2 */
void Obj_DieWithBonusDbgChk(const f3mem_t *m, uint32_t obj, uint32_t d0); /* ROM 0x0CEE9A */
void Obj_ArmOnScreen(const f3mem_t *m, uint32_t obj); /* ROM 0x0CF7FE: arm the script when re-entering the screen */
void Obj_ResetAnimKey(const f3mem_t *m, uint32_t obj); /* ROM 0x0D0050 */
void Obj_SetAnimByDir(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0); /* ROM 0x0D005A */
void Obj_AnimStep(const f3mem_t *m, uint32_t obj); /* ROM 0x0D0690 */
uint16_t Diff_FireChance(const f3mem_t *m); /* ROM 0x0D0C9E */
void Coll_AddEnemy(const f3mem_t *m, uint32_t obj); /* ROM 0x0D0DE4 */
void Score_OnHit(const f3mem_t *m, uint32_t obj, uint32_t d0); /* ROM 0x0D0E04 */
void Score_AddCurrent(const f3mem_t *m, uint32_t d0); /* ROM 0x0D0E30 */
uint16_t Rng_Mix1D60(const f3mem_t *m); /* ROM 0x0D0E3A */
uint16_t Angle360(const f3mem_t *m, uint16_t d0, uint16_t d1); /* ROM 0x0D0E52 */
void Obj_AimAtPlayers(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x0D09B8 */
void Obj_SetScript(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x0CF1D8: install a script pointer and request the first step */
void Obj_InitMover(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x0CEC4C: movement init + per-slot path record (table 0x0CECE4) */
void Obj_InitPalette(const f3mem_t *m, uint32_t obj); /* ROM 0x0CEE2C: palette from table 0x0CEE40[+0x4E] */
void Obj_InitMoverB(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x0CEDE4 */
void Obj_SetAnim(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x0D0678 */
void Obj_SetFrame(const f3mem_t *m, uint32_t obj, uint32_t a1); /* ROM 0x0CFA86 */
void Sound_PlayHooked(const f3mem_t *m, uint16_t cmdw); /* ROM 0x13F238 + 0x0039D8 */
void Coll_AddEnemyHazard(const f3mem_t *m, uint32_t obj); /* ROM 0x0D0DC2 */
#endif
