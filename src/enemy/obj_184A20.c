/* ROM 0x184A20: one-shot fuse. Counts obj+6 down and, while it is still
 * positive, restarts the same tick from the entry's top (branch back to
 * 0x184A14, so the count falls only by one per call). At zero it:
 *   0x184A26  resets the boss globals (0x1847E8) and uploads the boss palettes
 *   0x184A32  fills the boss status globals at A5+0x4B28..
 *   0x184A68  installs the next behaviour 0x184FEA in +0x64
 *   0x184A7C  clears the motion fields and scales the HP (+0x24) via 0x18495E
 *   0x184AD4  queues five palette uploads (0x1660)
 *   0x184B60  spawns the part 0x186782 (group 3 of 2, arg 0x186B6A) and keeps
 *             the child in A5+0x4B66; 0x184B82 spawns 0x186326 (arg 0)
 *   0x184B9E  arms the next tick (obj+6 = 2, fn = 0x184BAE) */
#include "obj_184A20.h"
#include "obj_18765E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h" /* Obj_18495E (ROM 0x18495E) */
#include "../video/palette.h"

/* ROM 0x184A20 */
int Obj_184A20_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x184A20: subq.w #1,(0x6,A0) ; bgt.b 0x184A14 */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, d);
    if ((int16_t)d > 0)
        return 0;                       /* 0x184A14: re-enter at the top */

    /* 0x184A26 */
    Obj_1847E8(m, obj);
    Obj_17B3AE(m);

    /* 0x184A32 */
    wr8(m, F3_A5 + 0x4B28u, 0x94u);
    wr16(m, F3_A5 + 0x4B2Au, 0x0019u);
    wr16(m, F3_A5 + 0x4B2Cu, 0x0032u);
    wr16(m, F3_A5 + 0x4B30u, 0x4020u);
    wr16(m, F3_A5 + 0x4B32u, 0x0018u);
    wr16(m, F3_A5 + 0x4B70u, 0x00E0u);
    wr32(m, F3_A5 + 0x4B6Au, 0x00031C70u);
    wr16(m, F3_A5 + 0x4B46u, 0u);
    /* 0x184A62: bfset (0x4B26,A5){8:1} = bit 7 of the byte at A5+0x4B27 */
    wr8(m, F3_A5 + 0x4B27u, (uint8_t)(rd8(m, F3_A5 + 0x4B27u) | 0x80u));
    wr32(m, obj + 0x64u, 0x00184FEAu);
    wr16(m, F3_A5 + 0x4B38u, 1u);
    /* 0x184A76: bfclr (0x4B26,A5){4:1} = bit 3 of the byte at A5+0x4B26 */
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
    wr16(m, obj + 0x50u, 0xFF9Cu);
    wr16(m, obj + 0x54u, 0xFF9Cu);
    wr32(m, obj + 0x58u, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    wr16(m, obj + 0x62u, 0xFFFFu);

    /* 0x184A9A: HP long 0x12C0000, then +0x24 (high word) scaled */
    wr32(m, obj + 0x24u, 0x012C0000u);
    Obj_18495E(m, obj);

    /* 0x184AA8: three bars and the divisor share the HP long */
    uint32_t d0 = rd32(m, obj + 0x24u);
    wr32(m, F3_A5 + 0x4B0Cu, d0);
    wr32(m, F3_A5 + 0x4B10u, d0);
    wr32(m, F3_A5 + 0x4B14u, d0);
    d0 /= 10u;                          /* 0x184AB8: divu.l #10 */
    wr32(m, F3_A5 + 0x4B18u, d0);
    d0 = rd32(m, F3_A5 + 0x4B18u) * 5u; /* 0x184AC4: mulu.l #5 */
    wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - d0);

    /* 0x184AD4: jsr 0x1660 with src, dst, count pushed in that order */
    Pal_QueueUpload(m, 0x00019500u, 0x6400u, 0x20u);
    Pal_QueueUpload(m, 0x00019400u, 0x6300u, 0x20u);
    Pal_QueueUpload(m, 0x0001E4C0u, 0x6480u, 0x10u);
    Pal_QueueUpload(m, 0x00016900u, 0x6500u, 0x20u);
    Pal_QueueUpload(m, 0x00019500u, 0x6680u, 0x20u);

    /* 0x184B60: move.l #arg ; move.w #2 ; move.w #3 ; move.l #fn ; jsr 0x1057C
     * -> Obj_Spawn(m, obj, fn, group 3, 2); D0 = child stored in A5+0x4B66 */
    wr32(m, F3_A5 + 0x4B66u, Obj_Spawn(m, obj, 0x00186782u, 3u, 2u, 0x00186B6Au));
    /* 0x184B82: pea 0 ; move.w #2 ; move.w #3 ; move.l #0x186326 ; jsr 0x1057C
     * -- second part, child not kept */
    (void)Obj_Spawn(m, obj, 0x00186326u, 3u, 2u, 0u);

    /* 0x184B9E: move.w #2,(0x6,A0) ; beq never taken ; move.l #0x184BAE,(A0) */
    wr16(m, obj + 0x06u, 2u);
    wr32(m, obj + 0x00u, 0x00184BAEu);
    return 0;
}
