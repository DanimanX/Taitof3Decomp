#include "obj_184D6C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t scale = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x87Eu));
    uint32_t prod = (uint32_t)d0w * (uint32_t)scale;
    uint16_t d1w = (uint16_t)(prod >> 8);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x878u) % d1w);
}

/* ROM 0x17B02E: resolve score target object from +0x35 / +0x36; -1 if none. */
static int32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u) {
        uint32_t p = rd32(m, obj + 0x36u);
        return (int32_t)rd32(m, p + 0x08u);
    }
    if (mode == 2u)
        return (int32_t)rd32(m, obj + 0x36u);
    return -1;
}

/* ROM 0x17940E: compute perspective zoom byte +0x1E/+0x1F from +0x58. */
static void sub_17940E(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = (rd32(m, obj + 0x58u) & 0x00FFFF00u) + 0x00010000u;
    uint32_t d1 = 0x01000000u / d0;
    uint8_t z = (uint8_t)(-(int8_t)(uint8_t)d1);
    wr8(m, obj + 0x1Fu, z);
    wr8(m, obj + 0x1Eu, z);
}

/* ROM 0x178CCE: step (+0x42, +0x5C) by (arg0, arg1), integrate (+0x50, +0x54), update +0x20. */
static void sub_178CCE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    int16_t d1 = (int16_t)(rd16(m, obj + 0x5Cu) + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, (uint16_t)d0);
    wr16(m, obj + 0x5Cu, (uint16_t)d1);

    int32_t l0 = (int32_t)(int16_t)(d0 + 0x20) << 8;
    l0 = (l0 & ~0xFFFF) | (l0 & 0xC000);
    int32_t l1 = (int32_t)(int16_t)(d1 + 0x20) << 8;
    l1 = (l1 & ~0xFFFF) | (l1 & 0xC000);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)l0);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)l1);
    wr8(m, obj + 0x20u, (uint8_t)Obj_Atan2(m, (uint16_t)d0, (uint16_t)d1));
}

/* Local 10-byte script step handling commands specific to 0x184D6C (and 0x178CCE). */
static int script10_step(const f3mem_t *m, uint32_t obj)
{
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);
        if (fn == 0u)
            return Obj_Script10(m, obj);

        switch (fn) {
        case 0x00178AB6u:
            wr16(m, obj + 0x54u, rd16(m, a1 + 8u));
            wr16(m, obj + 0x56u, 0u);
            break;
        case 0x00178AC6u:
            wr16(m, obj + 0x50u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x54u, rd16(m, a1 + 8u));
            wr16(m, obj + 0x52u, 0u);
            wr16(m, obj + 0x56u, 0u);
            break;
        case 0x00178B0Eu:
            wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x20u, rd16(m, a1 + 8u));
            break;
        case 0x00178B38u:
            wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x5Cu, rd16(m, a1 + 8u));
            break;
        case 0x00178CCEu:
            sub_178CCE(m, obj);
            break;
        case 0x00185176u:
            sub_178CCE(m, obj);
            if ((int16_t)rd16(m, obj + 0x54u) > 0xAE) {
                wr16(m, obj + 0x48u, 0u);
                Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x25));
                Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x30));
            }
            break;
        default:
            return Obj_Script10(m, obj);
        }

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0) {
                wr32(m, obj + 0x44u, a1);
                wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                return 0;
            }
            int16_t c = (int16_t)rd16(m, obj + 0x4Au);
            if (c >= 0) {
                if (c == 0)
                    wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
                c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
                wr16(m, obj + 0x4Au, (uint16_t)c);
                if (c == 0) {
                    a1 += 0x0Au;
                    wr32(m, obj + 0x44u, a1);
                    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                    return 0;
                }
            }
            a1 = rd32(m, a1 + 6u);
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            return 0;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }
}

