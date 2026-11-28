/* ROM 0x003D54: UNVERIFIED (weak capture): queue sound 0x486 on the channel
 * table entry selected by the stack argument. The word at SP+4 indexes the
 * pointer table at A5-0x1448; the record {0x486, cmd byte, arg byte at SP+7} is
 * put at the scratch block A5-0x13C2 and handed to 0x003998, then the DPRAM ring
 * index at 0xC00480 advances to the new index * 2. If the argument byte at SP+9
 * has its sign bit set, byte 1 of the table entry is copied over it first. When
 * the channel enable byte (A5-0x1442 + (cmd & 0x7F)) is zero nothing is queued.
 * The MOVEM restores D0/D1/A0/A1/A2.
 * The capture only reaches the disabled path (channel enable byte 0), so the
 * test also passes with an empty body; the queued path was read from the bytes. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003D54(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0w = rd16(m, sp + 4u);                        /* 0x003D58 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                 /* 0x003D5C */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int32_t)(int16_t)d0w); /* 0x003D60 */
    a0 += 1u;                                               /* 0x003D64 adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0) & 0x7Fu;              /* 0x003D68..0x003D6C */
    if (rd8(m, F3_A5 - 0x1442u + ch) == 0u)                  /* 0x003D70..0x003D78 */
        return 0;                                           /* 0x003DAA movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003D7A..0x003D84 */
    if ((int8_t)rd8(m, sp + 9u) < 0)                         /* 0x003D86 tst.b / 0x003D8A bpl */
        wr8(m, sp + 9u, rd8(m, a0 + 1u));                    /* 0x003D8C */

    uint32_t scratch = F3_A5 - 0x13C2u;                      /* 0x003D92 */
    wr16(m, scratch, 0x0486u);                               /* 0x003D96 */
    wr8(m, scratch + 2u, rd8(m, a0));                        /* 0x003D9A */
    wr8(m, scratch + 3u, rd8(m, sp + 7u));                   /* 0x003D9C (0x1b,SP) */
    Sound_WriteScratch(m, &d1);                              /* 0x003DA0 bsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                                /* 0x003DA4 add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                     /* 0x003DA6 */
    return 0;                                                /* 0x003DAE rts */
}
