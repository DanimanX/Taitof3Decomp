#include "fn_auto.h"
/* ROM 0x0E48B8: initialise a scripted enemy part (A0 = object). 0x0CEC4C
 * installs the mover from 0x0E3054, 0x0CF1D8 the script 0x0E429E, 0x0D0050
 * clears the anim key at (+0x0C)->+0x28; then the speed word +0x42 = 0x5A and
 * +0x08 = 0x005A0000 are stored and 0x0D005A picks the direction animation from
 * table 0x0D00B4 index 0x5A (record 0x0E4796, whose colour word 0 is left in
 * D0 and whose +8 is left in A1). +0x06 = 1 and the tick is armed at 0x0E48FA. */
#include "../enemy/obj_E8B72.h"      /* Obj_InitMover, Obj_SetScript */

#define DIR_TBL   0x000D00B4u        /* direction -> anim list offset table (word) */
#define ANIM_BASE 0x000E470Eu

/* ROM 0x0D0050: clear the anim key at (+0x0C)->+0x28. */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, a1 + 0x28u, 0u);
}

/* ROM 0x0D005A: install the animation for direction D0.w from the 0x0D00B4
 * table; leaves D0 = `move.w (A1)+` of the record's colour word (high half
 * cleared) and A1 = the record + 8. When the record is already the slot's
 * current animation it returns early with D0 = the table offset and A1 = the
 * record itself. */
static void sub_0D005A(const f3mem_t *m, uint32_t obj, uint32_t a1_in, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t off = rd16(m, DIR_TBL + (uint32_t)(int32_t)(int16_t)d0w * 2u); /* 0x0D005A */
    uint32_t a1 = a1_in + (uint32_t)(int32_t)(int16_t)off;   /* 0x0D0062 adda.w */
    a1 = rd32(m, a1);                                        /* 0x0D0064 movea.l (A1),A1 */
    uint32_t a2 = rd32(m, obj + 0x0Cu);                      /* 0x0D0066 */
    uint32_t d0 = off;                                       /* move.w clears D0 high */
    if (a1 == rd32(m, a2 + 0x28u)) {                         /* 0x0D006A cmpa.l ; beq */
        *d0_io = d0;
        *a1_io = a1;
        return;                                              /* 0x0D0072 rts */
    }
    wr32(m, obj + 0x48u, a1);                                /* 0x0D0074 */
    wr32(m, a2 + 0x28u, a1);                                 /* 0x0D0078 */
    wr16(m, obj + 0x4Cu, rd16(m, a1));                       /* 0x0D007C */
    a1 += 2u;
    d0 = rd16(m, a1);                                        /* 0x0D0080 move.w (A1)+,D0 */
    wr8(m, obj + 0x1Cu, (uint8_t)d0);                        /* 0x0D0082 */
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));                       /* 0x0D0086 */
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));                       /* 0x0D008A move.l (A1) */
    *d0_io = d0;
    *a1_io = a1;
}

int Fn_0E48B8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = 0x0000005Au;                               /* 0x0E48D2 move.w #0x5A,D0w */
    uint32_t a1 = r->a[1];

    Obj_InitMover(m, a0, 0x000E3054u);                       /* 0x0E48B8 lea (-0x1866,PC) */
    Obj_SetScript(m, a0, 0x000E429Eu);                       /* 0x0E48C2 lea (-0x626,PC) */
    sub_0D0050(m, a0);                                       /* 0x0E48CC jsr 0x0D0050 */

    wr16(m, a0 + 0x42u, (uint16_t)d0);                       /* 0x0E48D6 */
    wr32(m, a0 + 0x08u, 0x005A0000u);                        /* 0x0E48DA */

    sub_0D005A(m, a0, ANIM_BASE, (uint16_t)d0, &d0, &a1);    /* 0x0E48E2 lea (-0x1d6,PC) */

    wr8(m, a0 + 0x06u, 1u);                                  /* 0x0E48EC */
    wr32(m, a0 + 0x00u, 0x000E48FAu);                        /* 0x0E48F2 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;                                                /* 0x0E48F8 rts */
}
