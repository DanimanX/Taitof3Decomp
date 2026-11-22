#include "obj_11B022.h"
#include "../objects/obj_pool.h"

/* ROM 0x005918: clear playfield RAM (0x611000, 0x3ff longs), clear line RAM
 * (0x62A200, 0xff words) and the shared scroll/focus globals. */
static void F3_ClearGfxRam_5918(const f3mem_t *m)
{
    for (uint32_t a = 0x611000u; a < 0x612000u; a += 4u)
        wr32(m, a, 0);
    for (uint32_t a = 0x62A200u; a < 0x62A400u; a += 2u)
        wr16(m, a, 0);
    wr32(m, 0x402260u, 0); /* -0x5DA0(A5) */
    wr16(m, 0x402264u, 0); /* -0x5D9C(A5) */
    wr32(m, 0x402288u, 0); /* -0x5D78(A5) */
    wr16(m, 0x40228Cu, 0); /* -0x5D74(A5) */
}

/* ROM 0x0011B01A: re-arm the tick vector with oneself. */
static void Obj_11B022_SetVector(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0011B022u);
}

int Obj_11B022_Tick(const f3mem_t *m, uint32_t obj)
{
    /* A1 = the object linked at 0x8(A0). */
    uint32_t a1 = rd32(m, obj + 0x8u);

    if (rd8(m, a1 + 0x59u) == 0) {
        /* Screen-space offset against the scroll origin. */
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x2Cu) - rd16(m, obj + 0x14u));
        wr16(m, 0x402260u, d0);                   /* -0x5DA0(A5) */
        wr16(m, obj + 0x14u, rd16(m, 0x40B0D2u)); /* 0x30D2(A5) */
        d0 = (uint16_t)(rd16(m, obj + 0x30u) - rd16(m, 0x40B0D6u));
        wr16(m, 0x402264u, d0);                 /* -0x5D9C(A5) */
        wr16(m, 0x40C20Cu, rd16(m, 0x402260u)); /* 0x420C(A5) */
        wr16(m, 0x40C210u, rd16(m, 0x40C210u)); /* no-op copy of 0x4210(A5) */
        Obj_11B022_SetVector(m, obj);
        return 0;
    }

    F3_ClearGfxRam_5918(m);
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
