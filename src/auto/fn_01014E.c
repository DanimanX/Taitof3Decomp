/* ROM 0x01014E: the soft-reset sequence entered by 0x004E3C (the EEPROM
 * write-back failure of 0x004DEE, and every other `jsr 0x01014E` in src/).
 * It never returns: the last instruction is `bra.b *` at 0x010170.
 *
 *   01014E  ori.w #0x700,SR        supervisor, mask 7 (SR is not in f3regs_t)
 *   010152  moveq #0,D0 ; 010154 movec D0,CACR   (CACR is not in f3regs_t)
 *   010158  move.b #0,0x4A0004 / 010160 move.b #0,0x4A0014  write-only MMIO
 *   010168  jsr 0x00117E           Sprite_ClearAll (movem d0-d1/a0-a1)
 *   01016C  jsr 0x005A12           Gfx_ClearAllLayers (all five callees save
 *                                  d0-d2/a0-a1, so it is transparent)
 *   010170  bra.b 0x010170         endless spin
 *
 * The capture stops AT 0x010170 (F3_EXIT=010170 F3_INJECT_ABORT=1). Both
 * callees save and restore every register they touch, so the state there is
 * exactly the caller's with D0 = 0 from 0x010152: no register, no SP and no
 * stack byte moves, and the only RAM effect is the two clear passes over
 * sprite RAM 0x600030-0x603FFF / 0x608030-0x60BFFF and the whole playfield
 * (take the capture with F3_SPR=1 F3_GFX=1 to compare them). */
#include "fn_auto.h"
#include "../video/sprite.h"        /* Sprite_ClearAll 0x00117E, Gfx_ClearAllLayers 0x005A12 */

int Fn_01014E(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = 0;                                    /* 0x010152 moveq #0,D0 */
    Sprite_ClearAll(m);                             /* 0x010168 jsr 0x00117E */
    Gfx_ClearAllLayers(m);                          /* 0x01016C jsr 0x005A12 */
    return 0;                                       /* 0x010170 bra.b 0x010170 */
}
