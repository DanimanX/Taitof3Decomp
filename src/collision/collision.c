/* Darius Gaiden (Taito F3) — per-frame collision pass (Task_Collision, slot 3).
 * Decompiled from ROM 0x0C63F2..0x0C6630. Entities share the EntityObj header
 * (pos 16.16 at +0x14/+0x18, hp +0x24, contact_dmg +0x28, hitbox +0x2C..0x32,
 * coll_flags +0x34, hit_type +0x35, hit_by +0x36). */
#include "collision.h"

#define E_POS_X 0x14
#define E_POS_Y 0x18
#define E_HP    0x24
#define E_DMG   0x28
#define E_HBX0  0x2C
#define E_HBX1  0x2E
#define E_HBY0  0x30
#define E_HBY1  0x32
#define E_FLAGS 0x34
#define E_HIT   0x35
#define E_HITBY 0x36
#define PLAYERS 0x41B190u
#define PLAYER_SIZE 0x8A
#define BOMB_ACTIVE_FLAGS (F3_A5 + 0x1D58)

coll_stats_t coll_stats;

typedef struct { int16_t x0, x1, y0, y1; } box_t;

static box_t box_of(const f3mem_t *m, uint32_t e)
{
    int16_t x = (int16_t)rd16(m, e + E_POS_X), y = (int16_t)rd16(m, e + E_POS_Y);
    box_t b = {
        (int16_t)(rd16(m, e + E_HBX0) + x), (int16_t)(rd16(m, e + E_HBX1) + x),
        (int16_t)(rd16(m, e + E_HBY0) + y), (int16_t)(rd16(m, e + E_HBY1) + y),
    };
    return b;
}

/* 68k test order: skip if b.x1 < a.x0, b.x0 > a.x1, b.y1 < a.y0, b.y0 > a.y1 */
static int overlap(box_t a, box_t b)
{
    return !(b.x1 < a.x0 || b.x0 > a.x1 || b.y1 < a.y0 || b.y0 > a.y1);
}

static void sub_hp(const f3mem_t *m, uint32_t e, uint32_t dmg)
{
    wr32(m, e + E_HP, rd32(m, e + E_HP) - dmg);
}

void Coll_ResetLists(const f3mem_t *m)
{
    wr32(m, COLL_LIST1_PREV + 0, rd32(m, COLL_LIST1_END));
    wr32(m, COLL_LIST1_PREV + 4, rd32(m, COLL_LIST2_END));
    wr32(m, COLL_LIST1_PREV + 8, rd32(m, COLL_LIST3_END));
    wr32(m, COLL_LIST1_END, COLL_LIST1);
    wr32(m, COLL_LIST2_END, COLL_LIST2);
    wr32(m, COLL_LIST3_END, COLL_LIST3);
    wr32(m, COLL_BOMB_DMG + 0, 0);
    wr32(m, COLL_BOMB_DMG + 4, 0);
}

void Coll_PlayerShotsVsEnemies(const f3mem_t *m)
{
    int32_t shots_end = (int32_t)rd32(m, COLL_LIST2_END);
    for (uint32_t ps = COLL_LIST2; (int32_t)ps < shots_end; ps += 4) {
        uint32_t shot = rd32(m, ps);
        box_t s = box_of(m, shot);
        uint32_t pe = rd32(m, COLL_LIST1_END);
        while ((int32_t)pe > (int32_t)COLL_LIST1) {
            pe -= 4;
            uint32_t en = rd32(m, pe);
            if (!overlap(s, box_of(m, en)))
                continue;
            uint8_t ef = rd8(m, en + E_FLAGS);
            coll_stats.shot_hits++;
            wr8(m, en + E_HIT, 1);
            wr8(m, shot + E_HIT, 0xFF);
            wr32(m, en + E_HITBY, shot);
            wr32(m, shot + E_HITBY, en);
            if (!(ef & 0x01)) sub_hp(m, shot, rd32(m, en + E_DMG));
            if (!(rd8(m, en + E_FLAGS) & 0x02)) sub_hp(m, en, rd32(m, shot + E_DMG));
            if (!(rd8(m, en + E_FLAGS) & 0x40))
                break;                       /* shot consumed: next shot */
        }
    }
}

void Coll_PlayersVsHazards(const f3mem_t *m)
{
    uint32_t top = rd32(m, COLL_LIST3_END);
    for (int i = 0; i < 2; i++) {
        uint32_t pl = PLAYERS + (uint32_t)i * PLAYER_SIZE;
        uint32_t bomb = COLL_BOMB_DMG + 4u * (uint32_t)i;
        if (rd8(m, pl + E_FLAGS) || (rd8(m, pl + 0x65) & 0x04))
            continue;
        box_t p = box_of(m, pl);
        for (uint32_t ph = top; (int32_t)ph > (int32_t)COLL_LIST3; ) {
            ph -= 4;
            uint32_t hz = rd32(m, ph);
            if (!(rd8(m, hz + E_FLAGS) & 0x20)) {
                int32_t dmg = (int32_t)rd32(m, bomb);
                if (dmg > 0 && ((rd8(m, BOMB_ACTIVE_FLAGS) & 1) || !(rd8(m, hz + E_FLAGS) & 0x10))) {
                    coll_stats.bomb_hits++;
                    wr8(m, hz + E_HIT, 2);
                    wr32(m, hz + E_HITBY, pl);
                    if (!(rd8(m, hz + E_FLAGS) & 0x02)) sub_hp(m, hz, (uint32_t)dmg);
                }
                if (rd8(m, BOMB_ACTIVE_FLAGS) & 1)
                    continue;                /* bomb active: no body contact */
            }
            if (!overlap(p, box_of(m, hz)))
                continue;
            coll_stats.hazard_hits++;
            wr8(m, hz + E_HIT, 2);
            wr32(m, hz + E_HITBY, pl);
            wr32(m, pl + E_HITBY, hz);
            if (!(rd8(m, hz + E_FLAGS) & 0x02)) sub_hp(m, hz, rd32(m, pl + E_DMG));
            if (!(rd8(m, hz + E_FLAGS) & 0x01)) sub_hp(m, pl, rd32(m, hz + E_DMG));
        }
    }
}

void Terrain_SyncScroll(const f3mem_t *m)
{
    wr16(m, F3_A5 + 0x0918, rd16(m, 0x402258));
    wr16(m, F3_A5 + 0x091A, rd16(m, 0x40225C));
}

void Task_Collision_Body(const f3mem_t *m)
{
    wr8(m, F3_A5 + 0x0917, rd8(m, F3_A5 + 0x0916)); /* debug hitbox overlay latch */
    Coll_PlayerShotsVsEnemies(m);
    Coll_PlayersVsHazards(m);
    Terrain_SyncScroll(m);
    Coll_ResetLists(m);
}