/* ROM 0x1789FC: 12-byte script step; returns 0 on success, -1 on unknown command. */
static int sub_1789FC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t dz = (int16_t)rd16(m, a1 + 0x0Au);
    int16_t z16 = (int16_t)(rd16(m, obj + 0x5Eu) + dz);
    wr16(m, obj + 0x5Eu, (uint16_t)z16);
    int32_t z32 = (int32_t)rd32(m, obj + 0x58u) + ((int32_t)z16 * 0x100);
    if (z32 < 0)
        z32 &= 0x0F;
    wr32(m, obj + 0x58u, (uint32_t)z32);

    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);
        if (fn == 0u) {
            int16_t rep = (int16_t)rd16(m, obj + 0x4Au);
            if (rep < 0) {
                a1 = rd32(m, a1 + 6u);
            } else {
                if (rep == 0)
                    wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
                uint16_t nr = (uint16_t)(rd16(m, obj + 0x4Au) - 1u);
                wr16(m, obj + 0x4Au, nr);
                if (nr == 0u)
                    a1 += 12u;
                else
                    a1 = rd32(m, a1 + 6u);
            }
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            return 0;
        }

        if (fn == 0x00185626u) {
            wr32(m, obj + 0x50u, 0x01040000u);
            wr32(m, obj + 0x54u, 0x00AE0000u);
            wr32(m, obj + 0x58u, 0u);
            wr16(m, obj + 0x42u, 0u);
            wr16(m, obj + 0x5Cu, 0u);
            wr16(m, obj + 0x5Eu, 0u);
        } else if (fn == 0x00178CCEu) {
            sub_178CCE(m, obj);
        } else if (fn == 0x001793C4u) {
            wr16(m, obj + 0x50u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x54u, rd16(m, a1 + 8u));
            uint32_t d0 = rd16(m, a1 + 0x0Au);
            uint32_t cur = rd32(m, obj + 0x58u);
            wr32(m, obj + 0x58u, (d0 << 8) | (cur & 0xFFu));
            wr16(m, obj + 0x52u, 0u);
            wr16(m, obj + 0x56u, 0u);
        } else {
            return -1;
        }

        int16_t timer = (int16_t)rd16(m, obj + 0x48u);
        if (timer < 0)
            return 0;
        if (timer == 0) {
            a1 = rd32(m, obj + 0x44u) + 12u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            continue;
        }
        timer--;
        wr16(m, obj + 0x48u, (uint16_t)timer);
        if (timer > 0)
            return 0;

        a1 = rd32(m, obj + 0x44u) + 12u;
        if ((int32_t)rd32(m, a1) > 0) {
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            return 0;
        }

        int16_t rep = (int16_t)rd16(m, obj + 0x4Au);
        if (rep < 0) {
            a1 = rd32(m, a1 + 6u);
        } else {
            if (rep == 0)
                wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
            uint16_t nr = (uint16_t)(rd16(m, obj + 0x4Au) - 1u);
            wr16(m, obj + 0x4Au, nr);
            if (nr == 0u)
                a1 += 12u;
            else
                a1 = rd32(m, a1 + 6u);
        }
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        return 0;
    }
}

/* ROM 0x185938: phase 5 shot pattern update */
static void sub_185938(const f3mem_t *m)
{
    wr16(m, F3_A5 + 0x4B52u, 0u);
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x80u) != 0u && sub_0CC2BC(m, 0x0014u) == 0u) {
        uint16_t idx = rd16(m, F3_A5 + 0x4B54u);
        wr16(m, F3_A5 + 0x4B52u, rd16(m, 0x0018596Eu + idx));
        wr16(m, F3_A5 + 0x4B54u, (uint16_t)((idx + 1u) & 7u));
    }
}

/* ROM 0x184F26: update phase HP threshold and blink/phase flags */
static void sub_184F26(const f3mem_t *m)
{
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x10u));
    int32_t thresh = (int32_t)rd32(m, F3_A5 + 0x4B10u);
    int32_t hp = (int32_t)rd32(m, F3_A5 + 0x4B0Cu);
    if (thresh > hp) {
        uint32_t step = rd32(m, F3_A5 + 0x4B18u);
        wr32(m, F3_A5 + 0x4B10u, (uint32_t)(thresh - (int32_t)step));
        wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x01u));
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x10u));
    }
}

