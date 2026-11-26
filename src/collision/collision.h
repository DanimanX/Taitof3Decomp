#ifndef TAITOF3_COLLISION_H
#define TAITOF3_COLLISION_H
#include "../common/f3mem.h"

/* Collision lists (pointer arrays) and their end cursors (A5-relative). */
#define COLL_BOMB_DMG     0x41A980u  /* s32[2] area damage per player */
#define COLL_LIST1        0x41A988u  /* shootable entities */
#define COLL_LIST2        0x41ABE0u  /* player shots */
#define COLL_LIST3        0x41ACE0u  /* hazards (hurt players) */
#define COLL_LIST1_END    (F3_A5 + 0x08FE)
#define COLL_LIST2_END    (F3_A5 + 0x0902)
#define COLL_LIST3_END    (F3_A5 + 0x0906)
#define COLL_LIST1_PREV   (F3_A5 + 0x090A)

/* Instrumentation for tests (not part of the original code). */
typedef struct { unsigned shot_hits, hazard_hits, bomb_hits; } coll_stats_t;
extern coll_stats_t coll_stats;

void Coll_ResetLists(const f3mem_t *m);           /* ROM 0x0C640A */
void Coll_PlayerShotsVsEnemies(const f3mem_t *m); /* ROM 0x0C6446 */
void Coll_PlayersVsHazards(const f3mem_t *m);     /* ROM 0x0C64FC */
void Terrain_SyncScroll(const f3mem_t *m);        /* ROM 0x0C6624 */
void Task_Collision_Body(const f3mem_t *m);       /* ROM 0x0C63F2..0x0C6408 */
void Coll_ListAppend0D0DE4(const f3mem_t *m, uint32_t obj); /* ROM 0x0D0DE4 */

#endif
