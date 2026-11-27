/* ROM 0x15BAD8: spawn the three exploding cores of the boss death (A0 =
 * object). Plays sound 0x16, puts the object at (0x12C, 0x80), then spawns
 * 0x13E4EE three times into group 5 (2 groups) with the packed args
 * 0x10C00005, 0x10400005 and 0x10000005, and arms 0x15BB78. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_E8B72.h"

int Fn_15BAD8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1;

    Sound_PlayHooked(m, 0x0016u);                    /* 0x15BAD8 movem/0x15BADC..0x15BAEC */
    wr16(m, obj + 0x14u, 0x012Cu);                   /* 0x15BAF4 */
    wr16(m, obj + 0x18u, 0x0080u);                   /* 0x15BAFA */

    d1 = 0x00000005u;                                /* 0x15BB00 moveq #5,D1 */
    d0 = (d0 & 0xFFFF0000u) | 0x0010u;               /* 0x15BB02 move.w #0x10 */
    d1 = (d1 & 0x00FFFFFFu) | ((d0 & 0xFFu) << 24);  /* 0x15BB06 bfins D0,D1{0:8} */
    d0 = (d0 & 0xFFFF0000u) | 0xFFC0u;               /* 0x15BB0A move.w #-0x40 */
    d1 = (d1 & 0xFF00FFFFu) | ((d0 & 0xFFu) << 16);  /* 0x15BB0E bfins D0,D1{8:8} */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d1); /* 0x15BB12..0x15BB22 */

    d0 = (d0 & 0xFFFF0000u) | 0x0040u;               /* 0x15BB2C move.w #0x40 */
    d1 = (d1 & 0xFF00FFFFu) | ((d0 & 0xFFu) << 16);  /* 0x15BB30 */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d1); /* 0x15BB34..0x15BB44 */

    d0 = (d0 & 0xFFFF0000u) | 0x0000u;               /* 0x15BB4E move.w #0 */
    d1 = (d1 & 0xFF00FFFFu) | ((d0 & 0xFFu) << 16);  /* 0x15BB52 */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d1); /* 0x15BB56..0x15BB66 */

    wr32(m, obj + 0x00u, 0x0015BB78u);               /* 0x15BB70 */

    r->d[0] = d0;
    r->d[1] = d1;
    return 0;                                        /* 0x15BB76 rts */
}
