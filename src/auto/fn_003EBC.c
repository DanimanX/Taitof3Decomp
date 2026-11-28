/* ROM 0x003EBC: queue sound 0x690 with the slot, sub-slot and value bytes
 * for the channel-slot byte selected by (entry byte 0 - 1) * 8 + entry byte 2 at
 * A5+0x817, but only when that slot byte equals entry byte 5. The record {0x690,
 * byte0, byte2, byte5, arg byte at SP+7} goes to the scratch block A5-0x13C2
 * and through 0x003998 into the DPRAM ring, whose index at 0xC00480 becomes the
 * new index * 2. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003EBC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0w = rd16(m, sp + 4u);                        /* 0x003EC0 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                 /* 0x003EC4 */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int32_t)(int16_t)d0w); /* 0x003EC8 */
    a0 += 1u;                                               /* 0x003ECC adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0);                      /* 0x003ED0 move.b (A0),D0b */
    ch = (uint16_t)(ch - 1u);                                /* 0x003ED2 subi.w #1,D0 */
    ch = (uint16_t)(ch << 3);                                /* 0x003ED8 asl.w #3,D0 */
    ch = (ch & 0xFF00u) | (uint8_t)(ch + rd8(m, a0 + 2u));   /* 0x003EDA add.b (0x2,A0),D0 */
    int16_t off = (int16_t)(uint16_t)ch;                     /* word index, sign-extended */
    uint8_t en = rd8(m, F3_A5 + 0x817u + (uint32_t)(int32_t)off); /* 0x003EDE */
    if (en == 0u)                                            /* 0x003EE4 beq */
        return 0;                                            /* 0x003F18 movem */
    if (en != rd8(m, a0 + 5u))                               /* 0x003EE6 cmp.b / 0x003EEA bne */
        return 0;                                            /* 0x003F18 movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003EEC..0x003EF6 */
    uint32_t scratch = F3_A5 - 0x13C2u;                      /* 0x003EF8 */
    wr16(m, scratch, 0x0690);                               /* 0x003EFC */
    wr8(m, scratch + 2u, rd8(m, a0));                        /* 0x003F00 */
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));                   /* 0x003F02 */
    wr8(m, scratch + 4u, rd8(m, a0 + 5u));                   /* 0x003F06 */
    wr8(m, scratch + 5u, rd8(m, sp + 7u));                   /* 0x003F0A (0x1b,SP) */
    Sound_WriteScratch(m, &d1);                              /* 0x003F0E bsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                                /* 0x003F12 add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                     /* 0x003F14 */
    return 0;                                                /* 0x003F1C rts */
}
