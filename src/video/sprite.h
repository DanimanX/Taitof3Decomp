#ifndef TAITOF3_VIDEO_SPRITE_H
#define TAITOF3_VIDEO_SPRITE_H
/* Object -> sprite RAM emitter. Each frame objects append 16-byte F3 sprite
 * entries at the cursor A5-0xCA0 (0x407360, points into 0x600000..0x60FFFF).
 * Object fields used: +0x10 sprite def ptr {w.w, h.w, {tile.w, attr.w}[w*h]}
 * (or {1,1, tile, attr} single), +0x14 x.w, +0x18 y.w, +0x1C flip mode.b
 * (0..3) and attr high byte, +0x1D colour override.b, +0x1E zoom.w. */
#include "../common/f3mem.h"

#define SPR_CURSOR (F3_A5 - 0x0CA0)

void Obj_DrawSprite(const f3mem_t *m, uint32_t obj);  /* ROM 0x001282 */
void Obj_DrawSpriteSub(const f3mem_t *m, uint32_t obj, uint16_t d0, uint16_t d1, uint16_t d2); /* ROM 0x001410 */
void Video_SetSpriteScroll(const f3mem_t *m);         /* ROM 0x001232 */
void Gfx_InitLineRam(const f3mem_t *m);               /* ROM 0x005B0C */

#endif
