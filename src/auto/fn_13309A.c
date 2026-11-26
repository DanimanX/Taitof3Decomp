/* ROM 0x13309A: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_13309A(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13309A: subq.w #1,(0x6,A0) ; bgt.b 0x133092 */
    uint16_t cnt = rd16(m, r->a[0] + 6u);
    cnt--;
    wr16(m, r->a[0] + 6u, cnt);
    if ((int16_t)cnt > 0) {
        /* ROM 0x133092: move.l #0x13309a,(A0) ; rts */
        wr32(m, r->a[0], 0x0013309Au);
        return 0;
    }

    /* ROM 0x1330A0: bfclr (0x30ee,A5){0x2:0x1} ... {0x5:0x1} */
    {
        uint16_t v = rd16(m, F3_A5 + 0x30EEu);
        v &= (uint16_t)~0x3C00u;
        wr16(m, F3_A5 + 0x30EEu, v);
    }

    /* ROM 0x1330B8: cmpi.w #0x320,(0x30da,A5) ; ble.w 0x133198 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x320) {
        /* ROM 0x133198: move.w #0x13,-(SP) ; jsr 0x13F238 ; jsr 0x39D8 ; lea (2,SP),SP */
        Sound_PlayHooked(m, 0x13u);

        /* ROM 0x1331AC: bfset (0x30ee,A5){0xb:0x1} ; bfset (0x30ee,A5){0xc:0x1} */
        {
            uint16_t v = rd16(m, F3_A5 + 0x30EEu);
            v |= (uint16_t)0x0018u;
            wr16(m, F3_A5 + 0x30EEu, v);
        }

        /* ROM 0x1331B2: move.b #0x7,(0x30f2,A5) */
        wr8(m, F3_A5 + 0x30F2u, 7u);

        /* ROM 0x1331BE: move.l #0x1331c6,(A0) ; rts */
        wr32(m, r->a[0], 0x001331C6u);
        return 0;
    }

    /* ROM 0x1330C2: move.w #0x3c,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x003Cu);

    /* ROM 0x1330C8: move.l #0x1330d0,(A0) ; rts */
    wr32(m, r->a[0], 0x001330D0u);
    return 0;
}
