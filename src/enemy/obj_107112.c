#include "obj_107112.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x107112: the helper sprite spawned by 0x1070DA. A4 = obj+0x8 is the
 * parent object; the state in obj+0x6 selects a handler from the table at
 * 0x107212 (0: scripted dash, 1: launch/walk, 2: gravity flight, 3: dive).
 * 0x0C6632 is the terrain ink test Terrain_TestPoint, 0x0D2D44 the per-angle
 * speed scale and 0x0D0E52 the octant angle Angle360. */

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0690: one 6-byte script step (timer +0x4C, ptr +0x48) */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DC2: append A0 to A5+0x8FE, then Coll_AddHazard (0x0D0DCC) */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables {0x0D2D64, 0x0D2ECCu}
 * indexed by D0w and scaled by D1w through the jump table 0x0D4F26. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    /* 0x0D4F26 [0..50] -> exact (D0*k, D1*k) */
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* Handler 0 (0x107222): scripted dash; flags "gone" once outside the bounds */
static void state0_move(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x3Eu));

    if ((int16_t)rd16(m, obj + 0x14u) < (int16_t)0xFFE0 ||
        (int16_t)rd16(m, obj + 0x18u) > (int16_t)0x00E8) {
        /* 0x107256 */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u)); /* bset.b #3 */
        return;
    }

    /* 0x107246: gravity on the vertical speed, then draw (0x1282) */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x000036B0u);
    Obj_DrawSprite(m, obj);
}

/* Handler 1 (0x10725E): walk down until the ink test hits, then launch */
static void state1_launch(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & (uint8_t)~0x20u)); /* bclr.b #5 */

    if (rd8(m, a4 + 0x06u) == 1u)
        return; /* 0x107376: nothing while the parent is in state 1 */

    /* 0x10726E */
    uint16_t d6 = (uint16_t)(rd16(m, obj + 0x14u) + 0x14u);
    uint16_t d7 = (uint16_t)(rd16(m, obj + 0x18u) + 0x0Au);

    if (Terrain_TestPoint(m, (int16_t)d6, (int16_t)d7) == 0) {
        /* 0x10728E */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
        if ((int16_t)rd16(m, obj + 0x68u) < 0)
            return; /* 0x107318 */
        sub_0D0678(m, obj, 0x0010744Eu);
        return; /* 0x107318 */
    }

    /* 0x1072A8: touches ink */
    if (rd16(m, obj + 0x68u) != 0u)
        sub_0D0678(m, obj, 0x0010741Eu);
    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x20u)); /* bset.b #5 */

    if (Terrain_TestPoint(m, (int16_t)(uint16_t)(d6 - 1u), (int16_t)d7) != 0) {
        /* 0x1072D2 */
        if (Terrain_TestPoint(m, (int16_t)(uint16_t)(d6 + 1u), (int16_t)d7) == 0)
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 1u));
    } else {
        /* 0x1072EC */
        if (Terrain_TestPoint(m, (int16_t)(uint16_t)(d6 + 1u), (int16_t)d7) != 0)
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
    }

    /* 0x107302 */
    if (Terrain_TestPoint(m, (int16_t)d6, (int16_t)(uint16_t)(d7 - 3u)) != 0)
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 2u));

    /* 0x107318 */
    if (rd8(m, a4 + 0x06u) <= 4u)
        return;
    if (rd8(m, a4 + 0x22u) != 2u)
        return;

    /* 0x10732C: launch */
    uint16_t d0 = 0x006Eu;
    if (rd8(m, a4 + 0x06u) != 5u)
        d0 = 0x0032u;
    uint32_t o0, o1;
    sub_0D2D44(m, d0, 0x001Eu, &o0, &o1);
    wr32(m, obj + 0x3Au, o0);
    wr32(m, obj + 0x3Eu, o1);
    sub_0D0678(m, obj, 0x0010744Eu);
    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & (uint8_t)~0x20u)); /* bclr.b #5 */
    wr8(m, obj + 0x06u, 0x02u);

    /* 0x107362: move.w #0x11,-(SP); jsr 0x13F238; jsr 0x39D8 */
    Sound_PlayHooked(m, 0x0011u);
}

