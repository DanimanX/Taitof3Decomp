#include "obj_17A52A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17A52A: drifting debris spawned by 0x17A50E. Each frame it has a 1-in-6
 * chance (0x1794A4 with n = 5) of skipping the step, otherwise it rides the
 * parent's +0x50/+0x54 plus its own +0x16/+0x1A, jitters its angle and sparks
 * the hit effect 0x179E7A. When the boss-clear bit A5-0x144C.3 is set it just
 * frees itself. */
int Obj_17A52A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A52A: btst.b #3,(-0x144C,A5) ; bne.w 0x17A59A */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x08u) != 0u) {
        Obj_Free(m, obj);                       /* 0x17A59A */
        return 0;
    }

    /* 0x17A534: moveq #5,D0 ; jsr 0x1794A4 ; tst.w D0w ; bne.b 0x17A522 */
    if (Obj_RandRange(m, 5u) != 0u) {
        wr32(m, obj + 0x00u, 0x0017A52Au);      /* 0x17A522 */
        return 0;
    }

    /* 0x17A540: parent position + own offset -> +0x50/+0x54 */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
    }

    /* 0x17A55C: the angle byte -- bfextu (0xC,A0){16:8} and {24:8} pushed
     * through 0x1794A4 into the two halves of D1, then the whole D1 long is
     * the spawn argument (the spawned slot holds it at +0xC) */
    {
        uint32_t d = rd32(m, obj + 0x0Cu);
        uint32_t d1 = (uint32_t)Obj_RandRange(m, (uint16_t)((d >> 8) & 0xFFu)) << 16;
        d1 |= Obj_RandRange(m, (uint16_t)(d & 0xFFu));
        /* 0x17A57E: move.l D1,-(SP) ; move.w #2,-(SP) ; move.w #5,-(SP) ;
         * move.l #0x179E7A,-(SP) ; jsr 0x1057C */
        (void)Obj_Spawn(m, obj, 0x00179E7Au, 5u, 2u, d1);
    }

    wr32(m, obj + 0x00u, 0x0017A52Au);          /* 0x17A598 bra.b 0x17A522 */
    return 0;
}
