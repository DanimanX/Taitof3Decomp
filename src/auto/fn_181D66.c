/* ROM 0x181D66: polar-placed enemy/projectile initializer (A0 = object).
 * Places object relative to parent (+0x08) via 0x17BDA2, frees via 0x0105EC
 * if outside [-0x10..0x14F] x [-0x10..0xF7] (cmp2.w 0x1849F8/0x1849FC).
 * Otherwise seeds stats from 0x181E42 via 0x184888, initializes anim
 * 0x181E58 (index 0 -> 0x179550), installs script 0x181E78, sets +0x34 = 2,
 * +0x0C = 0x181DD8, and arms 0x17C14C. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BDA2 with D0/D1/A1/A4 register tracking. */
static void polar_place_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t d0_init = (int16_t)(int8_t)rd8(m, obj + 0x0Cu); /* 0x17BDA2 */
    int16_t d1_init = (int16_t)(int8_t)rd8(m, obj + 0x0Du); /* 0x17BDA8 */
    uint16_t d2w = (uint16_t)(rd8(m, obj + 0x0Eu) << 2);    /* 0x17BDAE, 0x17BDDC */
    uint16_t d3w = rd8(m, obj + 0x0Fu);                     /* 0x17BDB4 */

    wr16(m, obj + 0x16u, (uint16_t)d0_init);                /* 0x17BDBA */
    wr16(m, obj + 0x1Au, (uint16_t)d1_init);                /* 0x17BDBE */
    wr16(m, obj + 0x20u, 0u);                               /* 0x17BDC2 */
    wr8(m, obj + 0x20u, (uint8_t)d3w);                      /* 0x17BDC6 */

    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d3w * 2u); /* 0x17BDD0 */
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d3w * 2u);         /* 0x17BDD6 */
    uint32_t p0 = (uint32_t)((int32_t)c * (int32_t)(int16_t)d2w);           /* 0x17BDDE */
    uint32_t p1 = (uint32_t)((int32_t)s * (int32_t)(int16_t)d2w);           /* 0x17BDE0 */
    p0 = (p0 << 16) | (p0 >> 16);                                           /* 0x17BDE2 swap */
    p1 = (p1 << 16) | (p1 >> 16);                                           /* 0x17BDE4 swap */
    wr16(m, obj + 0x16u, (uint16_t)(rd16(m, obj + 0x16u) + (uint16_t)p0)); /* 0x17BDE6 */
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)p1)); /* 0x17BDEA */

    uint32_t a4 = rd32(m, obj + 0x08u);                     /* 0x17BDEE */
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)); /* 0x17BDF2..0x17BDFA */
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)); /* 0x17BDF6..0x17BDFE */
    wr16(m, obj + 0x50u, x);                                /* 0x17BE02 */
    wr16(m, obj + 0x54u, y);                                /* 0x17BE06 */
    wr16(m, obj + 0x14u, x);                                /* 0x17BE0A */
    wr16(m, obj + 0x18u, y);                                /* 0x17BE0E */

    r->d[0] = setw(p0, x);
    r->d[1] = setw(p1, y);
    r->a[1] = 0x00027E40u;
    r->a[4] = a4;
}

int Fn_181D66(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    polar_place_regs(m, r);                              /* 0x181D66 -> 0x17BDA2 */

    int16_t x = (int16_t)rd16(m, obj + 0x50u);           /* 0x181D6A */
    r->d[0] = setw(r->d[0], (uint16_t)x);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu)) { /* 0x181D6E cmp2.w */
        Obj_Free(m, obj);                                /* 0x181D76 -> 0x17C78E */
        return 0;
    }

    int16_t y = (int16_t)rd16(m, obj + 0x54u);           /* 0x181D7A */
    r->d[0] = setw(r->d[0], (uint16_t)y);
    if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) { /* 0x181D7E cmp2.w */
        Obj_Free(m, obj);                                /* 0x181D86 -> 0x17C78E */
        return 0;
    }

    Obj_184888(m, obj, 0x00181E42u);                     /* 0x181D8A..0x181D90 */
    r->d[0] = rd32(m, obj + 0x24u);                      /* 0x1848C4..0x1848D0 */

    {
        uint32_t a1 = 0x00181E58u;                       /* 0x181D96 */
        uint16_t anim_idx = rd16(m, a1); a1 += 2u;       /* 0x181D9C */
        wr16(m, obj + 0x40u, anim_idx);                  /* 0x181D9E */
        wr32(m, obj + 0x3Au, a1);                        /* 0x181DA2 */
        if (Obj_AnimInit(m, obj, anim_idx) < 0)          /* 0x181DA6..0x181DB0 */
            return -1;
    }

    {
        uint32_t a1 = 0x00181E78u;                       /* 0x181DB2 */
        r->a[1] = a1;
        wr32(m, obj + 0x44u, a1);                        /* 0x181DB8 */
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));          /* 0x181DBC */
        wr16(m, obj + 0x4Au, 0u);                        /* 0x181DC2 */
    }

    wr8(m, obj + 0x34u, 2u);                             /* 0x181DC6 */
    wr32(m, obj + 0x0Cu, 0x00181DD8u);                   /* 0x181DCC */
    wr32(m, obj + 0x00u, 0x0017C14Cu);                   /* 0x181DD4 -> 0x17C144 */
    return 0;
}