static int run_phase_cb(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    if (fn == 0x0017AE7Cu) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            wr32(m, obj + 0x44u, 0x0017AF48u);
            wr16(m, obj + 0x48u, rd16(m, 0x0017AF4Cu));
            wr16(m, obj + 0x4Au, 0u);
            uint32_t arg = ((uint32_t)rd8(m, obj + 0x17u) << 24) |
                           ((uint32_t)rd8(m, obj + 0x1Bu) << 16) |
                           (uint32_t)rd16(m, F3_A5 + 0x4B30u);
            wr32(m, obj + 0x0Cu, Obj_Spawn(m, obj, 0x0017A5A8u, 5u, 2u, arg));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x01u));
            wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x10u));
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x04u));
            (void)Obj_Spawn(m, obj, 0x0013E9E8u, 0u, 1u, rd16(m, F3_A5 + 0x4B32u));
        }
        if (script10_step(m, obj) < 0)
            return -1;
        int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x16u));
        int16_t y = (int16_t)(rd16(m, obj + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, F3_A5 + 0x32FAu, (uint16_t)x);
        wr16(m, F3_A5 + 0x32FCu, (uint16_t)y);
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 0xFFFFu);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            Obj_Free(m, rd32(m, obj + 0x0Cu));
        }
        return 0;
    }

    if (fn == 0x00185082u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            wr32(m, obj + 0x44u, 0x00185112u);
            wr16(m, obj + 0x48u, rd16(m, 0x00185116u));
            wr16(m, obj + 0x4Au, 0u);
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x01u));
            wr16(m, F3_A5 + 0x4B08u, rd16(m, obj + 0x54u));
        }
        wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x4B08u));
        if (script10_step(m, obj) < 0)
            return -1;
        wr16(m, F3_A5 + 0x4B4Eu, rd16(m, obj + 0x20u));
        wr16(m, F3_A5 + 0x4B08u, rd16(m, obj + 0x54u));
        if ((int16_t)rd16(m, obj + 0x54u) >= 0xAE) {
            uint16_t r = (uint16_t)(Rng_NextMem(m) & 3u);
            wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + r));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x04u));
        }
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 2u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x01u));
            wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x08u));
        }
        return 0;
    }

    if (fn == 0x001851B4u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            uint32_t tbl = 0x00185002u;
            if ((int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= (int32_t)rd32(m, F3_A5 + 0x4B10u)) {
                tbl = 0x00185042u;
                wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x02u));
            }
            uint16_t idx = (uint16_t)((rd16(m, obj + 0x62u) + 1u) & 0x0Fu);
            wr16(m, obj + 0x62u, idx);
            uint32_t a1 = rd32(m, tbl + (uint32_t)idx * 4u);
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
        }
        wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x4B08u));
        if (sub_1789FC(m, obj) < 0)
            return -1;
        sub_17940E(m, obj);
        wr16(m, F3_A5 + 0x4B08u, rd16(m, obj + 0x54u));
        wr16(m, F3_A5 + 0x4B4Eu, 0u);
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 2u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
        }
        wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x01u));
        if (rd16(m, obj + 0x1Eu) == 0u) {
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x01u));
            uint16_t r = (uint16_t)(Rng_NextMem(m) & 3u);
            wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + r));
            if ((int16_t)rd16(m, F3_A5 + 0x4B22u) >= 2)
                wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x04u));
            if (sub_0CC2BC(m, 0x00B4u) == 0u)
                wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x40u));
            if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x1C20)
                wr32(m, obj + 0x24u, 0u);
            if ((int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
                wr16(m, F3_A5 + 0x4B38u, 3u);
                wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
                int32_t score_obj = sub_17B02E(m, obj);
                if (score_obj >= 0)
                    wr32(m, (uint32_t)score_obj + 0x48u, rd32(m, (uint32_t)score_obj + 0x48u) + 0x7530u);
                wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & ~0x8Eu));
            }
        }
        return 0;
    }

    if (fn == 0x00185648u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            wr32(m, obj + 0x44u, 0x001856C4u);
            wr16(m, obj + 0x48u, rd16(m, 0x001856C8u));
            wr16(m, obj + 0x4Au, 0u);
        }
        wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x4B08u));
        if (script10_step(m, obj) < 0)
            return -1;
        wr16(m, F3_A5 + 0x4B4Eu, 0u);
        wr16(m, F3_A5 + 0x4B08u, rd16(m, obj + 0x54u));
        uint16_t r = (uint16_t)(Rng_NextMem(m) & 3u);
        wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + r));
        if ((int16_t)rd16(m, obj + 0x42u) <= 0) {
            wr16(m, F3_A5 + 0x4B38u, 4u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x04u));
            Sound_PlayCmdForced(m, Sound_PreCmdHook(m, 0x30));
        }
        return 0;
    }

    if (fn == 0x001856D8u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            wr32(m, obj + 0x44u, 0x001857C4u);
            wr16(m, obj + 0x48u, rd16(m, 0x001857C8u));
            wr16(m, obj + 0x4Au, 0u);
            wr32(m, obj + 0x0Cu, Obj_Spawn(m, obj, 0x0017A50Eu, 3u, 2u, 0x4020u));
        }
        if (script10_step(m, obj) < 0)
            return -1;
        wr16(m, F3_A5 + 0x4B4Eu, rd16(m, obj + 0x20u));
        if ((int16_t)rd16(m, obj + 0x54u) <= 0x50) {
            wr16(m, F3_A5 + 0x4B38u, 5u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x02u));
            wr32(m, obj + 0x24u, 0x012C0000u);
            Obj_18495E(m, obj);
            uint32_t hp = rd32(m, obj + 0x24u);
            wr32(m, F3_A5 + 0x4B0Cu, hp);
            wr32(m, F3_A5 + 0x4B10u, hp);
            wr32(m, F3_A5 + 0x4B14u, hp);
            uint32_t step = hp / 10u;
            wr32(m, F3_A5 + 0x4B18u, step);
            wr32(m, F3_A5 + 0x4B10u, hp - step * 2u);
            (void)Obj_Spawn(m, obj, 0x00179DEEu, 5u, 2u, 0u);
            Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x13));
            Obj_Free(m, rd32(m, obj + 0x0Cu));
        }
        return 0;
    }

    if (fn == 0x001857D8u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            wr32(m, obj + 0x44u, 0x00185890u);
            wr16(m, obj + 0x48u, rd16(m, 0x00185894u));
            wr16(m, obj + 0x4Au, 0u);
            wr16(m, F3_A5 + 0x4B70u, 0x00F0u);
            wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) | 0x04u));
        }
        if (script10_step(m, obj) < 0)
            return -1;
        wr16(m, F3_A5 + 0x4B4Eu, rd16(m, obj + 0x20u));
        if (sub_0CC2BC(m, 0x00F0u) == 0u)
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x40u));
        wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x80u));
        sub_185938(m);
        if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)
            wr32(m, obj + 0x24u, 0u);
        if ((int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
            wr16(m, F3_A5 + 0x4B38u, 0u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            int32_t score_obj = sub_17B02E(m, obj);
            if (score_obj >= 0)
                wr32(m, (uint32_t)score_obj + 0x48u, rd32(m, (uint32_t)score_obj + 0x48u) + 0x186A0u);
            wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & ~0x05u));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0xC0u));
            wr16(m, F3_A5 + 0x4B52u, 0u);
        }
        return 0;
    }

    return -1;
}

