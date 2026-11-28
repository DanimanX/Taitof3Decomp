/* ROM 0x0E405C: spawner for the 0x0E-range flyer group leader (A0 = parent
 * object). Allocates two group-5 objects (ngroups 2, arg 0) with 0x0E4310 and
 * 0x0E437E; if the first allocation fails the parent dies (0x0CEE74). On
 * success 0x0CEC4C installs the mover from 0x0E3054, 0x0CF1D8 the script
 * 0x0E429E, 0x0D0050 clears the anim key (+0x0C->+0x28), speed +0x42 = 0x5A
 * and +0x08 = 0x005A0000 are stored, 0x0D005A picks the direction animation
 * (table 0x0D00B4 index 0x5A -> 0x0E31CC -> 0x0E32A4), +0x06 = 1 and the tick
 * is armed at 0x0E40DE. */
#include "fn_auto.h"
#include "../objects/obj_common.h"     /* Obj_StartDying */
#include "../objects/obj_pool.h"       /* Obj_Spawn */
#include "../enemy/obj_E8B72.h"        /* Obj_InitMover, Obj_SetScript */

#define DIR_TBL   0x000D00B4u    /* direction -> anim list offset table (word) */
#define ANIM_BASE 0x000E31BCu

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0050: clear the anim key at (+0x0C)->+0x28. */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, a1 + 0x28u, 0u);
}

/* ROM 0x0D005A: install the animation for direction D0.w from the 0x0D00B4
 * table; leaves D0 = the record's second word and A1 = the record + 8. */
static void sub_0D005A(const f3mem_t *m, uint32_t obj, uint32_t a1_in, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *a1_io)
{
    int16_t off = (int16_t)rd16(m, DIR_TBL + (uint32_t)(int32_t)(int16_t)d0w * 2u);
    uint32_t a1 = rd32(m, a1_in + (uint32_t)(int32_t)off);   /* 0x0D0062..0x0D0064 */
    uint32_t a2 = rd32(m, obj + 0x0Cu);                      /* 0x0D0066 */
    uint32_t d0 = setw(*d0_io, (uint16_t)off);               /* 0x0D005A move.w table,D0 */
    if (a1 == rd32(m, a2 + 0x28u)) {                         /* 0x0D006A bne/rts */
        *d0_io = d0;
        *a1_io = a1;
        return;
    }
    wr32(m, obj + 0x48u, a1);                                /* 0x0D0074 */
    wr32(m, a2 + 0x28u, a1);                                 /* 0x0D0078 */
    wr16(m, obj + 0x4Cu, rd16(m, a1));                       /* 0x0D007C */
    a1 += 2u;
    d0 = setw(d0, rd16(m, a1));                              /* 0x0D0080 */
    wr8(m, obj + 0x1Cu, (uint8_t)d0);                        /* 0x0D0082 */
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));                       /* 0x0D0086 */
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));                       /* 0x0D008A */
    *d0_io = d0;
    *a1_io = a1;
}

int Fn_0E405C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = r->a[1];

    d0 = Obj_Spawn(m, obj, 0x000E4310u, 5u, 2u, 0u);        /* 0x0E405C..0x0E4074 */
    if ((int32_t)d0 < 0) {                                  /* 0x0E4078 tst.l/bmi */
        Obj_StartDying(m, obj);                             /* 0x0E407A -> 0x0CEE74 */
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    d0 = Obj_Spawn(m, obj, 0x000E437Eu, 5u, 2u, 0u);        /* 0x0E4080..0x0E4098 */

    Obj_InitMover(m, obj, 0x000E3054u);                     /* 0x0E409C lea (-0x104a,PC) */
    Obj_SetScript(m, obj, 0x000E429Eu);                     /* 0x0E40A6 lea (0x1f6,PC) */
    sub_0D0050(m, obj);                                     /* 0x0E40B0 */

    d0 = setw(d0, 0x005Au);                                 /* 0x0E40B6 */
    wr16(m, obj + 0x42u, (uint16_t)d0);                     /* 0x0E40BA */
    wr32(m, obj + 0x08u, 0x005A0000u);                      /* 0x0E40BE */

    sub_0D005A(m, obj, ANIM_BASE, (uint16_t)d0, &d0, &a1); /* 0x0E40C6 lea (-0xf0c,PC) */

    wr8(m, obj + 0x06u, 1u);                                /* 0x0E40D0 */
    wr32(m, obj + 0x00u, 0x000E40DEu);                      /* 0x0E40D6 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
