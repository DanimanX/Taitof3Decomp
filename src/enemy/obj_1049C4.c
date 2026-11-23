#include "obj_1049C4.h"
#include "../objects/obj_shared.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CBE34 */

/* ROM 0x0D0690 */
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
        d0 = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)d0);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E52 */
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = (d1 > d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        unsigned sh = (unsigned)(d3 & 63u);
        d0 = (sh >= 16u) ? 0u : (uint16_t)(d0 >> sh);
        d1 = (sh >= 16u) ? 0u : (uint16_t)(d1 >> sh);
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    d1 = (uint16_t)(d1 + d0);
    d0 = (uint16_t)rd8(m, 0x000D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x000D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x0D1EBC */
static int sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);

        /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k is the
         * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
         * mod 2^32 is exact for every k the table has. */

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0D25E4 */
static int sub_0D25E4(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;

    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }

    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) mod 2^32. */
    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    uint32_t d3 = d0;
    d2w = d4w;
    uint32_t d4 = d1;

    d1 = rd32(m, 0x000D27A4u + off);
    d0 = rd32(m, 0x000D263Cu + off);

        /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k is the
         * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
         * mod 2^32 is exact for every k the table has. */

    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    *out_d0 = d0 + d3;
    *out_d1 = d1 + d4;
    return 0;
}

/* ROM 0x0D2D44 */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);

        /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k is the
         * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
         * mod 2^32 is exact for every k the table has. */

    *out_d0 = (uint32_t)(d0 * (uint32_t)d2w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d2w);
    return 0;
}

/* ROM 0x0CF898 */
static int sub_0CF898(const f3mem_t *m, uint32_t obj,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t d6w = 0x0168u;
    uint32_t v;

    v = rd32(m, a1 + 0x08u) + rd32(m, a1 + 0x0Cu);
    wr32(m, a1 + 0x08u, v);
    if ((int32_t)v < 0) {
        wr16(m, a1 + 0x08u, (uint16_t)(rd16(m, a1 + 0x08u) + d6w));
    } else if (rd16(m, a1 + 0x08u) >= d6w) {
        wr16(m, a1 + 0x08u, (uint16_t)(rd16(m, a1 + 0x08u) - d6w));
    }

    v = rd32(m, a1 + 0x10u) + rd32(m, a1 + 0x14u);
    wr32(m, a1 + 0x10u, v);
    if ((int32_t)v < 0) {
        wr32(m, a1 + 0x10u, 0u);
    } else if (rd16(m, a1 + 0x10u) > 0x0032u) {
        wr16(m, a1 + 0x10u, 0x0032u);
    }

    v = rd32(m, a1 + 0x18u) + rd32(m, a1 + 0x1Cu);
    wr32(m, a1 + 0x18u, v);
    if ((int32_t)v < 0) {
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + d6w));
    } else if (rd16(m, a1 + 0x18u) >= d6w) {
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) - d6w));
    }

    v = rd32(m, a1 + 0x20u) + rd32(m, a1 + 0x24u);
    wr32(m, a1 + 0x20u, v);
    if ((int32_t)v < 0) {
        wr16(m, a1 + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) + d6w));
    } else if (rd16(m, a1 + 0x20u) >= d6w) {
        wr16(m, a1 + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) - d6w));
    }

    uint16_t d0w = rd16(m, a1 + 0x08u);
    uint16_t d1w = rd16(m, a1 + 0x10u);
    uint16_t d2w = d0w;
    uint16_t d3w = d1w;
    uint32_t d0, d1;
    if (sub_0D1EBC(m, d0w, d1w, &d0, &d1) < 0)
        return -1;

    uint32_t d4 = d0;
    uint32_t d5 = d1;

    d2w = (uint16_t)(d2w - rd16(m, a1 + 0x18u));
    if ((int16_t)d2w < 0)
        d2w = (uint16_t)(d2w + d6w);

    if (sub_0D1EBC(m, d2w, d3w, &d0, &d1) < 0)
        return -1;

    d0w = rd16(m, a1 + 0x20u);
    d1w = (uint16_t)(d1 >> 16);
    d3w = 0x005Au;
    if (d2w < 0x00B4u) {
        d1w = (uint16_t)(0u - d1w);
        d3w = (uint16_t)(d3w + 0x00B4u);
    }

    d2w = d1w;
    if (sub_0D1EBC(m, d0w, d1w, &d0, &d1) < 0)
        return -1;

    d0w = (uint16_t)(rd16(m, a1 + 0x18u) + d3w);
    if (d0w >= d6w)
        d0w = (uint16_t)(d0w - d6w);

    d1w = (uint16_t)((uint16_t)(d1 >> 16) + d2w);
    if (sub_0D25E4(m, d0w, d1w, &d0, &d1) < 0)
        return -1;

    *out_d0 = d0 + d4;
    *out_d1 = d1 + d5;
    return 0;
}

/* ROM 0x0CF7FE */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if ((flags & 0x80u) == 0u)
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;

    wr8(m, obj + 0x07u, (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u));
}

/* ROM 0x0D0E3A */

/* ROM 0x13F238 -> 0x0039D8 */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x104C98 / 0x104CF4: spawn a 0xD9092 shot when a muzzle is set */
static void spawn_shot_104C98(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, obj + 0x68u) == 0u)
        return;
    uint32_t c = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
    if ((int32_t)c < 0)
        return;
    wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x10u));
    wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x30u));
    uint16_t d0 = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) + 0x82u);
    wr16(m, c + 0x58u, d0);
    wr16(m, c + 0x10u, 0u);
}

