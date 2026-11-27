/* ROM 0x1672A8: draw the static overlay of one stage screen. Eight
 * 0x0053B2 run-length tilemap blits into the 32x32 pages at 0x610000,
 * 0x610400, 0x610020, 0x610420, 0x610040, 0x610440, 0x610060 and 0x610460
 * from the streams at 0x80E1C, 0x80E6C, 0x80EAE, 0x80EDE, 0x80EE6, 0x80F30
 * and 0x66D20 (the last one twice), all with the flags word 0; then spawns
 * one object of routine 0x1673DE in group 0 (arg 0, one group), stores the
 * new object pointer at A5+0x3BD2 (0x40BBD2) and re-arms A0 with 0x1673AA.
 * A0 is saved and restored; only D0 changes (it holds the child pointer). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"   /* Obj_Spawn (0x01057C) */
#include "../video/sprite.h"       /* Gfx_DrawCompressedTilemap (0x0053B2) */

int Fn_1672A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];

    /* 0x1672AC..0x1672C2: word 0, pea dst, long src, jsr 0x53B2 */
    Gfx_DrawCompressedTilemap(m, 0x00080E1Cu, 0x00610000u, 0u);
    /* 0x1672C6..0x1672DC */
    Gfx_DrawCompressedTilemap(m, 0x00080E6Cu, 0x00610400u, 0u);
    /* 0x1672E0..0x1672F6 */
    Gfx_DrawCompressedTilemap(m, 0x00080EAEu, 0x00610020u, 0u);
    /* 0x1672FA..0x167310 */
    Gfx_DrawCompressedTilemap(m, 0x00080EDEu, 0x00610420u, 0u);
    /* 0x167314..0x16732A */
    Gfx_DrawCompressedTilemap(m, 0x00080EE6u, 0x00610040u, 0u);
    /* 0x16732E..0x167344 */
    Gfx_DrawCompressedTilemap(m, 0x00080F30u, 0x00610440u, 0u);
    /* 0x167348..0x16735E */
    Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00610060u, 0u);
    /* 0x167362..0x167378 */
    Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00610460u, 0u);

    /* 0x16737C movem.l (SP)+,A0 */
    /* 0x167380..0x167394: push arg 0, count 1, group 0, fn 0x1673DE;
     * jsr 0x1057C; lea (0xc,SP),SP */
    uint32_t child = Obj_Spawn(m, a0, 0x001673DEu, 0u, 1u, 0u);
    wr32(m, F3_A5 + 0x3BD2u, child);             /* 0x16739E move.l D0,(0x3bd2,A5) */
    wr32(m, a0, 0x001673AAu);                    /* 0x1673A2 move.l #0x1673aa,(A0) */
    r->d[0] = child;
    return 0;                                    /* 0x1673A8 rts */
}