/* ROM 0x184D6C: boss main controller tick */
int Obj_184D6C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x4B0Cu, rd32(m, obj + 0x24u));
    int16_t state = (int16_t)rd16(m, F3_A5 + 0x4B38u);
    if (state == -1) {
        (void)Obj_Spawn(m, obj, 0x0017AE0Au, 0u, 1u, 0u);
        wr16(m, obj + 0x06u, 1u);
        wr32(m, obj + 0x00u, 0x00184DA8u);
        return 0;
    }

    if (state > 2 && (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
        (rd8(m, obj + 0x23u) & 0x80u) == 0u) {
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x80u));
        (void)Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x100u);
    }

    uint32_t tbl = rd32(m, obj + 0x64u);
    uint32_t fn = rd32(m, tbl + (uint32_t)state * 4u);
    if (run_phase_cb(m, obj, fn) < 0)
        return -1;

    wr8(m, F3_A5 + 0x4B36u, 0u);
    uint8_t d0 = rd8(m, F3_A5 + 0x4B28u);
    if (d0 == rd8(m, obj + 0x1Du)) {
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));
    } else if (rd8(m, obj + 0x35u) == 0u) {
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    } else {
        wr8(m, obj + 0x1Du, d0);
    }
    wr8(m, obj + 0x35u, 0u);
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));

    if (state == 5)
        sub_184F26(m);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u) {
        Obj_Free(m, rd32(m, F3_A5 + 0x4B62u));
        (void)Obj_Spawn(m, obj, 0x0018675Eu, 3u, 2u, 0x00186C0Eu);
        (void)Obj_Spawn(m, obj, 0x00186782u, 3u, 2u, 0x00186B9Au);
        for (int16_t d1 = 10; d1 >= 0; d1--) {
            (void)Obj_Spawn(m, obj, 0x00186064u, 3u, 2u, (uint32_t)(int32_t)d1);
        }
        wr16(m, obj + 0x06u, 1u);
        wr32(m, obj + 0x00u, 0x00184EDEu);
        return 0;
    }

    wr32(m, obj + 0x00u, 0x00184D6Cu);
    return 0;
}
