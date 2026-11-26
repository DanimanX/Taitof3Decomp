/* ROM 0x132F28: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_132F28(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x132F28: subq.w #1, 6(A0) */
    uint16_t tmp = (uint16_t)(rd16(m, r->a[0] + 6u) - 1u);
    wr16(m, r->a[0] + 6u, tmp);
    
    /* ROM 0x132F2C: bgt.b 0x132F20 */
    if ((int16_t)tmp > 0) {
        /* ROM 0x132F20: move.l #0x132F28, (A0) */
        wr32(m, r->a[0], 0x00132F28u);
        /* ROM 0x132F26: rts */
        return 0;
    }
    
    /* ROM 0x132F2E: falls through */
    /* ROM 0x132F2E: sf (0x30F3,A5) */
    wr8(m, F3_A5 + 0x30F3u, 0u);
    
    /* ROM 0x132F32: move.w #0x13, -(SP) */
    r->a[7] -= 2u;
    wr16(m, r->a[7], 0x0013u);
    
    /* ROM 0x132F36: jsr 0x13F238 */
    /* ROM 0x132F3C: jsr 0x39D8 */
    /* ROM 0x132F42: lea (2,SP),SP */
    /* Per pitfall: this sequence is ONE call to Sound_PlayHooked(m, id) */
    Sound_PlayHooked(m, 0x13u);
    r->a[7] += 2u; /* restore SP */
    
    /* ROM 0x132F46: falls through */
    /* ROM 0x132F46: bfset (0x30EE,A5){0xB:0x1} */
    uint16_t w = rd16(m, F3_A5 + 0x30EEu);
    w |= (1u << 11);
    wr16(m, F3_A5 + 0x30EEu, w);
    
    /* ROM 0x132F4C: move.b #0x6, (0x30F2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 6u);
    
    /* ROM 0x132F52: bfset (0x30EE,A5){0xC:0x1} */
    w = rd16(m, F3_A5 + 0x30EEu);
    w |= (1u << 12);
    wr16(m, F3_A5 + 0x30EEu, w);
    
    /* ROM 0x132F58: falls through */
    /* ROM 0x132F58: move.l #0x132F60, (A0) */
    wr32(m, r->a[0], 0x00132F60u);
    
    /* ROM 0x132F5E: rts */
    return 0;
}
