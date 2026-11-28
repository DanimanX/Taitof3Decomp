/* ROM 0x0ACB32: UNVERIFIED (no capture): draw the bank/debug tilemap into the
 * text layer. `movem.l d0-d7/a0-a1,-(a7)` then the three stack args
 * (word 0, long 0x610084, long A0) and `jsr 0x0053B2`, which is the
 * run-length compressed-tilemap blitter Gfx_DrawCompressedTilemap:
 *   Gfx_DrawCompressedTilemap(m, src = A0, dst = 0x610084, flags = 0)
 * 0x0053B2 saves d0-d7/a0-a2 itself, and the movem at 0x0ACB48 restores
 * d0-d7/a0-a1, so the register file is unchanged at the rts. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_0ACB32(const f3mem_t *m, f3regs_t *r)
{
    Gfx_DrawCompressedTilemap(m, r->a[0], 0x00610084u, 0u); /* 0x0ACB32..0x0ACB48 -> 0x0053B2 */
    /* 0x0ACB4C movem.l (a7)+,d0-d7/a0-a1 ; 0x0ACB50 rts: no register changes. */
    return 0;
}
