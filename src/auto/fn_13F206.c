/* ROM 0x13F206: per-level "A/L" jingle trigger. Indexes the word at
 * A5+0x32DC into the 10-byte-stride level table at 0x13F0EE; unless the entry
 * at +0x06 is negative, plays sound 0x4B through the hook 0x13F238 + 0x39D8.
 * D0/A0 are saved and restored, so no register changes at rts. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_13F206(const f3mem_t *m, f3regs_t *r)
{
    (void)r;

    int32_t d0 = (int16_t)rd16(m, F3_A5 + 0x32DCu) * 10;   /* 0x13F20A muls.w #0xA,D0 */
    uint32_t a0 = 0x0013F0EEu;                             /* 0x13F212 lea (-0x126,PC),A0 */
    int32_t idx = (int16_t)(uint16_t)d0;                   /* EA index is D0.w, sign-extended */

    if ((int16_t)rd16(m, a0 + 6u + (uint32_t)idx) < 0)     /* 0x13F216 tst.w (0x6,A0,D0.w) */
        return 0;                                          /* 0x13F21A bmi.w 0x13F232 */

    /* 0x13F21E move.w #0x4B,-(SP) ; 0x13F222 jsr 0x13F238 ; 0x13F228 jsr 0x39D8 */
    Sound_PlayHooked(m, 0x004Bu);                          /* 0x13F22E lea (0x2,SP),SP */
    return 0;
}
