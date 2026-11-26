/* ROM 0x11F868: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include <stdbool.h>
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

static void LAB_11F7B6(const f3mem_t *m, f3regs_t *r) {
    wr32(m, r->a[0], 0x0011F7BEu);
}

int Fn_11F868(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F868: subq.w #1, (0x6,A0) */
    uint16_t val = rd16(m, r->a[0] + 6);
    int16_t tmp = (int16_t)(val - 1);
    bool gt = tmp > 0;
    bool eq = tmp == 0;
    wr16(m, r->a[0] + 6, (uint16_t)tmp);

    if (gt) {
        /* ROM 0x11F86C: bgt.b 0x11F860 */
        /* ROM 0x11F860: move.l #0x11f868,(A0) */
        wr32(m, r->a[0], 0x0011F868u);
        return 0;
    }

    /* ROM 0x11F86E: bfclr (0x30ee,A5){0x6:0x1} */
    {
        uint16_t v = rd16(m, F3_A5 + 0x30EEu);
        v &= (uint16_t)~0x0200u;
        wr16(m, F3_A5 + 0x30EEu, v);
    }

    /* ROM 0x11F874: move.w #0x28,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x0028u);

    if (eq) {
        /* ROM 0x11F87A: beq.b 0x11F88A */
        /* ROM 0x11F88A: move.w #0x2f,-(SP) */
        r->a[7] -= 2;
        wr16(m, r->a[7], 0x002Fu);
        /* ROM 0x11F88E: jsr 0x13F238 */
        Sound_PlayHooked(m, 0x2Fu);
        /* ROM 0x11F894: jsr 0x3B66 */
        Sound_PlayCmdForced(m, 0);
        /* ROM 0x11F89A: lea (0x2,SP),SP */
        r->a[7] += 2;
        /* ROM 0x11F89E: move.w #0x1e,(0x6,A0) */
        wr16(m, r->a[0] + 6, 0x001Eu);

        if (eq) {
            /* ROM 0x11F8A4: beq.b 0x11F8B4 */
            /* ROM 0x11F8B4: bfset (0x30ee,A5){0x1:0x1} */
            {
                uint16_t v = rd16(m, F3_A5 + 0x30EEu);
                v |= 0x4000u;
                wr16(m, F3_A5 + 0x30EEu, v);
            }
            /* ROM 0x11F8BA: move.w #0xa,(0x6,A0) */
            wr16(m, r->a[0] + 6, 0x000Au);

            if (eq) {
                /* ROM 0x11F8C0: beq.b 0x11F8D0 */
                /* ROM 0x11F8D0: bfclr (0x30ee,A5){0x5:0x1} */
                {
                    uint16_t v = rd16(m, F3_A5 + 0x30EEu);
                    v &= (uint16_t)~0x0400u;
                    wr16(m, F3_A5 + 0x30EEu, v);
                }
                /* ROM 0x11F8D6: bra.w 0x11F7B6 */
                LAB_11F7B6(m, r);
            } else {
                /* ROM 0x11F8C2: move.l #0x11f8ca,(A0) */
                wr32(m, r->a[0], 0x0011F8CAu);
            }
        } else {
            /* ROM 0x11F8A6: move.l #0x11f8ae,(A0) */
            wr32(m, r->a[0], 0x0011F8AEu);
        }
    } else {
        /* ROM 0x11F87C: move.l #0x11f884,(A0) */
        wr32(m, r->a[0], 0x0011F884u);
    }

    return 0;
}
