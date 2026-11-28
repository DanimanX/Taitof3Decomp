/* ROM 0x003F1E: queue sound 0x68C with group id 1 for the slot byte
 * selected by (entry byte 0 - 1) * 8 + entry byte 2 at A5+0x817, but only when
 * that slot byte equals entry byte 5. The record {0x68C, byte0, byte2, 0x01,
 * arg byte at SP+7} goes to the scratch block A5-0x13C2 and through 0x003998
 * into the DPRAM ring, whose index at 0xC00480 becomes the new index * 2. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003F1E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t d0w = rd16(m, sp + 4u);                        /* 0x003F22 (0x18,SP) */
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);                 /* 0x003F26 */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int32_t)(int16_t)d0w); /* 0x003F2A */
    a0 += 1u;                                               /* 0x003F2E adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0);                      /* 0x003F32 move.b (A0),D0b */
    ch = (uint16_t)(ch - 1u);                                /* 0x003F34 subi.w #1,D0 */
    ch = (uint16_t)(ch << 3);                                /* 0x003F3A asl.w #3,D0 */
    ch = (ch & 0xFF00u) | (uint8_t)(ch + rd8(m, a0 + 2u));   /* 0x003F3C add.b (0x2,A0),D0 */
    int16_t off = (int16_t)(uint16_t)ch;                     /* word index, sign-extended */
    uint8_t en = rd8(m, F3_A5 + 0x817u + (uint32_t)(int32_t)off); /* 0x003F40 */
    if (en == 0u)                                            /* 0x003F46 beq */
        return 0;                                            /* 0x003F7A movem */
    if (en != rd8(m, a0 + 5u))                               /* 0x003F48 cmp.b / 0x003F4C bne */
        return 0;                                            /* 0x003F7A movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003F4E..0x003F58 */
    uint32_t scratch = F3_A5 - 0x13C2u;                      /* 0x003F5A */
    wr16(m, scratch, 0x068C);                               /* 0x003F5E */
    wr8(m, scratch + 2u, rd8(m, a0));                        /* 0x003F62 */
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));                   /* 0x003F64 */
    wr8(m, scratch + 4u, 0x01u);                             /* 0x003F68 */
    wr8(m, scratch + 5u, rd8(m, sp + 7u));                   /* 0x003F6C (0x1b,SP) */
    Sound_WriteScratch(m, &d1);                              /* 0x003F70 bsr 0x003998 */
    d1 = (uint16_t)(d1 + d1);                                /* 0x003F74 add.w D1w,D1w */
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);                     /* 0x003F76 */
    return 0;                                                /* 0x003F7E rts */
}
