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
void Gfx_DrawTilemapBlock(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t flags); /* ROM 0x0055C8 */
void Gfx_DrawDecimal(const f3mem_t *m, uint32_t val, uint16_t digits, uint32_t dst, uint16_t attr); /* ROM 0x00571A */
void Gfx_ClearPf3(const f3mem_t *m);                  /* ROM 0x0058AE */
void Gfx_ClearTextLayer(const f3mem_t *m);            /* ROM 0x005880 */
void Gfx_ClearPf0(const f3mem_t *m);                  /* ROM 0x0058DA */
void Gfx_ClearPf0b(const f3mem_t *m);                 /* ROM 0x005918 */
void Gfx_ClearPf1AndLineRam(const f3mem_t *m);        /* ROM 0x005956 */
void Gfx_ClearPf2AndLineRam(const f3mem_t *m);        /* ROM 0x0059B4 */
void Gfx_ClearAllLayers(const f3mem_t *m);            /* ROM 0x005A12 */
void Gfx_UploadTextTiles(const f3mem_t *m, uint32_t src, uint16_t slot); /* ROM 0x005A86 */
void Gfx_InitLineRam(const f3mem_t *m);               /* ROM 0x005B0C */
void Stage_DrawBanner13D11E(const f3mem_t *m);        /* ROM 0x13D11E */

#endif
