#include "obj_C6866.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0017D6 */
static void sub_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d1, uint32_t d2, uint32_t d6)
{
    uint32_t a2 = (F3_A5 - 0x7F50u) + d6;
    int32_t div = (int32_t)d1;
    uint16_t d7 = (uint16_t)((uint16_t)d2 - 1u);

    /* 0017E2 */
    for (;;) {
        uint32_t c0 = rd32(m, a0);
        a0 += 4u;
        uint32_t c1 = rd32(m, a1);
        a1 += 4u;

        /* 0017E6 */
        uint32_t s = ((c0 >> 16) & 0xFFu) << 8;
        wr16(m, a2 + 0x00u, (uint16_t)s);
        uint32_t e = ((c1 >> 16) & 0xFFu) << 8;
        int32_t step = (int32_t)(e - s) / div;
        wr16(m, a2 + 0x06u, (uint16_t)(uint32_t)step);

        /* 0017FE */
        s = ((c0 >> 8) & 0xFFu) << 8;
        wr16(m, a2 + 0x02u, (uint16_t)s);
        e = ((c1 >> 8) & 0xFFu) << 8;
        step = (int32_t)(e - s) / div;
        wr16(m, a2 + 0x08u, (uint16_t)(uint32_t)step);

        /* 001818 */
        s = (c0 & 0xFFu) << 8;
        wr16(m, a2 + 0x04u, (uint16_t)s);
        e = (c1 & 0xFFu) << 8;
        step = (int32_t)(e - s) / div;
        wr16(m, a2 + 0x0Au, (uint16_t)(uint32_t)step);

        /* 001832 */
        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x001792 */
static uint32_t sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                           uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    uint16_t d7 = 0x000Fu;

    /* 0017A2 */
    for (;;) {
        if (rd16(m, a3) == 0u) {
            /* 0017A6 */
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return 0xFFFFFFFFu;
            /* 0017AC */
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return 0xFFFFFFFFu;
            /* 0017B0 */
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_0017D6(m, a0, a1, d1, d2, d6);
            return a3;
        }
        /* 0017BC */
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }

    /* 0017C8 */
    return 0xFFFFFFFFu;
}

/* ROM 0x001866 Pal_FadeFromBlack */
static uint32_t sub_001866(const f3mem_t *m, uint32_t dst, uint32_t pal_idx,
                           uint32_t frames, uint32_t count)
{
    return sub_001792(m, 0x0000190Eu, dst, pal_idx, frames, count);
}

/* ROM 0x001890 Pal_FadeToBlack */
static uint32_t sub_001890(const f3mem_t *m, uint32_t src, uint32_t pal_idx,
                           uint32_t frames, uint32_t count)
{
    return sub_001792(m, src, 0x0000190Eu, pal_idx, frames, count);
}

/* ROM 0x0C689E Player_PulsePalette */
static void sub_0C689E(const f3mem_t *m, uint32_t a0)
{
    int16_t idx = (int16_t)rd16(m, a0 + 0x22u);
    uint32_t a1 = 0x000C6912u + (uint32_t)((int32_t)idx * 4);
    uint32_t d1 = (uint16_t)(((uint16_t)rd8(m, a0 + 0x0Cu) << 6) + 0x4078u);

    /* 0C68B6 */
    if (rd16(m, a0 + 0x06u) == 0u) {
        sub_001866(m, a1, d1, 0x0000000Au, 0x00000001u);
    }

    /* 0C68DA */
    if (rd16(m, a0 + 0x06u) == 0x000Au) {
        sub_001890(m, a1, d1, 0x0000000Au, 0x00000001u);
    }

    /* 0C68FE */
    uint16_t t = (uint16_t)(rd16(m, a0 + 0x06u) + 1u);
    wr16(m, a0 + 0x06u, t);
    if ((int16_t)t >= 0x0014) {
        wr16(m, a0 + 0x06u, 0u);
    }
}

/* ROM 0x0C8460 */
static void sub_0C8460(const f3mem_t *m, uint32_t a0)
{
    uint16_t saved_x = rd16(m, a0 + 0x14u);
    uint16_t saved_y = rd16(m, a0 + 0x18u);

    wr16(m, a0 + 0x14u, (uint16_t)(saved_x + rd16(m, F3_A5 + 0x1D20u)));
    wr16(m, a0 + 0x18u, (uint16_t)(saved_y + rd16(m, F3_A5 + 0x1D24u)));
    Obj_DrawSprite(m, a0);
    wr16(m, a0 + 0x18u, saved_y);
    wr16(m, a0 + 0x14u, saved_x);
}

/* ROM 0x0C8682 Player_UpdateShield */
static void sub_0C8682(const f3mem_t *m, uint32_t player)
{
    if (rd8(m, player + 0x65u) & 0x04u)
        return;

    uint32_t a1 = player;
    uint32_t a0 = 0x0041B2A4u;
    wr16(m, a0 + 0x1Cu, 0u);
    int16_t p_idx = (int16_t)rd16(m, a1 + 0x22u);
    wr8(m, a0 + 0x1Du, rd8(m, 0x000C8548u + (uint32_t)(int32_t)p_idx));

    /* 0C86AA */
    if ((int16_t)rd16(m, a1 + 0x4Eu) <= 0)
        return;

    /* 0C86B2 */
    int16_t sh_tmr = (int16_t)rd16(m, a1 + 0x5Eu);
    wr16(m, a1 + 0x5Eu, (uint16_t)((uint16_t)sh_tmr - 1u));
    if (sh_tmr == 1) {
        /* 0C86BE */
        if (!(rd8(m, F3_A5 + 0x08A0u) & 0x80u)) {
            wr16(m, a1 + 0x4Eu, (uint16_t)(rd16(m, a1 + 0x4Eu) - 1u));
        }
    } else if (sh_tmr < 1) {
        /* 0C86D0 */
        wr16(m, a1 + 0x5Eu, 0u);
    }

    /* 0C86D4 */
    wr32(m, a0 + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, a0 + 0x18u, rd32(m, a1 + 0x18u));

    uint32_t a2;
    if ((int16_t)rd16(m, F3_A5 + 0x1D4Au) > 7) {
        /* 0C8708 */
        a2 = 0x000C8792u;
    } else if ((int16_t)rd16(m, a1 + 0x4Eu) > 1) {
        /* 0C86FE */
        a2 = 0x000C876Cu;
    } else {
        /* 0C86F4 */
        a2 = 0x000C8746u;
    }

    /* 0C870E */
    int16_t frame_off = (int16_t)rd16(m, F3_A5 + 0x1D46u);
    wr32(m, a0 + 0x10u, rd32(m, a2 + 2u + (uint32_t)(int32_t)frame_off));

    int16_t z_idx = (int16_t)rd16(m, F3_A5 + 0x1D4Au);
    uint32_t tbl = 0x000C87B8u + (uint32_t)((int32_t)z_idx * 4);
    wr8(m, a0 + 0x1Fu, rd8(m, tbl + 0u));
    wr8(m, a0 + 0x1Eu, rd8(m, tbl + 1u));
    int16_t ox = (int16_t)(int8_t)rd8(m, tbl + 2u);
    wr16(m, a0 + 0x14u, (uint16_t)(rd16(m, a0 + 0x14u) + (uint16_t)ox));
    int16_t oy = (int16_t)(int8_t)rd8(m, tbl + 3u);
    wr16(m, a0 + 0x18u, (uint16_t)(rd16(m, a0 + 0x18u) + (uint16_t)oy));

    /* 0C873C */
    sub_0C8460(m, a0);
}

/* ROM 0x0C6866 Obj_PlayersTick */
int Obj_C6866_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = P1_STATE_ADDR;

    /* 0C6874 */
    for (int i = 0; i < 2; i++) {
        if ((int16_t)rd16(m, p + 0x4Cu) >= 0) {
            /* 0C6880 */
            Player_MoveAndBounds(m, p);
            sub_0C8682(m, p);
            sub_0C689E(m, p);
        }
        /* 0C6890 */
        p += 0x8Au;
    }

    /* 0C685E */
    wr32(m, obj, 0x000C6866u);
    return 0;
}
