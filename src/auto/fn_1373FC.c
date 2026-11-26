/* ROM 0x1373FC: boss wait-until-scroll/timer gate tick (A0 = object).
 * Syncs +0x1D via 0x11C35E; while (s16)(A5+0x3318) <= 0x3840 and (s32)(A5+0x30DA) > 0,
 * re-arms 0x1373FC. Otherwise sets bit 4 of A5-0x144C, sets A5+0x30F3 = 0xFF, and arms 0x137424. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x11C35E: clear +0x35 and set +0x1D=0x10 with SFX 0x1C once, else copy +0x4F to +0x1D. */
static void sub_11C35E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u && rd8(m, obj + 0x1Du) != 0x10u) { /* 0x11C35E..0x11C36C */
        wr8(m, obj + 0x35u, 0u);                                     /* 0x11C370 */
        wr8(m, obj + 0x1Du, 0x10u);                                  /* 0x11C374 */
        Sound_PlayHooked(m, 0x001Cu);                                /* 0x11C37A..0x11C38A */
        return;                                                      /* 0x11C38E */
    }
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));                        /* 0x11C392 */
}

int Fn_1373FC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11C35E(m, obj);                                              /* 0x1373FC */
    if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x3840                  /* 0x137402..0x137408 */
        && (int32_t)rd32(m, F3_A5 + 0x30DAu) > 0) {                  /* 0x13740C..0x137410 */
        wr32(m, obj, 0x001373FCu);                                   /* 0x1373F4 */
        return 0;
    }

    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x10u)); /* 0x137412 */
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);                                  /* 0x137418 */
    wr32(m, obj, 0x00137424u);                                       /* 0x13741C */
    return 0;
}
