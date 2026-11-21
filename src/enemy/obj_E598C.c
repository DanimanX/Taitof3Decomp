#include "obj_E598C.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0CBE34 */
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
        uint32_t a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */
static void sub_0D0E30(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x0D1EBC */
static int sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
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
    if (d4w > 50u)
        return -1;

    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    uint32_t d1 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
    uint32_t d3 = d0;
    uint32_t d4 = d1;

    d0 = rd32(m, 0x000D263Cu + off) * (uint32_t)d4w;
    d1 = rd32(m, 0x000D27A4u + off) * (uint32_t)d4w;
    *out_d0 = d0 + d3;
    *out_d1 = d1 + d4;
    return 0;
}

/* ROM 0x0CF898 */
static int sub_0CF898(const f3mem_t *m, uint32_t obj,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    const uint16_t d6w = 0x0168u;
    uint32_t v;

    v = rd32(m, a1 + 0x08u) + rd32(m, a1 + 0x0Cu);
    wr32(m, a1 + 0x08u, v);
    if ((int32_t)v < 0)
        wr16(m, a1 + 0x08u, (uint16_t)(rd16(m, a1 + 0x08u) + d6w));
    else if (rd16(m, a1 + 0x08u) >= d6w)
        wr16(m, a1 + 0x08u, (uint16_t)(rd16(m, a1 + 0x08u) - d6w));

    v = rd32(m, a1 + 0x10u) + rd32(m, a1 + 0x14u);
    wr32(m, a1 + 0x10u, v);
    if ((int32_t)v < 0)
        wr32(m, a1 + 0x10u, 0u);
    else if (rd16(m, a1 + 0x10u) > 0x0032u)
        wr16(m, a1 + 0x10u, 0x0032u);

    v = rd32(m, a1 + 0x18u) + rd32(m, a1 + 0x1Cu);
    wr32(m, a1 + 0x18u, v);
    if ((int32_t)v < 0)
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + d6w));
    else if (rd16(m, a1 + 0x18u) >= d6w)
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) - d6w));

    v = rd32(m, a1 + 0x20u) + rd32(m, a1 + 0x24u);
    wr32(m, a1 + 0x20u, v);
    if ((int32_t)v < 0)
        wr16(m, a1 + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) + d6w));
    else if (rd16(m, a1 + 0x20u) >= d6w)
        wr16(m, a1 + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) - d6w));

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

/* ROM 0x13F238 */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

static void play_sound(const f3mem_t *m, int16_t raw_cmd)
{
    int16_t cmd = sub_13F238(m, raw_cmd);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* Spawn child fragments + sound + die (0x0E5A10..0x0E5B85) */
static void spawn_fragments_and_die(const f3mem_t *m, uint32_t obj)
{
    static const struct {
        uint16_t dx;
        uint16_t dy;
        uint16_t ang;
        uint16_t kind;
    } frags[5] = {
        { 0x0000u, 0x0000u, 0x0087u, 0u },
        { 0x0030u, 0x0000u, 0x002Du, 1u },
        { 0x0000u, 0x0025u, 0x00E1u, 2u },
        { 0x0012u, 0x0026u, 0x010Eu, 3u },
        { 0x002Cu, 0x0026u, 0x013Bu, 4u },
    };

    uint32_t x = rd32(m, obj + 0x14u);
    uint32_t y = rd32(m, obj + 0x18u);
    for (int i = 0; i < 5; i++) {
        uint32_t child = Obj_Spawn(m, obj, 0x000E5C2Au, 5u, 2u, 0u);
        if ((int32_t)child < 0)
            break;
        wr32(m, child + 0x14u, x);
        if (frags[i].dx != 0u)
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + frags[i].dx));
        wr32(m, child + 0x18u, y);
        if (frags[i].dy != 0u)
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + frags[i].dy));
        wr16(m, child + 0x60u, 0x0001u);
        wr16(m, child + 0x58u, frags[i].ang);
        wr16(m, child + 0x10u, frags[i].kind);
    }

    play_sound(m, 0x0025);
    wr8(m, obj + 0x06u, 0u);
    Obj_StartDying(m, obj);
}

/* ROM 0x0E598C */
int Obj_E598C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            sub_0D0E30(m, 0x00000FA0u);
            spawn_fragments_and_die(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x00000FA0u);
        spawn_fragments_and_die(m, obj);
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        spawn_fragments_and_die(m, obj);
        return 0;
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + rd32(m, obj + 0x54u));
    if ((int16_t)rd16(m, obj + 0x3Au) < -0x0064) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    uint8_t state = rd8(m, obj + 0x06u);
    if (state == 1u) {
        /* 0x0E5BA2 */
        if (rd8(m, obj + 0x35u) != 0u) {
            wr8(m, obj + 0x23u, 0x0Au);
            wr8(m, obj + 0x06u, 0x02u);
        }
    } else if (state == 2u) {
        /* 0x0E5BB8 */
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, cnt);
        if (cnt == 0u)
            wr8(m, obj + 0x06u, 0x01u);
        uint32_t d0, d1;
        if (sub_0CF898(m, obj, &d0, &d1) < 0)
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
    } else {
        return -1;
    }

    sub_0D0DC2(m, obj);
    sub_0CBE34(m, obj);
    return 0;
}
