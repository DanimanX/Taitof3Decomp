/* ROM 0x004030: verified on an injected capture with the channel-enable table A5-0x1442 forced to 1 (fne_004030): queue sound 0x68C with group id 7 on
 * the channel table entry selected by the stack argument (same body as 0x003F80,
 * only the constant byte differs): record {0x68C, cmd byte, arg byte at SP+7,
 * 0x07, arg byte at SP+9} into the scratch block A5-0x13C2 for 0x003998, then
 * the DPRAM ring index at 0xC00480 = (index after the copy) * 2. Nothing is
 * queued when the A5-0x1442 channel enable byte is zero.
 * The capture only reaches the disabled path (channel enable byte 0), so the
 * test also passes with an empty body; the queued path was read from the bytes. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_004030(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0w = rd16(m, sp + 4u);                        /* 0x004034 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                 /* 0x004038 */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int32_t)(int16_t)d0w); /* 0x00403C */
    a0 += 1u;                                               /* 0x004040 adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0) & 0x7Fu;              /* 0x004044..0x004046 */
    if (rd8(m, F3_A5 - 0x1442u + ch) == 0u)                  /* 0x00404C..0x004054 */
        return 0;                                           /* 0x004082 movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x00405C..0x004060 */
    uint32_t scratch = F3_A5 - 0x13C2u;                      /* 0x004062 */
    wr16(m, scratch, 0x068C);                               /* 0x004066 */
    wr8(m, scratch + 2u, rd8(m, a0));                        /* 0x00406A */
    wr8(m, scratch + 3u, rd8(m, sp + 7u));                   /* 0x00406C (0x1b,SP) */
    wr8(m, scratch + 4u, 0x07);                         /* 0x004070 */
    wr8(m, scratch + 5u, rd8(m, sp + 9u));                   /* 0x004074 (0x1d,SP) */
    Sound_WriteScratch(m, &d1);                              /* 0x004078 bsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                                /* 0x00407C add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                     /* 0x00407E */
    return 0;                                                /* 0x004086 rts */
}
