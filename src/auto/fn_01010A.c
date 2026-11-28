/* ROM 0x01010A: UNVERIFIED (no capture): test-mode / boot idle entry.
 * Enters supervisor mode with the cache off (0x01010A ori.w #0x700,SR ;
 * 0x010110 movec D0,CACR -- SR and CACR are outside f3regs_t), clears the two
 * write-only MMIO input ports 0x4A0004 / 0x4A0014, saves A0, then rebuilds the
 * video state: Sprite_ClearAll (0x00117E), Gfx_ClearAllLayers (0x005A12),
 * the character/font tile upload Fn_005A3A (0x005A3A), the scroll-register
 * flush Video_UploadScroll (0x001C90). A0 is restored and the string at A0 is
 * drawn into text RAM 0x61C792 with colour 7 through 0x005666. The last
 * instruction is `bra.s *` (0x01014C): the routine never returns, so the model
 * reports 0 after the draws instead of spinning. */
#include "fn_auto.h"
#include "../io/input.h"
#include "../kernel/fn_010352.h"
#include "../video/sprite.h"

int Fn_01010A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];                          /* 0x010124 move.l A0,-(A7) */

    /* 0x010114/0x01011C: move.b #0,0x4A0004 / 0x4A0014 -- write-only MMIO,
     * not modelled (same convention as IO_PollInputs in src/io/input.c). */
    Sprite_ClearAll(m);                             /* 0x010126 jsr 0x00117E */
    Gfx_ClearAllLayers(m);                          /* 0x01012A jsr 0x005A12 */
    if (Fn_005A3A(m, r) < 0)                        /* 0x01012E jsr 0x005A3A */
        return -1;
    Video_UploadScroll(m);                          /* 0x010132 jsr 0x001C90 */
    r->d[0] = Kernel_005666(m, a0, 0x0061C792u, 7u); /* 0x010138..0x010144 jsr 0x005666 */
    /* 0x010148 lea 0xA(A7),A7 ; 0x01014C bra.s 0x01014C: endless loop. */
    return 0;
}
