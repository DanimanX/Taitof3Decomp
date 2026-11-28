#include "fn_auto.h"
/* ROM 0x12A13A: boss camera tick; on expiry retarget (0x54=3, 0x50=0xC8, 0x52=0x6E),
 * play hooked sound 0x3A, set A5+0x30F2 = 2 and arm 0x12A18E for 100 frames. */
#include "fn_sound_frames.h"
#include "round8_camera_regs.h"

int Fn_12A13A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x12A13Au)) {
        wr32(m, obj, 0x12A13Au);          /* 0x12A132 re-arm thunk */
    } else {
        wr8(m, obj + 0x54u, 3u);          /* 0x12A152 */
        wr16(m, obj + 0x50u, 0x00C8u);    /* 0x12A158 */
        wr16(m, obj + 0x52u, 0x006Eu);    /* 0x12A15E */
        /* 0x12A164..0x12A174: move.w #0x3A,-(SP) ; jsr 0x13F238 ; jsr 0x39D8 ; lea (2,SP). */
        call_hooked_sound(m, r, 0x3Au, 0x12A16Eu, 0x12A174u);
        wr8(m, r->a[5] + 0x30F2u, 2u);    /* 0x12A178 */
        wr16(m, obj + 6u, 0x0064u);       /* 0x12A17E */
        wr32(m, obj, 0x12A18Eu);          /* 0x12A186 (BEQ 0x12A1A6 never taken) */
    }
    r->a[7] += 4u;                        /* rts */
    return 0;
}
