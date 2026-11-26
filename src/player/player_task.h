#ifndef TAITOF3_PLAYER_TASK_H
#define TAITOF3_PLAYER_TASK_H
#include "../common/f3mem.h"
#include "player_move.h" /* PS_* field offsets */

/* ROM 0x0C6A64 Task_PlayerControl (kernel slot 5): one call = one game frame
 * of the player lane. It runs its own yield points (TRAP #5) and the
 * stage-fail loop internally, as the ROM task does. Returns 0 when the call
 * completed, or -1 when it reached a ROM path that is not decompiled yet: no
 * side effect of that path is applied and the model stops at the yield (or
 * JSR) that starts it. */
int PlayerControl_RunFrame(const f3mem_t *m); /* ROM 0x0C6A64 */

/* Per-frame helpers of the same task, translated in player_task.c. Each
 * returns 0, or -1 at a not-yet-decompiled tail (0x001660, 0x5566, 0x0C8E1A,
 * 0x0C8BCE, 0x13D638, 0x0C8AB4..). */
int  Player_SceneInit(const f3mem_t *m);                   /* ROM 0x0C6B36, once per scene */
void Player_ShotPoolReset(const f3mem_t *m);               /* ROM 0x0C6DEC */
void Player_ArmRespawn(const f3mem_t *m, uint32_t player); /* ROM 0x0C6CE6 */
int  Player_Respawn(const f3mem_t *m, uint32_t player);    /* ROM 0x0C6D0E */
void Player_PickActive(const f3mem_t *m);                  /* ROM 0x0C6E10 */
uint8_t Coin_CheckStartReady(const f3mem_t *m);           /* ROM 0x000FEC */
uint8_t Coin_CoinsNeeded(const f3mem_t *m);               /* ROM 0x001016 */
uint8_t Player_CoinCheckA(const f3mem_t *m, uint32_t player); /* ROM 0x0C77BE: D0.b */
/* Text at text RAM 0x61C000 + (row << 7) + col*2, colour 7 (returns the end) */
uint32_t Player_PanelText(const f3mem_t *m, uint32_t str, uint16_t col, uint16_t row);
void Player_FailText(const f3mem_t *m, uint8_t d0b, uint16_t col, uint16_t row); /* ROM 0x0C7814 */
int  Player_DefeatBookKeeping(const f3mem_t *m, uint32_t player); /* ROM 0x0C7C60 */
int  Player_AliveShipTick(const f3mem_t *m, uint32_t player, uint32_t io_ptr); /* ROM 0x0C7946 */
int  Player_HudRow1(const f3mem_t *m);                     /* ROM 0x0C8488 */
void Player_HudRow2(const f3mem_t *m);                     /* ROM 0x0C87F8 */
int  Player_Hud(const f3mem_t *m);                         /* ROM 0x0C8A5E */
int  Player_StageFail(const f3mem_t *m, uint32_t player);  /* ROM 0x0C6E5A */

/* Values of player +0x00 (the per-player tick fn). Only the respawn pair below
 * is written by this lane; the ship fn the scene init spawns reads 0x0C7946 in
 * all three states, the task itself is armed with 0x0C7480. */
#define PLAYER_FN_STAGE_FAIL 0x0C7480u /* task fn of the 0x0C6E5A fail loop */
#define PLAYER_FN_SHIP       0x0C7946u /* alive ship tick (0x0C7946, set by 0x0C793E) */
#define PLAYER_FN_RESPAWN    0x0C793Eu /* respawn tick = 0x0C6CE6 / 0x0C6D0E */

int16_t Hiscore_Insert(const f3mem_t *m, uint32_t a0, uint32_t *slot); /* ROM 0x13D638 */
#endif
