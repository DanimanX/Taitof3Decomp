#include "obj_11967A.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"

/* ROM 0x11967A: entrance of the "GOLDEN OGRE" stage-intro object -- it spawns
 * the banner attractor (0x119974, group 0 type 1) and three palette uploads
 * (0x1BC80 -> 0x4580, 0x119874 -> 0x4080, 0x1198F4 -> 0x4100), clears the 0x44
 * slot table, installs the 0x11983E work record in +0x3A and parks at 0x119714,
 * which replays that record as a vertical sprite slide (not translated here). */

int Obj_11967A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11967A: pea #0 ; move.w #1,-(SP) ; move.w #0,-(SP) ;
     * move.l #0x119974,-(SP) ; jsr 0x1057C -- group 0, one group */
    (void)Obj_Spawn(m, obj, 0x00119974u, 0u, 1u, 0u);

    /* 0x119696 / 0x1196B2 / 0x1196CE: Pal_QueueUpload(src, dst, count) */
    Pal_QueueUpload(m, 0x0001BC80u, 0x4580u, 0x0020u);
    Pal_QueueUpload(m, 0x00119874u, 0x4080u, 0x0020u);
    Pal_QueueUpload(m, 0x001198F4u, 0x4100u, 0x0020u);

    /* 0x1196EA: move.l #0x11983E,(0x3A,A0) ; clr.l (0x3E,A0) */
    wr32(m, obj + 0x3Au, 0x0011983Eu);
    wr32(m, obj + 0x3Eu, 0u);

    /* 0x1196F6: lea (0x44,A0),A1 ; move.w #5,D0w ; dbf -- six long clears */
    for (uint32_t i = 0u; i < 6u; i++)             /* +0x44 .. +0x5B */
        wr32(m, obj + 0x44u + i * 4u, 0u);

    wr16(m, obj + 0x1Eu, 0u);                   /* 0x119704 */
    wr16(m, obj + 0x1Cu, 0u);                   /* 0x119708 */

    /* 0x11970C: move.l #0x119714,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x00119714u); /* 0x11970C -> scroll driver lab_119714 */
    return 0;
}