/* jump table 0x104C3E [obj+6]: 1=0x104C52 2=0x104C68 3=0x104C6A 4=0x104CF4 */
static void state_104C3E(const f3mem_t *m, uint32_t obj, uint8_t st)
{
    switch (st) {
    case 1: /* 0x104C52 */
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
            wr8(m, obj + 0x06u, 2u);
        break;
    case 2: /* 0x104C68 */
        break;
    case 3: /* 0x104C6A */
    {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x4Eu) - 1u);
        wr8(m, obj + 0x4Eu, t);
        if (t == 0u) {
            wr8(m, obj + 0x4Eu, 0x5Au);
            wr8(m, obj + 0x23u, 1u);
            wr8(m, obj + 0x06u, 4u);
            play_sound(m, 0x0027u);
        }
        spawn_shot_104C98(m, obj);
        break;
    }
    default: /* 4: 0x104CF4 */
    {
        spawn_shot_104C98(m, obj);
        wr8(m, obj + 0x4Fu, 0u);
        uint8_t t = (uint8_t)(rd8(m, obj + 0x4Eu) - 1u);
        wr8(m, obj + 0x4Eu, t);
        if (t == 0u) {
            wr8(m, obj + 0x4Eu, 0x1Eu);
            wr8(m, obj + 0x06u, 3u);
            break;
        }
        t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t != 0u)
            break;
        wr8(m, obj + 0x23u, 5u);
        uint32_t c = Obj_Spawn(m, obj, 0x000DA0D6u, 9u, 2u, 0u);
        if ((int32_t)c < 0)
            break;
        wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x1Bu));
        wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x3Bu));
        wr16(m, c + 0x58u, 0x00CCu);
        wr8(m, obj + 0x4Fu, 0x70u);
        break;
    }
    }
}

/* ROM 0x1049C4 */
int Obj_1049C4_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto lab_104acc;
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto lab_104b64;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u ||
        (int16_t)rd16(m, obj + 0x14u) < -0x000A ||
        (int16_t)rd16(m, obj + 0x14u) > 0x011C) {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (f7 & 0x02u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, 0x00104DC0u) < 0)
                return -1;
        }
    }

    /* 0x104A1E */
    if (rd8(m, obj + 0x07u) & 0x10u) {
        int16_t x = (int16_t)rd16(m, obj + 0x3Au);
        if (x < -0x50 || x > 0x154)
            goto lab_104c34;
    }
    {
        uint8_t st = rd8(m, obj + 0x06u);
        if (st < 1u || st > 4u)
            return -1; /* jump table 0x104C3E entry not handled */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        state_104C3E(m, obj, st);
    }
    sub_0D0690(m, obj);
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    if (rd8(m, obj + 0x1Du) == 0u)
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x22u));

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return 0;
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x < -0x000A || x > 0x011C)
            return 0;
    }
    {
        uint32_t d0, d1;
        if (sub_0CF898(m, obj, &d0, &d1) < 0)
            return -1;
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
        uint16_t ang = sub_0D0E52(m, (uint16_t)(rd16(m, obj + 0x3Au) - 0x0078u),
                                  (uint16_t)(rd16(m, obj + 0x3Eu) - 0x0054u));
        if (sub_0D2D44(m, ang, 7u, &d0, &d1) < 0)
            return -1;
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
    }
    return 0;

lab_104acc:
    if (rd8(m, obj + 0x06u) >= 3u) {
        /* 0x104B3E */
        sub_0D0E04(m, obj, 0x00001770u);
        Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00001770u);
        goto lab_104b64;
    }
    {
        uint32_t c = Obj_Spawn(m, obj, 0x000DC5B6u, 3u, 2u, 0u);
        if ((int32_t)c >= 0) {
            wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x28u));
            wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x26u));
            wr16(m, c + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, c + 0x60u, rd16(m, obj + 0x60u));
        }
    }
    /* 0x104B1E: 0x0CF1D8 (start script 0x104E40) */
    wr32(m, obj + 0x44u, 0x00104E40u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    wr32(m, obj + 0x24u, 0x000F0000u);
    wr8(m, obj + 0x4Eu, 0x28u);
    wr8(m, obj + 0x06u, 3u);
    return 0;

lab_104b64:
    {
        uint32_t d1 = rd32(m, obj + 0x14u) + 0x00180000u;
        uint32_t d2 = rd32(m, obj + 0x18u) + 0x00300000u;
        uint16_t d3w = rd16(m, obj + 0x58u);
        uint16_t d4w = rd16(m, obj + 0x60u);
        static const uint32_t fns[3] = { 0x000DC5B6u, 0x000DC49Au, 0x000DC49Au };
        static const uint32_t ddx[3] = { 0u, 0x00100000u, 0x00140000u };
        for (int i = 0; i < 3; i++) {
            uint32_t c = Obj_Spawn(m, obj, fns[i], 3u, 2u, 0u);
            if ((int32_t)c < 0)
                goto lab_104c34;
            if (i > 0) {
                d1 += ddx[i];
                d2 -= 0x00100000u;
            }
            wr32(m, c + 0x14u, d1);
            wr32(m, c + 0x18u, d2);
            wr16(m, c + 0x58u, d3w);
            wr16(m, c + 0x60u, d4w);
        }
    }

lab_104c34:
    wr8(m, obj + 0x06u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
