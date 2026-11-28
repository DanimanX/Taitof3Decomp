/* ROM 0x003E5E: queue sound 0x58B for the channel-slot byte selected by
 * (entry byte 0 - 1) * 8 + entry byte 2 at A5+0x817, but only when that slot byte
 * equals entry byte 5 (same body as 0x003E00, id 0x58B). */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003E5E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0w = rd16(m, sp + 4u);                        /* 0x003E62 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                 /* 0x003E66 */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int32_t)(int16_t)d0w); /* 0x003E6A */
    a0 += 1u;                                               /* 0x003E6E adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0);                      /* 0x003E72 move.b (A0),D0b */
    ch = (uint16_t)(ch - 1u);                                /* 0x003E74 subi.w #1,D0 */
    ch = (uint16_t)(ch << 3);                                /* 0x003E7A asl.w #3,D0 */
    ch = (ch & 0xFF00u) | (uint8_t)(ch + rd8(m, a0 + 2u));   /* 0x003E7C add.b (0x2,A0),D0 */
    int16_t off = (int16_t)(uint16_t)ch;                     /* word index, sign-extended */
    uint8_t en = rd8(m, F3_A5 + 0x817u + (uint32_t)(int32_t)off); /* 0x003E80 */
    if (en == 0u)                                            /* 0x003E86 beq */
        return 0;                                            /* 0x003EB6 movem */
    if (en != rd8(m, a0 + 5u))                               /* 0x003E88 cmp.b / 0x003E8C bne */
        return 0;                                            /* 0x003EB6 movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003E8E..0x003E98 */
    uint32_t scratch = F3_A5 - 0x13C2u;                      /* 0x003E9A */
    wr16(m, scratch, 0x058B);                               /* 0x003E9E */
    wr8(m, scratch + 2u, rd8(m, a0));                        /* 0x003EA2 */
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));                   /* 0x003EA4 */
    wr8(m, scratch + 4u, 0x01u);                              /* 0x003E9E */
    wr8(m, scratch + 4u, rd8(m, sp + 7u));                   /* 0x003EA8 (0x1b,SP) */
    Sound_WriteScratch(m, &d1);                              /* 0x003EAC bsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                                /* 0x003EB0 add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                     /* 0x003EB2 */
    return 0;                                                /* 0x003EBA rts */
}
