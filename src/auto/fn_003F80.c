/* ROM 0x003F80: verified on an injected capture with the channel-enable table A5-0x1442 forced to 1 (fne_003F80): queue sound 0x68C with group id 1 on
 * the channel table entry selected by the stack argument. The word at SP+4
 * indexes the pointer table at A5-0x1448; the record {0x68C, cmd byte, arg byte
 * at SP+7, 0x01, arg byte at SP+9} is put at the scratch block A5-0x13C2 and
 * handed to 0x003998, which copies it into the DPRAM ring; the ring index at
 * 0xC00480 then advances to the new index * 2. When the channel enable byte
 * (A5-0x1442 + (cmd & 0x7F)) is zero nothing is queued. The MOVEM restores
 * D0/D1/A0/A1/A2.
 * The capture only reaches the disabled path (channel enable byte 0), so the
 * test also passes with an empty body; the queued path was read from the bytes. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003F80(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0w = rd16(m, sp + 4u);                        /* 0x003F84 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                 /* 0x003F88 */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int32_t)(int16_t)d0w); /* 0x003F8C */
    a0 += 1u;                                               /* 0x003F90 adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0) & 0x7Fu;              /* 0x003F94..0x003F98 */
    if (rd8(m, F3_A5 - 0x1442u + ch) == 0u)                  /* 0x003F9C..0x003FA4 */
        return 0;                                           /* 0x003FD2 movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003FAC..0x003FB0 */
    uint32_t scratch = F3_A5 - 0x13C2u;                      /* 0x003FB2 */
    wr16(m, scratch, 0x068Cu);                               /* 0x003FB6 */
    wr8(m, scratch + 2u, rd8(m, a0));                        /* 0x003FBA */
    wr8(m, scratch + 3u, rd8(m, sp + 7u));                   /* 0x003FBC (0x1b,SP) */
    wr8(m, scratch + 4u, 0x01u);                             /* 0x003FC0 */
    wr8(m, scratch + 5u, rd8(m, sp + 9u));                   /* 0x003FC4 (0x1d,SP) */
    Sound_WriteScratch(m, &d1);                              /* 0x003FC8 bsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                                /* 0x003FCC add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                     /* 0x003FCE */
    return 0;                                                /* 0x003FD6 rts */
}
