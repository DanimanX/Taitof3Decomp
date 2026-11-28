/* ROM 0x0C95DE: rear-gun shot object tick (A0 = object, +0x08 = the turret's
 * parent). The word at +0x0C picks the branch:
 *  - +0x0C < 0: fire. 0x0CBD86 clears the fields, the parent's +0x0C colour
 *    byte becomes +0x1D, the parent's +0x65 gets bits 1 and 2, and the two
 *    queue arguments are the long from table 0x0C982C[parent+0x22] and
 *    ((parent+0x0C << 6) + 0x4000); 0x001660 pushes {0x20, arg, pointer} into
 *    the A5-0x6750 ring. Then the hitbox halves +0x2C/+0x30 are cleared, sound
 *    0x10 plays, sprite 0x00056ED0 with +0x2E = 0x0600 / +0x32 = 0 / +0x22 = 0
 *    is set up and the tick becomes 0x0C966E.
 *  - otherwise the same queue push with table 0x0C92DA[parent+0x22] and the
 *    object is freed (0x0105EC). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"      /* Sound_PlayHooked */
#include "../objects/obj_common.h"    /* Obj_ResetFields */
#include "../objects/obj_pool.h"      /* Obj_Free */

#define TBL_FIRE 0x000C982Cu   /* 0x0C9618 + 0x214 */
#define TBL_FREE 0x000C92DAu   /* 0x0C9768 - 0x48E */
#define FX_RING  (F3_A5 - 0x6750u)
#define FX_COUNT (F3_A5 - 0x5DF0u)
#define FX_FLAG  (F3_A5 - 0x5DEEu)

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001660: queue {d1.w, a0.l, d0.w} in the 0x20-entry A5-0x6750 ring. */
static void sub_001660(const f3mem_t *m, uint32_t a0, uint32_t d0, uint32_t d1)
{
    uint16_t n = rd16(m, FX_COUNT);                     /* 0x001678 */
    if ((int16_t)n >= 0x0020)                           /* 0x00167C cmpi/bge */
        return;
    uint32_t a2 = FX_RING + (uint32_t)(uint16_t)(n << 3); /* 0x001682..0x001688 */
    wr16(m, a2, (uint16_t)d1);                          /* 0x00168A */
    wr32(m, a2 + 2u, a0);                               /* 0x00168C */
    wr16(m, a2 + 6u, (uint16_t)d0);                     /* 0x001690 */
    wr16(m, FX_FLAG, 2u);                               /* 0x001694 */
    wr16(m, FX_COUNT, (uint16_t)(n + 1u));              /* 0x00169A */
}

/* (parent+0x0C << 6) + 0x4000, the second queue argument of both branches. */
static uint32_t gun_arg(const f3mem_t *m, uint32_t a1)
{
    uint32_t d1 = rd8(m, a1 + 0x0Cu);                   /* moveq #0/move.b */
    d1 = (uint16_t)(d1 << 6);                           /* 0x0C9610 lsl.w #6 */
    return (uint16_t)(d1 + 0x4000u);                    /* 0x0C9612 addi.w */
}

int Fn_0C95DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x0C95DE */

    if ((int16_t)rd16(m, obj + 0x0Cu) >= 0) {           /* 0x0C95E2 bge 0x0C9752 */
        d0 = setw(d0, rd16(m, a1 + 0x22u));             /* 0x0C9756 */
        uint32_t d1 = gun_arg(m, a1);                   /* 0x0C975A..0x0C9762 */
        d0 = rd32(m, TBL_FREE + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 4u); /* 0x0C9766 */
        sub_001660(m, d0, d1, 0x00000020u);             /* 0x0C976C..0x0C977C */
        Obj_Free(m, obj);                               /* 0x0C9782 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    Obj_ResetFields(m, obj);                            /* 0x0C95EA -> 0x0CBD86 */
    a1 = rd32(m, obj + 0x08u);                          /* 0x0C95F0 */
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x0Cu));            /* 0x0C95F4 */
    wr8(m, a1 + 0x65u, (uint8_t)(rd8(m, a1 + 0x65u) | 0x02u)); /* 0x0C95FA bset #1 */
    wr8(m, a1 + 0x65u, (uint8_t)(rd8(m, a1 + 0x65u) | 0x04u)); /* 0x0C9600 bset #2 */
    d0 = setw(d0, rd16(m, a1 + 0x22u));                 /* 0x0C9606 */
    uint32_t d1 = gun_arg(m, a1);                       /* 0x0C960A..0x0C9612 */
    d0 = rd32(m, TBL_FIRE + (uint32_t)(int32_t)(int16_t)(uint16_t)d0 * 4u); /* 0x0C9616 */
    sub_001660(m, d0, d1, 0x00000020u);                 /* 0x0C961E..0x0C962E */

    wr16(m, obj + 0x2Cu, 0u);                           /* 0x0C9632 */
    wr16(m, obj + 0x30u, 0u);                           /* 0x0C9636 */
    Sound_PlayHooked(m, 0x0010u);                       /* 0x0C963A..0x0C964A */
    wr32(m, obj + 0x10u, 0x00056ED0u);                  /* 0x0C964E */
    wr16(m, obj + 0x2Eu, 0x0600u);                      /* 0x0C9656 */
    wr16(m, obj + 0x32u, 0x0000u);                      /* 0x0C965C */
    wr16(m, obj + 0x22u, 0u);                           /* 0x0C9662 */
    wr32(m, obj + 0x00u, 0x000C966Eu);                  /* 0x0C9666 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
