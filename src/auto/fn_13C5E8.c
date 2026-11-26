/* ROM 0x13C5E8: enemy/projectile init (A0 = object).
 * Clears per-frame fields via 0x0CBD86, sets timer +0x4C = 0x1770, unpacks
 * +0x0C bitfields {0:12} -> +0x50 (-0x30), {12:12} -> +0x52 (-0x10, setting
 * +0x1C bit 1 if negative), {24:8} -> +0x22 / +0x42 = -(val+6) / +0x06 =
 * (val<<5)+1, clears +0x54/+0x56, sets sprite 0x0003C664, arms 0x13C652. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_13C5E8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    Obj_ResetFields(m, obj);                            /* 0x13C5E8: jsr 0x0CBD86 */
    wr8(m, obj + 0x4Fu, 0u);                            /* 0x13C5EE */
    wr16(m, obj + 0x4Cu, 0x1770u);                      /* 0x13C5F2 */

    uint32_t arg = rd32(m, obj + 0x0Cu);
    d0 = (uint32_t)((int32_t)arg >> 20);                /* 0x13C5F8: bfexts (0xC,A0){0:12},D0 */
    d0 = setw(d0, (uint16_t)(d0 - 0x0030u));            /* 0x13C5FE */
    wr16(m, obj + 0x50u, (uint16_t)d0);                 /* 0x13C602 */

    d0 = (uint32_t)((int32_t)(arg << 12) >> 20);        /* 0x13C606: bfexts (0xC,A0){12:12},D0 */
    if ((int32_t)d0 < 0)                                /* 0x13C60C */
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u)); /* 0x13C610 */
    d0 = setw(d0, (uint16_t)(d0 - 0x0010u));            /* 0x13C616 */
    wr16(m, obj + 0x52u, (uint16_t)d0);                 /* 0x13C61A */

    d0 = arg & 0x000000FFu;                             /* 0x13C61E: bfextu (0xC,A0){24:8},D0 */
    wr16(m, obj + 0x22u, (uint16_t)d0);                 /* 0x13C624 */
    d1 = setw(d1, (uint16_t)d0);                        /* 0x13C628 */
    d0 = setw(d0, (uint16_t)(d0 + 6u));                 /* 0x13C62A */
    d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));       /* 0x13C62C */
    wr16(m, obj + 0x42u, (uint16_t)d0);                 /* 0x13C62E */
    d1 = setw(d1, (uint16_t)(((uint16_t)d1 << 5) + 1u)); /* 0x13C632..0x13C634 */
    wr16(m, obj + 0x06u, (uint16_t)d1);                 /* 0x13C636 */
    wr16(m, obj + 0x54u, 0u);                           /* 0x13C63A */
    wr16(m, obj + 0x56u, 0u);                           /* 0x13C63E */
    wr32(m, obj + 0x10u, 0x0003C664u);                  /* 0x13C642 */
    wr32(m, obj, 0x0013C652u);                          /* 0x13C64A */

    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
