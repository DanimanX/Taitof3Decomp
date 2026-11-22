#include "obj_10048C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_CEF44.h"

/* ROM 0x0CBE34: hit flash colour on obj+0x1D */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

/* ROM 0x0D0E04: credit the score to the killer object in obj+0x36 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit_by = rd8(m, obj + 0x35u);
    if (hit_by == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit_by == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}
/* ROM 0x0D0E30: add D0 to the score of the object at A5+0x1D50 */
static void sub_0D0E30(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}
/* ROM 0x0CEF0E: off-screen / pause handling that follows 0x0D0E30 */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
        (int16_t)rd16(m, obj + 0x14u) >= -0x000A &&
        (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr32(m, obj + 0x00u, 0x000CEF44u);
        return;
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x10048C */
int Obj_10048C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10048C: on-screen (pause bit 1) and inside the playfield */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
        (int16_t)rd16(m, obj + 0x14u) >= -0x000A &&
        (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr16(m, obj + 0x18u, 0u);
        sub_0D0E30(m, 0x0064u);      /* 0x1005B4 */
        sub_0CEF0E(m, obj);          /* 0x1005BC */
        return 0;
    }

    /* 0x1004B0 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x10055C: moveq 0x64,D0; jsr 0x0D0E04 */
        sub_0D0E04(m, obj, 0x0064u);
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        /* 0x1004B8: fell through to 0x100564 */
    } else {
        /* 0x1004C2 */
        uint32_t d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);

        /* 0x1004DA */
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);   /* 0x1004E6 */
            return 0;
        }

        /* 0x1004EC */
        if ((int16_t)rd16(m, obj + 0x3Eu) < 0x00AF) {
            /* 0x100534 */
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
            sub_0D0DC2(m, obj);
            sub_0CBE34(m, obj);
            Obj_DrawSprite(m, obj);
            wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x1388u);
            return 0;
        }

        /* 0x1004F6: Obj_Spawn(obj, 0x100A32, group 5, n 2, arg 0) */
        uint32_t child = Obj_Spawn(m, obj, 0x00100A32u, 5u, 2u, 0u);
        if ((int32_t)child < 0) {
            Obj_StartDying(m, obj);   /* 0x100514 */
            return 0;
        }
        wr32(m, child + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, child + 0x18u, rd32(m, obj + 0x3Eu));
        wr16(m, child + 0x10u, rd16(m, obj + 0x68u));
        Obj_StartDying(m, obj);       /* 0x10052E */
        return 0;
    }

    /* 0x100564: Obj_Spawn(obj, 0x100B94, group 9, n 2, arg 0) */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x00100B94u, 9u, 2u, 0u);
        if ((int32_t)child < 0) {
            Obj_StartDying(m, obj);   /* 0x100582 */
            return 0;
        }
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x14u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x14u));
        wr16(m, child + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
        Obj_StartDying(m, obj);       /* 0x1005AE */
    }
    return 0;
}
