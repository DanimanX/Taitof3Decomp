/* ROM 0x182D24: boss/enemy sub-part spawner & init (A0 = object).
 * Inherits parent (+0x08) position + packed (+0x0C, +0x0D) offsets via 0x17BD68,
 * sets angle (+0x20) = +0x0F and state (+0x06) = +0x0E, frees if outside screen
 * bounds (0x1849F8/0x1849FC), spawns shadow/companion 0x183014 (group 9, 2),
 * seeds stats from 0x183032 (0x184888), initializes anim 0 from 0x183048, sets
 * script 0x1831D8, sets +0x0C = 0x182DD4, and arms 0x17BEC8. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_182D24(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Cu);
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du);

    Obj_InheritOffsetPos(m, obj);                        /* 0x182D24 -> 0x17BD68 */
    d0 = setw(d0, rd16(m, obj + 0x50u));
    d1 = setw(d1, rd16(m, obj + 0x54u));

    wr8(m, obj + 0x20u, rd8(m, obj + 0x0Fu));            /* 0x182D28 */
    wr8(m, obj + 0x06u, rd8(m, obj + 0x0Eu));            /* 0x182D2E */

    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);       /* 0x182D34 */
        d0 = setw(d0, (uint16_t)x);
        if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu)) {
            Obj_Free(m, obj);                            /* 0x17C78E */
            r->d[0] = d0;
            r->d[1] = d1;
            return 0;
        }
        int16_t y = (int16_t)rd16(m, obj + 0x54u);       /* 0x182D44 */
        d0 = setw(d0, (uint16_t)y);
        if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
            Obj_Free(m, obj);                            /* 0x17C78E */
            r->d[0] = d0;
            r->d[1] = d1;
            return 0;
        }
    }

    d0 = Obj_Spawn(m, obj, 0x00183014u, 9u, 2u, 0u);     /* 0x182D54..0x182D68 */
    if ((int32_t)d0 <= 0) {                              /* 0x182D72 */
        Obj_Free(m, obj);                                /* 0x17C78E */
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;
    }

    wr32(m, obj + 0x58u, d0);                            /* 0x182D78 */
    Obj_184888(m, obj, 0x00183032u);                     /* 0x182D7C..0x182D82 */
    d0 = rd32(m, obj + 0x24u);

    {
        uint32_t a1 = 0x00183048u;                       /* 0x182D88 */
        uint16_t anim_idx = rd16(m, a1);                 /* 0x182D8E */
        a1 += 2u;
        d0 = setw(d0, anim_idx);
        wr16(m, obj + 0x40u, anim_idx);                  /* 0x182D90 */
        wr32(m, obj + 0x3Au, a1);                        /* 0x182D94 */
        if (Obj_AnimInit(m, obj, anim_idx) < 0)          /* 0x182D98..0x182DA2 */
            return -1;
    }

    {
        uint32_t a1 = 0x001831D8u;                       /* 0x182DA4 */
        wr32(m, obj + 0x44u, a1);                        /* 0x182DAA */
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));          /* 0x182DAE */
        wr16(m, obj + 0x4Au, 0u);                        /* 0x182DB4 */
        r->a[1] = a1;
    }

    if (rd8(m, obj + 0x20u) != 0x80u)                    /* 0x182DB8..0x182DBE */
        wr8(m, obj + 0x1Cu, 1u);                         /* 0x182DC2 */

    wr32(m, obj + 0x0Cu, 0x00182DD4u);                   /* 0x182DC8 */
    wr32(m, obj + 0x00u, 0x0017BEC8u);                   /* 0x182DD0 -> 0x17BEC0 */
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
