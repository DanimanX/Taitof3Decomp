#include "fn_auto.h"
/* ROM 0x129DC0: boss camera state (A0 = object, round-8 boss script). Runs the
 * three camera helpers 0x11B288 / 0x11B2CA / 0x11B262 (steer toward the
 * target, ease the speed word and move the camera by +0x42), then decrements
 * +0x06: while it stays > 0 the state re-arms 0x129DC0; on expiry it plays
 * hooked sound 0x3A when A5+0x30EF bit 6 is set, sets A5+0x30EF bit 5,
 * +0x06 = 1 and moves to 0x129E0C. */
#include "fn_boss_camera.h"
#include "../enemy/obj_E8B72.h"   /* Sound_PlayHooked (0x13F238 + 0x0039D8) */

int Fn_129DC0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    boss_camera_step(m, r, 0x129DC0u);       /* 0x129DC0/0x129DC6/0x129DCC jsr */

    uint16_t timer = rd16(m, obj + 0x06u);   /* 0x129DD2 subq.w #1,(6,A0) */
    wr16(m, obj + 0x06u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {                /* 0x129DD6 bgt.b 0x129DB8 */
        wr32(m, obj, 0x00129DC0u);           /* 0x129DB8 */
        return 0;                            /* 0x129DBE rts */
    }

    /* 0x129DD8 bftst (0x30EE,A5){0xE:1} -> bit 6 of the byte at A5+0x30EF */
    if ((rd8(m, F3_A5 + 0x30EFu) & 0x02u) != 0u)
        Sound_PlayHooked(m, 0x003Au);        /* 0x129DE2..0x129DF2 */

    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x04u)); /* 0x129DF6 {0xD:1} */
    wr16(m, obj + 0x06u, 0x0001u);           /* 0x129DFC (beq 0x129E24 never taken) */
    wr32(m, obj, 0x00129E0Cu);               /* 0x129E04 */
    return 0;                                /* 0x129E0A rts */
}
