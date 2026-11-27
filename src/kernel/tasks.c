/* Kernel task bodies: setup + TRAP #5 frame loop (see tasks.h).
 * Translated by Claude from the ROM listing. TaskCollision_Frame is verified
 * on 20 loop passes (test "Task 0x0C63F2 Collision loop", capture from the
 * resume PC to the next TRAP #5 with F3_NOIRQ=1) and TaskCollision_Init on 3
 * task starts (test "Task 0x0C63E8 Collision init"; weak: the lists were
 * already empty, only 2 bytes change). The Group11 task is verified in
 * tests/test_objpool.c (100 loop passes from zones 04/14/27, 10 of them paused
 * by F3_POKE8=406BB5=02; tick fn/obj order logged at 0x010568) and its setup
 * 0x010516 on 20 injected calls (test_fn "Task 0x010516 Group11 init"). */
#include "tasks.h"
#include "../collision/collision.h"

/* ROM 0x0C63E8 Task_Collision
 *   0C63E8  sf (0x1d1c,A5)
 *   0C63EC  bsr 0x0C640A            Coll_ResetLists
 *   0C63F0  trap #5                 (yield; the loop resumes at 0C63F2) */
void TaskCollision_Init(const f3mem_t *m)
{
    wr8(m, F3_A5 + 0x1D1Cu, 0x00u);
    Coll_ResetLists(m);
}

/*   0C63F2  move.b (0x916,A5),(0x917,A5)
 *   0C63F8  bsr 0x0C6446            Coll_PlayerShotsVsEnemies
 *   0C63FC  bsr 0x0C64FC            Coll_PlayersVsHazards
 *   0C6400  bsr 0x0C6624            Terrain_SyncScroll
 *   0C6404  bsr 0x0C640A            Coll_ResetLists
 *   0C6408  bra 0x0C63F0            (yield again) */
void TaskCollision_Frame(const f3mem_t *m)
{
    wr8(m, F3_A5 + 0x0917u, rd8(m, F3_A5 + 0x0916u));
    Coll_PlayerShotsVsEnemies(m);
    Coll_PlayersVsHazards(m);
    Terrain_SyncScroll(m);
    Coll_ResetLists(m);
}

/* ROM 0x010502 Task_ObjGroup11Runner: bsr 0x010516, then the loop at 0x010506.
 * 0x010516: jsr 0x010672 (Obj_ResetGroup11), then clear 0x6C0 words from
 * 0x4194E0 (move.w #0x6bf,D0 / move.w D1,(A0)+ / dbf). */
void TaskGroup11_Init(const f3mem_t *m)
{
    Obj_ResetGroup11(m);
    for (uint32_t i = 0; i < 0x6C0u; i++)
        wr16(m, 0x004194E0u + 2u * i, 0u);
}

/*   010506  trap #5
 *   010508  btst.b #1,(-0x144b,A5)
 *   01050E  bne 0x010506            paused: yield again
 *   010510  bsr 0x010532            Obj_RunGroup11
 *   010514  bra 0x010506 */
int TaskGroup11_Frame(const f3mem_t *m, obj_tick_cb cb, void *ctx)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)
        return 1;
    Obj_RunGroup11(m, cb, ctx);
    return 0;
}
