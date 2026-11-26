/* ROM 0x12478C: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_12478C(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x12478C: addi.l #-0xc000,(0x30d2,A5) */
    wr32(m, F3_A5 + 0x30d2u, rd32(m, F3_A5 + 0x30d2u) - 0xc000u);
    /* ROM 0x124794: addi.l #0x0,(0x30d6,A5) */
    wr32(m, F3_A5 + 0x30d6u, rd32(m, F3_A5 + 0x30d6u) + 0u);
    /* ROM 0x12479C: cmpi.w #0xe0,(0x30d2,A5) */
    /* ROM 0x1247A2: bgt.b 0x00124784 */
    if ((int16_t)(rd16(m, F3_A5 + 0x30d2u)) > 0xE0) {
        /* LAB_00124784 */
        wr32(m, r->a[0], 0x0012478Cu);
        return 0;
    }
    /* ROM 0x1247A4: bfset (0x30ee,A5){0xf:0x1} */
    wr8(m, F3_A5 + 0x30EEu, rd8(m, F3_A5 + 0x30EEu) | 0xFEu);
    /* ROM 0x1247AA: move.w #0x50,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x50u);
    /* ROM 0x1247B0: beq.b 0x001247c0 (never taken after move.w) */
    if (0) {
        /* LAB_001247c0 */
        /* ROM 0x1247C0: sf (0x30f3,A5) */
        wr8(m, F3_A5 + 0x30f3u, 0u);
        /* ROM 0x1247C4: move.w #0x13,-(SP) */
        r->a[7] -= 2;
        wr16(m, r->a[7], 0x0013u);
        /* ROM 0x1247C8: jsr 0x0013f238.l */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x001247CEu);
        Sound_PlayHooked(m, 0x13u);
        r->a[7] += 4;
        /* ROM 0x1247CE: jsr 0x000039d8.l */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x001247D4u);
        Sound_PlayCmd(m, 0x13);
        r->a[7] += 4;
        /* ROM 0x1247D4: lea (0x2,SP),SP */
        r->a[7] += 2;
        /* ROM 0x1247D8: bset.b #0x0,(0x30f2,A5) */
        wr8(m, F3_A5 + 0x30f2u, rd8(m, F3_A5 + 0x30f2u) | 0x01u);
        /* ROM 0x1247DE: move.w #-0x6000,(0x22,A0) */
        wr16(m, r->a[0] + 0x22u, 0xA000u);
        /* ROM 0x1247E4: move.w #-0x6000,(0x20,A0) */
        wr16(m, r->a[0] + 0x20u, 0xA000u);
        /* ROM 0x1247EA: move.w #0x6,(0x42,A0) */
        wr16(m, r->a[0] + 0x42u, 0x0006u);
        /* ROM 0x1247F0: clr.w (0x3e,A0) */
        wr16(m, r->a[0] + 0x3eu, 0u);
        /* ROM 0x1247F4: clr.w (0x3a,A0) */
        wr16(m, r->a[0] + 0x3au, 0u);
        /* falls through to LAB_001247f8 */
    }
    /* LAB_001247f8 */
    /* ROM 0x1247F8: move.l #0x124800,(A0) */
    wr32(m, r->a[0], 0x00124800u);
    /* ROM 0x1247FE: rts */
    return 0;
}
