/* ROM 0x19D7D2: segmented boss appendage initializer (A0 = object).
 * Clears common fields (0x17A7FA), sets hitboxes +0x2C/+0x30 = 0x80018 and
 * +0x28 = 0x10000, loads anim list 0x19DEB4 (index 0xA -> 0x179614/0x1798D4),
 * installs script 0x19DCF6 (+0x44/+0x48/+0x4A) and player target A5+0x1D28,
 * indexes 14-byte descriptor table 0x19DD9C by +0x0C.b into +0x16/+0x1A/+0x64/
 * +0x66/+0x68/+0x24, scales HP via 0x18495E, allocates 16-node trail buffer
 * in 0x41B8E8 via 0x19D988, sets +0x6A = 0xA and arms 0x19D876. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x19D988: allocate 16-entry trail buffer in 0x41B8E8 and inherit parent +0x50/+0x54. */
static void sub_19D988(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t idx = rd8(m, obj + 0x22u);
    uint32_t a3 = 0x0041B8E8u + (uint32_t)idx * 0x00C0u; /* 0x19D988..0x19D998 */
    wr32(m, obj + 0x0Cu, a3);                            /* 0x19D99C */

    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19D9A0 */
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);                             /* 0x19D9B4 */
    wr16(m, obj + 0x54u, y);                             /* 0x19D9B8 */

    for (int i = 0; i < 16; i++) {                       /* 0x19D9BC..0x19D9D2 */
        wr16(m, a3, x);
        wr16(m, a3 + 2u, y);
        wr16(m, a3 + 4u, x);
        wr16(m, a3 + 8u, y);
        a3 += 0x0Cu;
    }

    r->d[0] = setw(r->d[0], x);
    r->d[1] = setw(r->d[1], y);
    r->d[7] = setw(r->d[7], 0xFFFFu);
    r->a[3] = a3;
    r->a[4] = a4;
}

int Fn_19D7D2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Obj_17A7FA(m, obj);                                  /* 0x19D7D2 */
    wr32(m, obj + 0x2Cu, 0x00080018u);                   /* 0x19D7D8 */
    wr32(m, obj + 0x30u, 0x00080018u);                   /* 0x19D7E0 */
    wr32(m, obj + 0x28u, 0x00010000u);                   /* 0x19D7E8 */

    uint32_t a1 = 0x0019DEB4u;                           /* 0x19D7F0 */
    uint16_t anim_idx = rd16(m, a1); a1 += 2u;           /* 0x19D7F6 */
    wr16(m, obj + 0x40u, anim_idx);                      /* 0x19D7F8 */
    wr32(m, obj + 0x3Au, a1);                            /* 0x19D7FC */
    Obj_AnimInit(m, obj, anim_idx);                      /* 0x19D800..0x19D80A */

    a1 = 0x0019DCF6u;                                    /* 0x19D80C */
    wr32(m, obj + 0x44u, a1);                            /* 0x19D812 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));              /* 0x19D816 */
    wr16(m, obj + 0x4Au, 0u);                            /* 0x19D81C */
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));      /* 0x19D820 */
    wr16(m, obj + 0x62u, 0u);                            /* 0x19D826 */
    wr8(m, obj + 0x34u, 0x0Au);                          /* 0x19D82A */

    uint32_t d0 = rd32(m, obj + 0x0Cu);                  /* 0x19D830 */
    wr8(m, obj + 0x22u, (uint8_t)d0);                    /* 0x19D834 */
    d0 = (uint32_t)(uint16_t)d0 * 0x000Eu;               /* 0x19D838 mulu.w #0xe,D0 */
    a1 = 0x0019DD9Cu + (uint32_t)(int32_t)(int16_t)(uint16_t)d0; /* 0x19D83C..0x19D842 */
    wr16(m, obj + 0x16u, rd16(m, a1)); a1 += 2u;         /* 0x19D846 */
    wr16(m, obj + 0x1Au, rd16(m, a1)); a1 += 2u;         /* 0x19D84A */
    wr16(m, obj + 0x64u, rd16(m, a1)); a1 += 2u;         /* 0x19D84E */
    wr16(m, obj + 0x66u, rd16(m, a1)); a1 += 2u;         /* 0x19D852 */
    wr16(m, obj + 0x68u, rd16(m, a1)); a1 += 2u;         /* 0x19D856 */
    wr32(m, obj + 0x24u, rd32(m, a1)); a1 += 4u;         /* 0x19D85A */
    r->d[0] = d0;
    r->a[1] = a1;

    Obj_18495E(m, obj);                                  /* 0x19D85E */
    sub_19D988(m, r);                                    /* 0x19D864 */
    wr16(m, obj + 0x6Au, 0x000Au);                       /* 0x19D868 */
    wr32(m, obj, 0x0019D876u);                           /* 0x19D86E */
    return 0;
}
