#include "obj_12601C.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x11C35E: blink between palette 0x10 and obj+0x4F while obj+0x35 is set;
 * the first toggle also plays sound command 0x1C. */
static void sub_11C35E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        goto restore_11c392;                           /* 0x11C362 */
    if (rd8(m, obj + 0x1Du) == 0x10u)
        goto restore_11c392;                           /* 0x11C36C */

    wr8(m, obj + 0x35u, 0u);                           /* sf (0x35,A0) */
    wr8(m, obj + 0x1Du, 0x10u);                        /* 0x11C374 */
    /* 0x11C37A: move.w #0x1C,-(SP) ; jsr 0x13F238 ; jsr 0x39D8 */
    Sound_PlayHooked(m, 0x001Cu);
    return;

restore_11c392:
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));          /* 0x11C392 */
}

/* ROM 0x12601C: a short-lived cloud. While the level timer (A5+0x3318) is at
 * most 0x3840 and the spawn budget (long at A5+0x30DA) is positive it blinks
 * and re-arms itself; otherwise it raises the wave flag A5-0x144C bit 4, sets
 * the global A5+0x30F3, clears its palette fields and hops to 0x12604C. */
int Obj_12601C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12601C: cmpi.w #0x3840,(0x3318,A5) ; bgt 0x126032 */
    if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x3840) {
        sub_11C35E(m, obj);                            /* 0x126026 */
        /* 0x12602C: tst.l (0x30DA,A5) ; bgt 0x126014 (re-arm) */
        if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)
            goto stop_126032;
        wr32(m, obj + 0x00u, 0x0012601Cu);             /* 0x126014 */
        return 0;
    }

stop_126032:
    /* 0x126032: bset #4 on A5-0x144C ; st A5+0x30F3 ; sf +0x1D ; sf +0x4F */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x10u));
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);
    wr8(m, obj + 0x1Du, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    wr32(m, obj + 0x00u, 0x0012604Cu);                 /* 0x126044 */
    return 0;
}
