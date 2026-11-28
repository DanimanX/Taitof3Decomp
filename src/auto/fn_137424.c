/* ROM 0x137424: boss script hand-off (A0 = the boss object).
 * Points the object A5+0x30DE holds at 0x11B34E, spawns one 0x13E9E8 helper
 * in group 3 (2 groups scanned) with argument 0x40 through 0x01057C, then
 * copies +0x4F to +0x1D, arms a 0xC8-frame timer at +0x06 and re-arms itself
 * at 0x137460. 0x01057C restores D1-D4/A0-A2, so A0 survives and D0 carries
 * the new object, or -1 (0x0105C0 moveq #$ff sign-extends) when the pool is
 * full. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_137424(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t a1 = rd32(m, F3_A5 + 0x30DEu);                   /* 0x137424 */
    wr32(m, a1, 0x0011B34Eu);                                 /* 0x137428 */

    /* 0x13742E..0x137442: push 0x40, count 2, group 3, fn 0x13E9E8. */
    uint32_t spawned = Obj_Spawn(m, obj, 0x0013E9E8u, 3u, 2u, 0x0040u);
    r->d[0] = spawned;   /* 0x0105E4 move.l a2,d0; 0x0105C0 moveq #$ff = -1 */
    r->a[1] = a1;

    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));                 /* 0x13744C */
    wr16(m, obj + 0x06u, 0x00C8u);                            /* 0x137452 */
    wr32(m, obj, 0x00137460u);                                /* 0x137458 */
    return 0;
}
