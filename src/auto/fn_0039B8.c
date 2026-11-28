/* ROM 0x0039B8: (recipe capture fnk_): append a length-prefixed byte
 * string (A0) to the sound DPRAM ring. The write index is DPRAM word 0xC00480
 * (byte offset *2); every register is saved and restored, so only the DPRAM
 * changes:
 *
 *   0039B8  movem.l {D0 D1 A0 A1 A2},-(SP)
 *   0039BC  A1 = 0xC00000 ; D1w = word 0xC00480 >> 1
 *   0039C8  bsr 0x003B4E Sound_CopyToRing: D0 = *A0++ (byte count), then that
 *           many bytes *A0++ -> DPRAM[D1++ & 0x3FF]
 *   0039CC  D1w += D1w ; word 0xC00480 = D1w
 *   0039D2  movem.l (SP)+,{D0 D1 A0 A1 A2} ; rts
 *
 * work/ram/fnk_0039B8 (30 injected pairs, F3_DPRAM=1): every pair is the same
 * call with A0 = 0x400200 pointing at {03, 'A', 'B', 'C'}, so the routine
 * copies 3 bytes to 0xC00000..0xC00002 from ring index 0 and advances the
 * index word 0xC00480 from 0 to 6. The null body leaves 0xC00480 = 0 and
 * 0xC00000..2 at their old values, so it fails all 30 pairs. Every
 * instruction is re-read from work/maincpu.bin at 0x0039B8. The peer sound
 * CPU's read index at 0xC00482 is the other side of the DPRAM handshake and
 * moves on its own (see CAP_ARTIFACT_DPRAM_LO in tests/capture.h).
 */
#include "fn_auto.h"

int Fn_0039B8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a0 = r->a[0], a1 = r->a[1], a2 = r->a[2];
    uint32_t d0 = r->d[0], d1 = r->d[1];
    uint16_t idx, count;
    uint8_t val;

    /* 0x0039B8 movem.l {D0 D1 A0 A1 A2},-(SP) */
    r->a[7] = sp - 20u;
    wr32(m, r->a[7] + 0u, d0);
    wr32(m, r->a[7] + 4u, d1);
    wr32(m, r->a[7] + 8u, a0);
    wr32(m, r->a[7] + 12u, a1);
    wr32(m, r->a[7] + 16u, a2);

    r->a[1] = 0x00C00000u;                        /* 0x0039BC lea */
    idx = (uint16_t)(rd16(m, 0x00C00480u) >> 1);  /* 0x0039C2/0x0039C6 */

    /* 0x0039C8 bsr 0x003B4E. The count byte is the number of bytes copied:
     * the dbf at 0x3B5E decrements first, and a count of 0 leaves 0xFFFF
     * (= -1) and falls straight through, so nothing is written. */
    count = rd8(m, r->a[0]);                       /* 0x3B4E/0x3B50 move.b (A0)+,D0 */
    r->a[0] += 1u;
    for (;;) {
        count = (uint16_t)(count - 1u);            /* 0x3B5E dbf D0 */
        if (count == 0xFFFFu)
            break;
        val = rd8(m, r->a[0]);                     /* 0x3B54 move.b (A0)+,(A1,D1w) */
        r->a[0] += 1u;
        wr8(m, 0x00C00000u + (uint32_t)(int32_t)(int16_t)idx, val);
        idx = (uint16_t)(idx + 1u);                /* 0x3B58/0x3B5A */
        idx = (uint16_t)(idx & 0x03FFu);
    }

    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)(idx + idx); /* 0x0039CC add.w */
    wr16(m, 0x00C00480u, (uint16_t)r->d[1]);       /* 0x0039CE */

    /* 0x0039D2 movem.l (SP)+,{D0 D1 A0 A1 A2} */
    r->d[0] = rd32(m, r->a[7] + 0u);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->a[0] = rd32(m, r->a[7] + 8u);
    r->a[1] = rd32(m, r->a[7] + 12u);
    r->a[2] = rd32(m, r->a[7] + 16u);
    r->a[7] = sp + 4u;                             /* the rts pops the caller's PC */
    return 0;
}
