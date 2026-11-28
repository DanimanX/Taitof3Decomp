#include "fn_auto.h"
/* ROM 0x0E0424: spawner (A0 = parent object). 0x0CF046(0x0E0266, 2) allocates
 * two group-5 objects that register themselves in the A5+0x2862 pointer list
 * and copies the parent's +0x4E into the last one; the first one's tick becomes
 * 0x0DFC1A. On allocation failure the partial list is freed and the parent dies
 * (0x0CEE74). Then one 0x0E029E object is spawned (group 5, count 2) and the
 * parent is set up by 0x0CEC4C (mover, table 0x0E022E), 0x0CF1D8 (script
 * 0x0DFBA8) and 0x0D0678 (animation 0x0DEC56); +0x06 = 1 and the shared tail
 * at 0x0DF9B0 re-arms the tick to 0x0DF9D8. */
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x0CF046: spawn D1.w objects through 0x01057C (group 5, ngroups 2, arg
 * 0), chaining each as the next parent, storing each in the A5+0x2862 list
 * (8 bytes apart) and clearing its +0x4E. Returns the last object's address
 * with the low word cleared in D0; on failure frees the partial list and
 * returns -1. D1 leaves 0xFFFF from the dbf. */
static int spawn_chain(const f3mem_t *m, uint32_t *a0_io, uint32_t fn, int16_t d1_in,
                       int16_t *d1_io)
{
    uint32_t a3 = *a0_io;
    uint32_t a0 = *a0_io;
    int16_t d1 = (int16_t)(d1_in - 1);                  /* 0x0CF048 subq.w */
    uint32_t a2 = F3_A5 + 0x2862u;                      /* 0x0CF04A */
    uint32_t ptr;

    for (;;) {                                          /* 0x0CF04E */
        ptr = Obj_Spawn(m, a0, fn, 5u, 2u, 0u);         /* 0x0CF05C -> 0x01057C */
        wr32(m, a2, ptr);                               /* 0x0CF066 */
        a2 += 4u;
        if ((int32_t)ptr < 0)                           /* 0x0CF068 bmi */
            break;
        a0 = ptr;                                       /* 0x0CF06C */
        wr8(m, a0 + 0x4Eu, 0u);                         /* 0x0CF06E */
        d1--;
        if (d1 < 0)                                     /* dbf D1w */
            break;
    }

    if ((int32_t)ptr < 0) {                             /* 0x0CF082 */
        a2 = F3_A5 + 0x2862u;
        for (;;) {                                      /* 0x0CF086 */
            uint32_t p = rd32(m, a2);
            a2 += 4u;
            if ((int32_t)p < 0)
                break;
            Obj_Free(m, p);                             /* 0x0CF08C */
        }
        *a0_io = a3;
        *d1_io = d1;                                    /* dbf not reached on failure */
        return -1;
    }

    wr8(m, a0 + 0x4Eu, rd8(m, a3 + 0x4Eu));             /* 0x0CF076 */
    *a0_io = a3;                                        /* 0x0CF07E */
    *d1_io = d1;
    return (int)(ptr & 0xFFFF0000u);                    /* 0x0CF07C clr.w D0 */
}

int Fn_0E0424(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1;
    int16_t d1 = 0;
    int d0 = spawn_chain(m, &a0, 0x000E0266u, 2, &d1);  /* 0x0E0424..0x0E042A */

    if (d0 < 0) {                                       /* 0x0E0430 tst.w/bmi */
        r->a[0] = a0;
        r->a[1] = 0x000E0266u;                          /* A1 kept the spawn fn */
        r->d[0] = (uint32_t)d0;
        r->d[1] = (uint32_t)(uint16_t)d1;
        Obj_StartDying(m, a0);                          /* 0x0E0432 -> 0x0CEE74 */
        return 0;
    }

    a1 = rd32(m, F3_A5 + 0x2862u);                      /* 0x0E0438 */
    wr32(m, a1, 0x000DFC1Au);                           /* 0x0E043C */

    /* 0x0E0442..0x0E045A: pea #0 ; move.w #2 ; move.w #5 ; pea 0x0E029E ;
     * jsr 0x01057C ; lea (0xc,SP),SP. */
    d0 = (int)Obj_Spawn(m, a0, 0x000E029Eu, 5u, 2u, 0u);

    a1 = 0x000E022Eu;                                   /* 0x0E045E lea (pc) */
    Obj_InitMover(m, a0, a1);                           /* 0x0E0462 -> 0x0DF9B0 */
    a1 = 0x000DFBA8u;                                   /* 0x0DF9B6 lea (pc) */
    Obj_SetScript(m, a0, a1);                           /* 0x0DF9BA -> 0x0CF1D8 */
    a1 = 0x000DEC56u;                                   /* 0x0DF9C0 lea (pc) */
    Obj_SetAnim(m, a0, a1);                             /* 0x0DF9C4 -> 0x0D0678 */
    a1 += 8u;                                           /* left by 0x0D0678 */

    wr8(m, a0 + 0x06u, 1u);                             /* 0x0DF9CA */
    wr32(m, a0 + 0x00u, 0x000DF9D8u);                   /* 0x0DF9D0 */

    r->a[0] = a0;
    r->a[1] = a1;
    r->d[0] = (uint32_t)d0 & 0xFFFF0000u;               /* 0x0CF07C clr.w D0 */
    r->d[1] = (uint32_t)(uint16_t)d1;                   /* dbf D1w left -1 */
    return 0;
}
