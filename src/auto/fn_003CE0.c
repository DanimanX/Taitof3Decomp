/* ROM 0x003CE0: queue the "channel volume" command 0x0789 for the sound
 * channel that the ROM descriptor selected by the stack argument names.
 *   arg1.w (SP+4)   command id: index into the pointer table at A5-0x1448
 *   arg2.b (SP+7)   byte 5 of the queued record (the caller's volume byte)
 *   arg3.b (SP+9)   byte 3 of the record; 0x80 = "use descriptor byte +1"
 *   arg4.b (SP+11)  byte 4 of the record; same 0x80 convention
 *   003CE0 movem.l D0/D1/A0-A2,-(SP) ; 003CE4 move.w 0x18(SP),D0
 *   003CE8/0x003CEC A0 = ((*)(A5-0x1448))[D0w] ; 003CF0 adda.w #1,A0
 *   003CF4..0x003CF8 D0w = (*A0) & 0x7F
 *   003D00 tst.b (A5-0x1442,D0w) : zero -> nothing queued, straight to the
 *   MOVEM at 0x003D4E.
 *   003D0C D1w = 0xC00480 >> 1 (the DPRAM ring index); 0x003D12..0x003D28
 *   replace arg3/arg4 in the *caller's* frame with descriptor byte +1 when
 *   their sign bit is set; 0x003D2E..0x003D40 builds {0x0789, *A0, arg3,
 *   arg4, arg2, 0} at the scratch block A5-0x13C2; 0x003D44 bsr 0x003998
 *   copies the counted string into the DPRAM ring; 0x003D48/0x003D4A store
 *   index*2 back to 0xC00480.
 * The MOVEM restores D0/D1/A0-A2, so no register but PC/SP changes and the C
 * model leaves all of them alone (the capture's post SP is the caller's own
 * cleanup of the four argument words, +0xC, which is not this routine's doing).
 * Verified against work/ram/fnk_003CE0 (F3_DPRAM=1): the two argument bytes
 * sit at 0x4052A5/0x4052A7, above the entry SP, so test_fn compares them; the
 * run poked both to 0x80 so the descriptor-byte path fires on every pair.
 * src/enemy/obj_13EFA0.c keeps its own static copy of this body for the music
 * sequencer's in-game call, where the argument bytes are C locals. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003CE0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t cmd = rd16(m, sp + 4u);                          /* 0x003CE4 (0x18,SP) */
    uint32_t a0 = rd32(m, rd32(m, F3_A5 - 0x1448u)              /* 0x003CE8 */
                          + 4u * (uint32_t)(int32_t)(int16_t)cmd) /* 0x003CEC */
                + 1u;                                          /* 0x003CF0 adda.w #1 */
    uint32_t ch = (uint32_t)rd8(m, a0) & 0x7Fu;                /* 0x003CF4..0x003CF8 */
    if (rd8(m, F3_A5 - 0x1442u + ch) == 0u)                    /* 0x003D00 tst.b */
        return 0;                                              /* 0x003D4E movem */

    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1); /* 0x003D0C..0x003D10 */
    if ((int8_t)rd8(m, sp + 9u) < 0)                           /* 0x003D12 tst.b (0x1d,SP) */
        wr8(m, sp + 9u, rd8(m, a0 + 1u));                      /* 0x003D18 */
    if ((int8_t)rd8(m, sp + 11u) < 0)                          /* 0x003D1E tst.b (0x1f,SP) */
        wr8(m, sp + 11u, rd8(m, a0 + 1u));                     /* 0x003D24 */

    uint32_t scratch = F3_A5 - 0x13C2u;                        /* 0x003D2A */
    wr16(m, scratch, 0x0789u);                                 /* 0x003D2E */
    wr8(m, scratch + 2u, rd8(m, a0));                          /* 0x003D32 */
    wr8(m, scratch + 3u, rd8(m, sp + 9u));                     /* 0x003D34 */
    wr8(m, scratch + 4u, rd8(m, sp + 11u));                    /* 0x003D38 */
    wr8(m, scratch + 5u, rd8(m, sp + 7u));                     /* 0x003D3C (0x1b,SP) */
    wr8(m, scratch + 6u, 0u);                                  /* 0x003D40 */
    Sound_WriteScratch(m, &d1);                                /* 0x003D44 bsr 0x003998 */
    wr16(m, F3_DPRAM_BASE + 0x480u, (uint16_t)(d1 + d1));      /* 0x003D48/0x003D4A */
    return 0;                                                  /* 0x003D52 rts */
}
