#include "obj_10AC10.h"
#include "../objects/obj_shared.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0CEE52 */
static void sub_0CEE52(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | b;
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    Obj_StartDying(m, obj);
}

/* ROM 0x0CEE48 */
static void sub_0CEE48(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEE52(m, obj, d0);
}

/* ROM 0x0CEEA2 */
static void sub_0CEEA2(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | b;
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEEF0u);
}

/* ROM 0x0CEE9A */
static void sub_0CEE9A(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEEA2(m, obj, d0);
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
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

/* ROM 0x13F238 -> 0x0039D8 */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
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

/* ROM 0x0CFA7C: frame record A1 + table 0x0CFA96[d0] -> sprite + hit box */
static void sub_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    uint16_t off = rd16(m, 0x000CFA96u + (uint32_t)((int32_t)(int16_t)d0w * 2));
    a1 += (uint32_t)(int32_t)(int16_t)off;
    /* 0x0CFA86 */
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x10AC10 */
int Obj_10AC10_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x00000190u);
        goto lab_10acc2;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_10acc2;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x0010AE42u) < 0)
            return -1;
    }

    /* 0x10AC4C */
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        /* 0x10AD60 */
        wr8(m, obj + 0x06u, 0u);
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10AC68: state jump table 0x10ADDA */
    switch (rd8(m, obj + 0x06u)) {
    case 1u:
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        wr32(m, obj + 0x08u, rd32(m, obj + 0x58u));   /* 0x10ADEE */
        break;
    case 3u:
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        wr32(m, obj + 0x58u, rd32(m, obj + 0x08u));   /* 0x10AE20 */
        wr8(m, obj + 0x06u, 4u);
        play_sound(m, 0x0011);
        break;
    case 4u:
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        break;                                        /* 0x10AE40: rts */
    default:
        return -1;   /* state 2 (0x10ADF6: 0xD06E0/0xD0BBC) not translated */
    }

    /* 0x10AC76 */
    sub_0CFA7C(m, obj, 0x0010AF16u, rd16(m, obj + 0x08u));
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x10AD78 */
            wr8(m, obj + 0x06u, 0u);
            d0 = 0x00000190u;
            Obj_RelayTimer(m, d0);
            uint32_t a1 = rd32(m, obj + 0x0Cu);
            uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
            wr8(m, a1 + 0x22u, rem);
            if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, d0);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_RelayTimer(m, 0x00000FA0u);
            d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000FA0u);
            sub_0CEEA2(m, obj, d0);
        }
    }
    return 0;

lab_10acc2:
    wr8(m, obj + 0x06u, 0u);
    d0 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
    if ((int32_t)d0 >= 0) {
        wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x10u));
        wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x10u));
        wr16(m, d0 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, d0 + 0x60u, rd16(m, obj + 0x60u));
    }
    {
        /* 0x10AD0E */
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
        wr8(m, a1 + 0x22u, rem);
        if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
            sub_0CEE48(m, obj, d0);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        sub_0D0E04(m, obj, 0x00000FA0u);
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x00000FA0u);
        sub_0CEE52(m, obj, d0);
        return 0;
    }
}
