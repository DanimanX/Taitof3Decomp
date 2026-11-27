/* ROM 0x14C464: install a 128-byte ROM block (0x14C4B6) at 0x40B3AE, then
 * queue a palette upload {count 0x19, src 0x40B3AE, dst 0x0E00} through the
 * ring at 0x4018B0 indexed by the word at 0x402210 (0x001660). D0/D1/A0/A1
 * come back untouched (saved at 0x14C464 and restored at 0x14C484). */
#include "fn_auto.h"
#include "../video/palette.h"

/* ROM 0x16A19C: forward longword copy, D0.w = count-1 (dbf). */
static void Fn14C464_Copy(const f3mem_t *m, uint32_t src, uint32_t dst,
                          uint16_t count)
{
    for (uint32_t i = 0; i <= (uint32_t)count; i++)   /* 0x16A19C dbf D0w */
        wr32(m, dst + i * 4u, rd32(m, src + i * 4u));
}

int Fn_14C464(const f3mem_t *m, f3regs_t *r)
{
    (void)r;                                          /* D0/D1/A0/A1 restored */

    Fn14C464_Copy(m, 0x14C4B6u, 0x40B3AEu, 0x001Fu);   /* 0x14C468..0x14C47E */
    Pal_QueueUpload(m, 0x40B3AEu, 0x0E00u, 0x0019u);  /* 0x14C488..0x14C49A -> 0x001660 */
    return 0;                                         /* 0x14C4A0 lea (0xc,SP),SP ; rts */
}
