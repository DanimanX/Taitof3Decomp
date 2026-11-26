/* ROM 0x1812FE: radial-placed projectile/enemy init (A0 = object).
 * Places obj polar-relative to parent (+0x08) via 0x17BDA2, plays sound 0x2A,
 * frees obj if (+0x50, +0x54) falls outside [-16, 335] x [-16, 247] (0x1849F8),
 * seeds stats from 0x1813F8 (0x184888), initializes animation index 2
 * (0x18140E -> 0x179A5C), installs script 0x181490, spawns child 0x17C4D4
 * (frees self on spawn failure), sets (+0x16, +0x1A) from sprite size << 3,
 * stores 0x1813C6 at +0x0C, and arms 0x17C240. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BDA2: polar place with exact D0/D1/A1 register outputs. */
static void sub_17BDA2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d2 = rd8(m, obj + 0x0Eu);
    uint16_t d3 = rd8(m, obj + 0x0Fu);
    int32_t c = (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d3 * 2u);
    int32_t s = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)d3 * 2u);
    int32_t rad = (int32_t)(int16_t)(uint16_t)(d2 << 2);
    uint32_t p0 = (uint32_t)(c * rad);
    uint32_t p1 = (uint32_t)(s * rad);

    Obj_PolarPlace(m, obj);
    r->d[0] = (p0 << 16) | rd16(m, obj + 0x50u);
    r->d[1] = (p1 << 16) | rd16(m, obj + 0x54u);
    r->a[1] = 0x00027E40u;
}

int Fn_1812FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_17BDA2(m, r);                               /* 0x1812FE */
    Sound_PlayHooked(m, 0x002Au);                   /* 0x181302..0x181312 */

    int16_t x = (int16_t)rd16(m, obj + 0x50u);      /* 0x181316 */
    r->d[0] = setw(r->d[0], (uint16_t)x);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu)) {
        Obj_Free(m, obj);                           /* 0x181322 -> 0x17C78E */
        return 0;
    }

    int16_t y = (int16_t)rd16(m, obj + 0x54u);      /* 0x181326 */
    r->d[0] = setw(r->d[0], (uint16_t)y);
    if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
        Obj_Free(m, obj);                           /* 0x181332 -> 0x17C78E */
        return 0;
    }

    Obj_184888(m, obj, 0x001813F8u);                /* 0x181336..0x18133C */

    uint16_t anim_idx = rd16(m, 0x0018140Eu);       /* 0x181342..0x181348 (2) */
    wr16(m, obj + 0x40u, anim_idx);                 /* 0x18134A */
    wr32(m, obj + 0x3Au, 0x00181410u);              /* 0x18134E */
    if (Obj_AnimInit(m, obj, anim_idx) < 0)         /* 0x181352..0x18135C */
        return -1;

    wr32(m, obj + 0x44u, 0x00181490u);              /* 0x18135E..0x181364 */
    wr16(m, obj + 0x48u, rd16(m, 0x00181494u));     /* 0x181368 */
    wr16(m, obj + 0x4Au, 0u);                       /* 0x18136E */
    r->a[1] = 0x00181490u;

    uint32_t child = Obj_Spawn(m, obj, 0x0017C4D4u, 3u, 2u, 7u); /* 0x181372..0x18138C */
    r->d[0] = child;
    wr32(m, obj + 0x62u, child);                    /* 0x181390 */
    if ((int32_t)child <= 0) {                      /* 0x181394 -> 0x17C78E */
        Obj_Free(m, obj);
        return 0;
    }

    uint32_t spr = rd32(m, obj + 0x10u);            /* 0x181398 */
    uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);   /* 0x18139C, 0x1813A2 */
    uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3); /* 0x18139E, 0x1813A0 */
    r->a[1] = spr + 4u;
    r->d[0] = setw(r->d[0], d0w);
    r->d[1] = setw(r->d[1], d1w);

    wr16(m, obj + 0x16u, d0w);                      /* 0x1813A4 */
    wr16(m, obj + 0x1Au, d1w);                      /* 0x1813A8 */
    wr16(m, obj + 0x58u, 0u);                       /* 0x1813AC */
    wr16(m, obj + 0x5Au, 0u);                       /* 0x1813B0 */
    wr8(m, obj + 0x34u, 2u);                        /* 0x1813B4 */
    wr32(m, obj + 0x0Cu, 0x001813C6u);              /* 0x1813BA */
    wr32(m, obj, 0x0017C240u);                      /* 0x1813C2 -> 0x17C238 */
    return 0;
}
