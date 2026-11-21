#ifndef TAITOF3_PLAYER_MOVE_H
#define TAITOF3_PLAYER_MOVE_H
#include "../common/f3mem.h"

/* PlayerState (0x8A bytes; players[2] at 0x41B190), fields used here */
#define PS_POS_X      0x14 /* s32 16.16, high word = screen X */
#define PS_POS_Y      0x18 /* s32 16.16 */
#define PS_ANGLE      0x20 /* s16, 0..255 or -1 */
#define PS_HP         0x24 /* s32 16.16 */
#define PS_BANK       0x3A /* s32 16.16, [-2,+2] */
#define PS_SPEED      0x42 /* s16 */
#define PS_IO_CH_PTR  0x44 /* u32 -> io_ch[n].cur */
#define PS_SHIELD     0x4E /* s16 */
#define PS_INVULN     0x50 /* s16 frames */
#define PS_ARM_LEVEL  0x58 /* s16 */
#define PS_FLAGS      0x65 /* u8: bit0 lock movement */

#define P1_STATE_ADDR 0x41B190u

int  Terrain_TestPoint(const f3mem_t *m, int16_t x, int16_t y); /* ROM 0x0C6632 */
void Obj_SubScrollVel(const f3mem_t *m, uint32_t obj);          /* ROM 0x0CBE00 */
void Obj_MoveByAngle(const f3mem_t *m, uint32_t obj, uint16_t angle); /* ROM 0x0CBE12 */
void Player_MoveAndBounds(const f3mem_t *m, uint32_t player);   /* ROM 0x0C691A */

#endif