/* Handler 2 (0x107378): gravity flight; once the parent leaves state 4, dives */
static void state2_slide(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    if (rd8(m, a4 + 0x06u) <= 4u)
        return; /* 0x1073F6; the handler is only entered from state 2 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x3Eu));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x000036B0u);
    if ((int32_t)rd32(m, obj + 0x3Eu) < 0)
        return; /* 0x1073A4 */

    wr8(m, obj + 0x06u, 0x03u);
}

/* Handler 3 (0x1073A6): dive; on ink, restart the launch sequence */
static void state3_dive(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    if (rd8(m, a4 + 0x06u) > 4u) {
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x3Eu));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x000036B0u);

        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x14u) + 0x14u);
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x18u) + 0x0Eu);
        if (Terrain_TestPoint(m, (int16_t)d0, (int16_t)d1) == 0)
            return; /* 0x1073E4 */

        /* 0x1073E6 */
        sub_0D0678(m, obj, 0x0010741Eu);
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x20u)); /* bset.b #5 */
    }

    /* 0x1073F6 */
    wr8(m, obj + 0x06u, 0x01u);
}

/* ROM 0x107112 */
int Obj_107112_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x107112 */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, 0x5Eu);

    /* 0x10711C */
    if (rd8(m, obj + 0x06u) != 0u) {
        if (rd8(m, a4 + 0x06u) == 0u) {
            /* 0x10712C: pick a random direction and speed once */
            uint16_t d0 = (uint16_t)((Obj_Next4Way(m) & 0x0007u) + 0x001Cu);
            uint16_t d1 = d0;
            d0 = (uint16_t)((Obj_Next4Way(m) & 0x003Fu) + 0x003Au);
            uint32_t o0, o1;
            sub_0D2D44(m, d0, d1, &o0, &o1);
            wr32(m, obj + 0x3Au, o0);
            wr32(m, obj + 0x3Eu, o1);
            wr8(m, obj + 0x06u, 0u);
        } else {
            /* 0x107160 */
            uint8_t d0b = rd8(m, a4 + 0x1Du);
            if (d0b != 0u)
                wr8(m, obj + 0x1Du, d0b);
            if (rd8(m, a4 + 0x35u) == 0u &&
                (int32_t)rd32(m, a4 + 0x24u) > 0) {
                /* 0x10717C: hand the helper's last hit over */
                wr8(m, a4 + 0x35u, rd8(m, obj + 0x35u));
                wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
                wr8(m, obj + 0x35u, 0u);
                if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
                    wr32(m, a4 + 0x24u, 0u);
                    wr32(m, obj + 0x24u, 0x001E0000u);
                }
            }
        }
    }

    /* 0x1071A0: scroll velocity applied to the helper position */
    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);
    d0 = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);

    /* 0x1071B0: jsr ([0x107212, pc, D0w * 4]) */
    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_move(m, obj); break;
    case 1u: state1_launch(m, obj, a4); break;
    case 2u: state2_slide(m, obj, a4); break;
    case 3u: state3_dive(m, obj, a4); break;
    default: return -1;
    }

    /* 0x1071BE: off the playfield -> generic dying tick (0x0CEE74) */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x1071CA */
    if (rd8(m, obj + 0x06u) == 0u)
        return 0;

    /* 0x1071D2 */
    sub_0D0DC2(m, obj);
    sub_0D0690(m, obj);
    if (rd8(m, a4 + 0x06u) != 1u)
        return 0;

    /* 0x1071EA: aim at the parent and close in by 7 units */
    d0 = (uint16_t)(rd16(m, obj + 0x14u) - 0x0091u);
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x18u) - 0x00A0u);
    uint32_t ang = Angle360(m, (uint16_t)d0, d1);
    uint32_t o0, o1;
    sub_0D2D44(m, (uint16_t)ang, 0x0007u, &o0, &o1);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + o0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + o1);
    return 0;
}
