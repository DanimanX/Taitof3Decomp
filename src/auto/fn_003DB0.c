/* ROM 0x003DB0: UNVERIFIED (weak capture): queue one sound command from the
 * id table when the channel is enabled.
 * arg.w (SP+4) indexes the pointer table whose base is the long at A5-0x1448;
 * that entry + 1 is the command byte, whose low 7 bits index the enable byte
 * table at A5-0x1442. When that byte is non-zero, the four-byte record
 * {0x0487, cmd byte, arg.b from SP+7} is put at the scratch block A5-0x13C2
 * and handed to 0x003998, then the DPRAM ring index at 0xC00480 is advanced to
 * the new index * 2. All five registers it touches are saved and restored.
 * The capture only ever reaches the disabled (table byte 0) path, so it
 * passes with an empty body too; both paths are read from the bytes. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003DB0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0 = rd16(m, sp + 4u);                        /* 0x003DB4 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                /* 0x003DB8 */
    a0 = rd32(m, a0 + (uint32_t)(int32_t)(int16_t)d0 * 4u); /* 0x003DBC */
    a0 += 1u;                                              /* 0x003DC0 adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0) & 0x7Fu;            /* 0x003DC4..0x003DC8 */
    if (rd8(m, F3_A5 - 0x1442u + ch) == 0u)                /* 0x003DCC..0x003DD4 */
        return 0;                                          /* 0x003DFA movem restore */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003DD6..0x003DE0 */
    uint32_t scratch = F3_A5 - 0x13C2u;                    /* 0x003DE2 */
    wr16(m, scratch, 0x0487u);                             /* 0x003DE6 */
    wr8(m, scratch + 2u, rd8(m, a0));                      /* 0x003DEA */
    wr8(m, scratch + 3u, rd8(m, sp + 7u));                 /* 0x003DEC (0x1b,SP) */
    Sound_WriteScratch(m, &d1);                            /* 0x003DF0 jsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                              /* 0x003DF4 add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                   /* 0x003DF6 */
    return 0;                                              /* 0x003DFA movem restore */
}
